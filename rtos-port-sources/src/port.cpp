/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Implementation routines for the µOS++ scheduler, mainly
 * the context switching and context creation.
 */

#if defined(__ARM_EABI__)

// ----------------------------------------------------------------------------

#if defined(HAVE_MICRO_OS_PLUS_CONFIG_H)
#include <micro-os-plus/config.h>
#endif // HAVE_MICRO_OS_PLUS_CONFIG_H

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_INCLUDE_RTOS)

// ----------------------------------------------------------------------------

#include <string.h>
#include <cassert>

#include <micro-os-plus/rtos.h>

#include <micro-os-plus/architecture-cortexm/exception-handlers.h>

#include <micro-os-plus/device.h>

// ----------------------------------------------------------------------------

using namespace micro_os_plus::rtos;

// ----------------------------------------------------------------------------

#if !defined(__ARM_ARCH_7M__) && !defined(__ARM_ARCH_7EM__) \
    && !defined(__ARM_ARCH_6M__)
#error "Architecture not supported."
#endif

#if defined(__ARM_ARCH_6M__)

#if defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)
#error \
    "MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY should not be used with Cortex-M0/M0+ devices."
#endif

#endif // defined(__ARM_ARCH_6M__)

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

#define __MAX_PRIO ((1 << __NVIC_PRIO_BITS) - 1)

#define MICRO_OS_PLUS_MACRO_SHARP(x) #x
#define MICRO_OS_PLUS_MACRO_STRINGIFY(x) MICRO_OS_PLUS_MACRO_SHARP (x)

static_assert (MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
                   != 0,
               "MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_"
               "PRIORITY cannot be 0");
static_assert (
    MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
        <= __MAX_PRIO,
    "MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY must be "
    "<= " MICRO_OS_PLUS_MACRO_STRINGIFY (__MAX_PRIO));

#endif // defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_6M__)
extern uint32_t __vectors_start;
#endif

// ----------------------------------------------------------------------------

extern "C"
{
  extern unsigned int __heap_end__;
  extern unsigned int __stack;
}

// ----------------------------------------------------------------------------

namespace micro_os_plus
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace stack
      {
        // Stack frame, as used by PendSV.

        // Offsets in words, from SP up

        // Saved always by ARM
        // (17 optional padding/aligner)
        // 16 xPSR (xPSR bit 9 = 1 if padded)
        // 15 return address (PC, R15)
        // 14 LR (R14)
        // 13 R12
        // 12 R3
        // 11 R2
        // 10 R1
        //  9 R0

        // Saved always by context switch handler.
        // "stmdb %[r]!, {r4-r9,sl,fp,lr}"
        //  8 EXC_RETURN (R14)
        //  7 FP (R11)
        //  6 SL (R10)
        //  5 R9
        //  4 R8
        //  3 R7
        //  2 R6
        //  1 R5
        //  0 R4 <-- new SP value

        // Valid Values for EXC_RETURN
        // 0xFFFFFFFD/0xFFFFFFED - Return to Thread mode and use the
        // Process Stack for return
        // 0xFFFFFFF9/0xFFFFFFE9 - Return to Thread mode and use the
        // Main Stack for return
        // 0xFFFFFFF1/0xFFFFFFE1 - Return to Handler mode
        // (always use Main Stack)
        // (Joseph Yiu, Table 8.2, pag 279)
        //
        // Bit Fields of the EXC_RETURN
        // - bits 31:28 EXC_RETURN indicator, 0xF
        // - bits 27:5 Reserved, all 1
        // - bit 4 (0x10) Stack Frame type
        // 1 (8 words) or 0 (26 words).
        // Always 1 when the floating unit is unavailable.
        // This value is set to the inverted value of FPCA bit in the
        // CONTROL register when entering an exception handler.
        // - bit 3 (0x08) Return mode
        // 1 (Return to Thread) or 0 (Return to Handler)
        // - bit 2 (0x04) Return stack
        // 1 (Return with Process Stack) or 0 (Return with Main Stack)
        // - bit 1 (0x02) Reserved, 0
        // - bit 0 (0x01) Reserved, 1
        // (Joseph Yiu, Table 8.1, pag 278)

        // CONTROL.FPCA (bit 2, 0x04)
        // Floating Point Context Active – This bit is only available
        // in Cortex-M4 with floating point unit implemented. The
        // exception handling mechanism uses this bit to determine if
        // registers in the floating point unit need to be saved when
        // an exception has occurred.
        // When this bit is 0 (default), the floating point unit has
        // not been used in the current context and therefore there
        // is no need to save floating point registers.
        // When this bit is 1, the current context has used floating
        // point instructions and therefore need to save floating
        // point registers.
        // The FPCA bit is set automatically when a floating point
        // instruction is executed. This bit is clear by hardware
        // on exception entry.
        // There are several options for handling saving of floating
        // point registers.
        // (Joseph Yiu, Table 4.3, pag. 87)

        // Saved always by ARM.
        // (50 optional padding/aligner)
        // 49 FPSCR
        // 48 S15
        // ...
        // 34 S1
        // 33 S0
        // 32 xPSR (xPSR bit 9 = 1 if padded)
        // 31 return address (PC, R15)
        // 30 LR (R14)
        // 29 R12
        // 28 R3
        // 27 R2
        // 26 R1
        // 25 R0

        // Saved conditionally if EXC_RETURN, bit 4 is 0 (zero).
        // "vldmiaeq %[r]!, {s16-s31}"
        // 24 S31
        // 23 S30
        // ...
        // 10 S17
        //  9 S16

        // Saved always by context switch handler.
        // "stmdb %[r]!, {r4-r9,sl,fp,lr}"
        //  8 EXC_RETURN (R14)
        //  7 FP (R11)
        //  6 SL (R10)
        //  5 R9
        //  4 R8
        //  3 R7
        //  2 R6
        //  1 R5
        //  0 R4 <-- new SP value

        typedef struct frame_s
        {
          // Restored manually by ldmia %[r]!, {r4-r9,sl,fp[,r14]}.
          // r14 is stored twice for a convenient restore, to test
          // the FPU bit.
          stack::element_t r4;
          stack::element_t r5;
          stack::element_t r6;
          stack::element_t r7;
          stack::element_t r8;
          stack::element_t r9;
          stack::element_t r10_sl;
          stack::element_t r11_fp;
          stack::element_t r14_exec_return;
          // Restored automatically by exception return.
          stack::element_t r0;
          stack::element_t r1;
          stack::element_t r2;
          stack::element_t r3;

          stack::element_t r12;
          stack::element_t r14_lr;
          stack::element_t r15_pc;
          stack::element_t psr;
        } frame_t;
      } // namespace stack

      /**
       * @brief Create a new thread context on the stack.
       * @param [in] context Pointer to thread context.
       * @param [in] function Pointer to function to execute in the new
       * context.
       * @param [in] arguments Function arguments.
       *
       * @details
       * Initialise the stack with a repetitive pattern; create an
       * exception stack frame (at stack top) such that a later
       * PendSV will pass control to the new context.
       */
      void
      context::create (void* context, void* function, void* arguments)
      {
#if defined(MICRO_OS_PLUS_TRACE_RTMICRO_OS_PLUS_THREAD_CONTEXT)
        trace::printf ("port::context::%s(%p)\n", __func__, context);
#endif
        rtos::thread::context* th_ctx
            = static_cast<rtos::thread::context*> (context);

        class rtos::thread::stack& stack = th_ctx->stack ();

        rtos::thread::stack::element_t* p = stack.top ();

        // Be sure the stack is large enough to hold at least
        // two exception frames.
        assert ((p - stack.bottom ())
                > static_cast<int> (2 * sizeof (stack::frame_t)));

        p -= (sizeof (stack::frame_t)
              / sizeof (rtos::thread::stack::element_t));

        // Align the frame to 8 bytes than leave one more word for the extra
        // stack element, r14_exec_return, which is the 9th.
        // Note: if the thread stack is not aligned at an 8 byte boundary,
        // var_args() will fail (for example printf() does not floats/doubles).
        if ((reinterpret_cast<uintptr_t> (p) & 3) != 0)
          {
            p = reinterpret_cast<rtos::thread::stack::element_t*> (
                (reinterpret_cast<int> (p)) & (~3));
          }

        if ((reinterpret_cast<uintptr_t> (p) & 7) == 0)
          {
            --p;
          }

        // For convenience, use the stack frame structure.
        stack::frame_t* f = reinterpret_cast<stack::frame_t*> (p);

        // The stack as if after a context save.

        // Thread starts in thumb state (T bit set).
        f->psr = 0x01000000; // xPSR +15*4=64

        // The address of the trampoline code. // PC/R15 +14*4=60
        f->r15_pc = static_cast<rtos::thread::stack::element_t> (
            (reinterpret_cast<ptrdiff_t> (function)) & (~1));

        // Link register // LR/R14 +13*4=56
#if defined(MICRO_OS_PLUS_BOOL_RTOS_PORT_CONTEXT_CREATE_ZERO_LR)
        f->r14_lr = 0x00000000;
#else
        // 0x0 looks odd in the debugger, so try to hide it.
        // In Eclipse using 'function+2' will make the stack trace
        // start with 'function' (don't ask why).
        f->r14_lr = static_cast<rtos::thread::stack::element_t> (
            (reinterpret_cast<ptrdiff_t> (function) + 2));
#endif
        // R13 is the SP; it is not present in the frame,
        // it is loaded separately as PSP.

        // The 0x00010203 is added to spot possible endianness issues.
        f->r12 = 0xCCCCCCCC + 0x00010203; // R12 +12*4=52

        // According to ARM ABI, the first 4 word parameters are
        // passed in R0-R3. Only 1 is used.
        f->r3 = 0x33333333 + 0x00010203; // R3 +11*4=48
        f->r2 = 0x22222222 + 0x00010203; // R2 +10*4=44
        f->r1 = 0x11111111 + 0x00010203; // R1 +9*4=40
        f->r0 = reinterpret_cast<rtos::thread::stack::element_t> (
            arguments); // R0 +8*4=36

        // This frame does not include initial FPU registers.
        // bit 4: 1 (8 words), 0 (26 words)
        // bit 3: 1 (return to thread), 0 (return to handler)
        // bit 2: 1 (return with PSP), 0 (return with MSP)
        // bit 1 = 0
        // bit 0 = 1
        f->r14_exec_return = 0xFFFFFFFD;

        f->r11_fp = 0xBBBBBBBB + 0x00010203; // R11 +7*4=32
        f->r10_sl = 0xAAAAAAAA + 0x00010203; // R10 +6*4=28
        f->r9 = 0x99999999 + 0x00010203; // R9 +5*4=24
        f->r8 = 0x88888888 + 0x00010203; // R8 +4*4=20
        f->r7 = 0x77777777 + 0x00010203; // R7 +3*4=16
        f->r6 = 0x66666666 + 0x00010203; // R6 +2*4=12
        f->r5 = 0x55555555 + 0x00010203; // R5 +1*4=8
        f->r4 = 0x44444444 + 0x00010203; // R4 +0*4=4

        // Store the current stack pointer in the context.
        th_ctx->port_.stack_ptr = p;

        // Guarantee that the stack is properly aligned.
        assert (((reinterpret_cast<int> (&f->r0)) & 7) == 0);
      }

      /**
       * @brief Start the SysTick clock.
       *
       * @details
       * Some vendors libraries (like ST HAL) already initialise SysTick
       * during their code, but with a default rate.
       *
       * It is here explicitly to be sure it is always done properly,
       * with the required rate.
       */
      void
      clock_systick::start (void)
      {
#if defined(NDEBUG)
        SysTick_Config (SystemCoreClock / rtos::clock_systick::frequency_hz);
#else
        assert (SysTick_Config (SystemCoreClock
                                / rtos::clock_systick::frequency_hz)
                == 0);
#endif

        // Set SysTick interrupt priority to the lowest level (highest value).
        NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
      }

      // ----------------------------------------------------------------------

      namespace scheduler
      {
        state_t lock_state;

        result_t
        initialize (void)
        {
          __disable_irq ();

          return result::ok;
        }

        /**
         * @brief Start the scheduler and pass control to the main thread.
         *
         * @details
         * After power up, the processor is in privileged mode.
         * The hardware automatically
         * initialises the MSP with the first word in the vector table.
         * The core has CONTROL.SPSEL = 0 (bit 1), meaning
         * "always using MSP".
         *
         * The traditional way to start the first thread
         * is to use SVC 0 and in the handler change the stack
         * pointer, change the mode and return into the first thread.
         *
         * However, if the entire OS remains in privileged mode,
         * this seems not necessary, and Chapter 12.9.1
         * "Running a system with two stacks" in "The Definitive
         * Guide to ARM Cortex!-M3 and Cortex-M4 Processors" by
         * Joseph Yiu, provides a simpler solution.
         *
         * The optimization attribute is required for ARM-v6M
         * where instead of reading VTOR, the stack address is
         * fetched from the first flash word, and reading from
         * 0x0 is usually considered undefined behaviour.
         */
        void
#if defined(__ARM_ARCH_6M__)
            __attribute__ ((optimize ("no-delete-null-pointer-checks")))
#endif
            start (void)
        {
#if defined(MICRO_OS_PLUS_TRACE_RTOS_SCHEDULER)
          trace::printf ("port::scheduler::%s() \n", __func__);
#endif

#if defined(__ARM_FP)
          // The FPU should have been enabled by
          // micro_os_plus_startup_initialize_hardware_early().
#endif

          // Disable all interrupts, to safely change the stack.
          __disable_irq ();

          // The main trick is to switch the current SP from MSP to PSP
          // without breaking the running code. This is simply done by
          // initialising PSP with the same value as MSP.
          __set_PSP (__get_MSP ());
          // Configure thread mode to use PSP (CONTROL.SPSEL=1).
          __set_CONTROL (__get_CONTROL () | CONTROL_SPSEL_Msk);

          // Barrier, as usual after all changes to CONTROL.
          __ISB ();

          // Re-initialise MSP to the reset value. This might be a small
          // problem, since PSP is only a few words below  it, and enabling
          // interrupts might allow the exception stack to overwrite the
          // current thread stack, but in reality this routine does not
          // use stack variables and does not return, but will switch
          // to the main thread, which has its own PSP, so the current
          // stack shouldn't be a problem.

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if !defined(MICRO_OS_PLUS_DISABLE_CORTEXM_SET_MSP_VIA_VTOR)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
          uint32_t* vectors_addr = reinterpret_cast<uint32_t*> (SCB->VTOR);
#pragma GCC diagnostic pop
          __set_MSP (*vectors_addr);
#endif // !defined(MICRO_OS_PLUS_DISABLE_CORTEXM_SET_MSP_VIA_VTOR)

#elif defined(__ARM_ARCH_6M__)

          // VTOR in not available on this architecture.
          // Read the stack pointer from the first word
          // stored in the vectors table. Since normally
          // accesses that dereference the NULL pointer,
          // are undefined, the function needs to be marked
          // as -fno-delete-null-pointer-checks.

#if 0
          // An alternate way would try to fool the compiler
          // with a volatile pointer, but this is not
          // guaranteed to work in future versions.
          uint32_t* volatile vectors_addr = 0x00000000;
          __set_MSP (*vectors_addr);
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
          __set_MSP (*(static_cast<uint32_t*> (0x00000000)));
#pragma GCC diagnostic pop
#endif

#else

#error Implement __set_MSP() on this architecture.

#endif

          // Guarantee that the interrupt stack is properly aligned.
          assert ((__get_MSP () & 7) == 0);

          // Set the beginning address and size of the interrupt stack.
          rtos::interrupts::stack ()->set (
              reinterpret_cast<stack::element_t*> (&__heap_end__),
              (static_cast<std::size_t> (&__stack - &__heap_end__)
               * sizeof (__stack)));

          // Set PendSV interrupt priority to the lowest level (highest value).
          NVIC_SetPriority (PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

          // One disadvantage of this simple method is that
          // the first context switch will want to save the initial SP
          // somewhere, so prepare a fake thread context.
          // Don't worry for being on the stack, this is used
          // only once and can be overridden later.
          micro_os_plus_thread_t thread_ghost;
          memset (&thread_ghost, 0, sizeof (micro_os_plus_thread_t));

          thread_ghost.name = "ghost";

          // Make the ghost thread look like the current thread.
          rtos::scheduler::current_thread_
              = reinterpret_cast<rtos::thread*> (&thread_ghost);

          // Trigger the PendSV; the exception will happen a bit later,
          // after re-enabling the interrupts.
          scheduler::reschedule ();

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

          // Disable the base priority (allow all interrupts).
          __set_BASEPRI (0);

#endif

          lock_state = state::init;

          // Enable all interrupts; allow PendSV to occur.
          __enable_irq ();

          // The context switch will occur somewhere here.
          for (;;)
            __NOP ();

          /* NOTREACHED */
        }

        // --------------------------------------------------------------------

        state_t
        locked (state_t state)
        {
          micro_os_plus_assert_throw (!interrupts::in_handler_mode (), EPERM);

          state_t tmp;

          {
            rtos::interrupts::critical_section ics;

            tmp = lock_state;
            lock_state = state;
          }

          return tmp;
        }

        // --------------------------------------------------------------------

        /**
         * @brief Reschedule the next thread.
         *
         * @details
         * Thanks to the Cortex-M architecture, this is greatly
         * simplified by the use of PendSV, which does the actual
         * context switching.
         *
         * This routine only sets the PendSV request, the actual
         * rescheduling is done in the PendSV_Handler.
         *
         * If interrupts are disabled, it can be invoked multiple
         * times without a problem, the handler will be entered
         * when interrupts are enabled.
         *
         * @note Can be invoked from Interrupt Service Routines.
         */

        void
        reschedule (void)
        {
          if (rtos::scheduler::locked ()
              || (rtos::interrupts::in_handler_mode ()
                  && !rtos::scheduler::preemptive ()))
            {
#if defined(MICRO_OS_PLUS_TRACE_RTMICRO_OS_PLUS_THREAD_CONTEXT)
              trace::printf ("port::scheduler::%s() %s nop\n", __func__,
                             rtos::scheduler::current_thread_->name ());
#endif
              return;
            }

#if defined(MICRO_OS_PLUS_TRACE_RTMICRO_OS_PLUS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s()\n", __func__);
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

          // Set PendSV to request a context switch.
          SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

#pragma GCC diagnostic pop

          // The DSB/ISB are recommended by ARM after programming
          // the control registers.
          // (Application Note 321: ARM Cortex-M Programming Guide to
          // Memory Barrier Instructions)
          // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf
          __DSB ();
          __ISB ();
        }

        // --------------------------------------------------------------------

// warning: ISO C++17 does not allow 'register' storage class specifier
// [-Wregister]
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister"

        /**
         * @brief Save the current thread context on stack.
         * @return The SP after saving the context.
         *
         * @details
         * Get the current thread stack (PSP) and on it save R4-R11
         * and possibly the FPU registers.
         *
         * The memory address after saving these registers is returned and
         * will be saved by switch_stacks() in the current thread context.
         *
         * @warning Any change in this routine must be checked in the
         * generated code, otherwise bad surprises can occur, like
         * adding clobber registers that added more initial pushes.
         */

        inline __attribute__ ((always_inline)) stack::element_t*
        save_on_stack (void)
        {
          register stack::element_t* sp_;

          __asm__ volatile(
              // Get the thread stack
              " mrs %[r], PSP                       \n"
              " isb                                 \n"

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(__ARM_FP)

              // Is the thread using the FPU context?
              " tst lr, #0x10                       \n"
              " it eq                               \n"
              // If so, push high vfp registers.
              " vstmdbeq %[r]!, {s16-s31}           \n"

#endif

              // Save the core registers r4-r11,r14. Decrement [r].
              // The EXC_RETURN may be tested again for FP
              // in the restore sequence.
              " stmdb %[r]!, {r4-r9,sl,fp,lr}       \n"

#elif defined(__ARM_ARCH_6M__)

              // With decrement not available, manually
              // move the pointer 9 words below.
              " sub %[r], %[r], #36                 \n"

              // Save the core registers r4-r7. Increment [r].
              " stmia %[r]!, {r4-r7}                \n"

              // Move the core registers r8-r11 to lower registers.
              " mov r3, r8                          \n"
              " mov r4, r9                          \n"
              " mov r5, sl                          \n"
              " mov r6, fp                          \n"
              " mov r7, lr                          \n"
              // Save the core registers r8-r11,lr. Increment [r].
              " stmia %[r]!, {r3-r7}                \n"

              // Move [r] down, to cancel increment.
              " sub %[r], %[r], #36                 \n"

#else

#error Implement registers save on this architecture.

#endif

              : [r] "=r"(sp_) /* out */
              : /* in */
              : /* clobber. DO NOT add anything here! */
          );

          return sp_;
        }

        /**
         * @brief Restore the new thread from the given stack address.
         * @param [in] sp Address where the thread context was saved.
         *
         * @details
         * Restore R4-R11 and possibly the FPU registers.
         * Finally write the current stack address in PSP.
         *
         * @warning Any change in this routine must be checked in the
         * generated code, otherwise bad surprises can occur, like
         * adding clobber registers that added more initial pushes.
         */

        inline __attribute__ ((always_inline)) void
        restore_from_stack (stack::element_t* sp)
        {
          // Without enforcing optimisations, an intermediate variable
          // would be needed to avoid using R4, which collides with
          // the R4 in the list of ldmia.

          // register stack::element_t* sp_ asm ("r0") = sp;

          __asm__ volatile(

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

              // Restore the core registers r4-r11,r14. Increment [r].
              // R14 contains the EXC_RETURN value
              // and may be used in the FP test.
              " ldmia %[r]!, {r4-r9,sl,fp,lr}       \n"

#if defined(__ARM_FP)

              // Is the thread using the FPU context?
              " tst lr, #0x10                       \n"
              " it eq                               \n"
              // If so, pop the high vfp registers too.
              " vldmiaeq %[r]!, {s16-s31}           \n"

#endif

#elif defined(__ARM_ARCH_6M__)

              // Move [r] to upper registers location (4*4).
              " add %[r], %[r], #16                 \n"

              // Restore the core registers r8-r11,lr to lower registers.
              // Increment [r].
              " ldmia %[r]!, {r3-r7}                \n"

              // Move lower registers to upper registers r8-r11.
              " mov r8, r3                          \n"
              " mov r9, r4                          \n"
              " mov sl, r5                          \n"
              " mov fp, r6                          \n"
              " mov lr, r7                          \n"

              // Move [r] down, to lower registers location (9*4).
              " sub %[r], %[r], #36                 \n"

              // Restore the core registers r4-r7. Increment [r].
              " ldmia %[r]!, {r4-r7}                \n"

              // Move [r] up, after upper registers (5*4).
              " add %[r], %[r], #20                 \n"

#else

#error Implement registers restore on this architecture.

#endif

              // Restore the thread stack register.
              " msr PSP, %[r]                       \n"
              " isb                                 \n"

              : /* out */
              : [r] "r"(sp) /* in */
              : /* clobber. DO NOT add anything here! */
          );
        }

#pragma GCC diagnostic pop

        /**
         * @brief Switch stacks to perform the rescheduling.
         * @param [in] sp Pointer to the initial thread context.
         * @return Pointer to the new thread context.
         *
         * @details
         * This function is the heart of the scheduler and
         * performs the context switches. It is called from the
         * PendSV_Handler.
         *
         * The main purpose of this function is to:
         * - remember the SP address in the thread context
         * - add the running thread to the ready list
         * - select the next thread
         * - get the new SP from there
         *
         * To protect the internal lists, interrupts are partly
         * disabled by a local critical section.
         */

        stack::element_t*
        switch_stacks (stack::element_t* sp)
        {
          uint32_t pri;

          // Enter a local critical section to protect the lists.
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          pri = __get_BASEPRI ();
          __set_BASEPRI_MAX (
              MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
              << ((8 - __NVIC_PRIO_BITS)));

#else

          // Read the current PRIMASK, to be returned and later restored.
          pri = __get_PRIMASK ();

          // Disable all interrupts.
          __disable_irq ();

#endif // defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

#elif defined(__ARM_ARCH_6M__)

          // Read the current PRIMASK, to be returned and later restored.
          pri = __get_PRIMASK ();

          // Disable all interrupts.
          __disable_irq ();

#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

          // Clear the PendSV bit. This is done automatically,
          // but it seems that in some extreme conditions with
          // late arrival/preemption, it might trigger a new
          // exception.
          // (http://embeddedgurus.com/state-space/2011/09/whats-the-state-of-your-cortex/)
          // This shouldn't be a problem for this scheduler,
          // but without it the semaphore stress test sometimes fails
          // on debug, so better safe than sorry.
          SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;

#pragma GCC diagnostic pop

          rtos::thread* old_thread = rtos::scheduler::current_thread_;

          // Save the current SP in the initial context.
          old_thread->context_.port_.stack_ptr = sp;

#if defined(MICRO_OS_PLUS_TRACE_RTMICRO_OS_PLUS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s() leave %s\n", __func__,
                         old_thread->name ());
#endif

          rtos::scheduler::internal_switch_threads ();

#if defined(MICRO_OS_PLUS_TRACE_RTMICRO_OS_PLUS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s() to %s\n", __func__,
                         rtos::scheduler::current_thread_->name ());
#endif

          // Prepare a local copy of the new thread SP.
          stack::element_t* out_sp
              = rtos::scheduler::current_thread_->context_.port_.stack_ptr;

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          // Restore BASEPRI to the value saved at the beginning.
          __set_BASEPRI (pri);

#else

          // Restore PRIMASK to the value saved at the beginning.
          __set_PRIMASK (pri);

#endif // defined(MICRO_OS_PLUS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

#elif defined(__ARM_ARCH_6M__)

          // Restore PRIMASK to the value saved at the beginning.
          __set_PRIMASK (pri);

#endif

          // Return the new thread SP. Registers will be
          // restored in the assembly code.
          return out_sp;
        }

        // --------------------------------------------------------------------
      } // namespace scheduler
    } // namespace port
  } // namespace rtos
} // namespace micro_os_plus

// ----------------------------------------------------------------------------

/**
 * @brief PendSV exception handler.
 *
 * @details
 * The PendSV_Handler is used to perform the context switches.
 * PendSV should be configured with the lowest priority; it will
 * execute right after interrupts are enabled or after the last
 * interrupt is terminated if invoked from an ISR context.
 *
 * Its main purpose is to:
 * - suspend the execution of the current thread
 * - save all registers on the thread stack (using PSP)
 * - switch stacks
 * - restore all registers from the new stack
 * - resume execution of the new thread
 *
 * The `optimize("1")` is needed to avoid the nightmarish code
 * generated with `-O0`.
 *
 * Without it, this function would have been naked and LR have been
 * manually pushed/popped because the inlined functions include
 * variables, and on `-O0` they always get allocated local stack space,
 * which manipulates R7, and R7 is not saved in the exception frame,
 * so this would unnecessarily complicate things to recover it from MSP.
 *
 * @note To be observed here is the use of two stacks.
 * Being an exception handler, the regular push/pop in the function
 * entry/exit code use MSP. However, part of the exception
 * frame, R0-R3 are automatically saved on PSP. R4-R11 must be
 * manually saved/restored.
 */

// clang-format off
/*
 * An actual version of the Cortex-M3 handler looks pretty cool:
 *
 * 800019a <PendSV_Handler>:
 * 800019a: f3ef 8009   mrs r0, PSP
 * 800019e: f3bf 8f6f   isb sy
 * 80001a2: e920 0ff0   stmdb r0!, {r4, r5, r6, r7, r8, r9, sl, fp}
 * 80001a6: f000 fe21   bl  8000dec <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)>
 * 80001aa: e8b0 0ff0   ldmia.w r0!, {r4, r5, r6, r7, r8, r9, sl, fp}
 * 80001ae: f380 8809   msr PSP, r0
 * 80001b2: f3bf 8f6f   isb sy
 * 80001b6: 4770        bx  lr
 *
 * The switch_stack() function (using BASEPRI) doesn't look bad either:
 *
 * 8000d88 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)>:
 * 8000d88: b538        push  {r3, r4, r5, lr}
 * 8000d8a: f3ef 8511   mrs r5, BASEPRI
 * 8000d8e: 2340        movs  r3, #64 ; 0x40
 * 8000d90: f383 8812   msr BASEPRI_MAX, r3
 * 8000d94: 4b0c        ldr r3, [pc, #48] ; (8000dc8 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x40>)
 * 8000d96: f04f 6200   mov.w r2, #134217728  ; 0x8000000
 * 8000d9a: 4c0c        ldr r4, [pc, #48] ; (8000dcc <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x44>)
 * 8000d9c: 605a        str r2, [r3, #4]
 * 8000d9e: 6821        ldr r1, [r4, #0]
 * 8000da0: f891 3058   ldrb.w  r3, [r1, #88] ; 0x58
 * 8000da4: 66c8        str r0, [r1, #108]  ; 0x6c
 * 8000da6: 2b03        cmp r3, #3
 * 8000da8: d105        bne.n 8000db6 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x2e>
 * 8000daa: 68cb        ldr r3, [r1, #12]
 * 8000dac: b91b        cbnz  r3, 8000db6 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x2e>
 * 8000dae: 3108        adds  r1, #8
 * 8000db0: 4807        ldr r0, [pc, #28] ; (8000dd0 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x48>)
 * 8000db2: f003 f935   bl  8004020 <micro_os_plus::rtos::ready_threads_list::link(micro_os_plus::rtos::waiting_thread_node&)>
 * 8000db6: 4806        ldr r0, [pc, #24] ; (8000dd0 <micro_os_plus::rtos::port::scheduler::switch_stacks(unsigned long*)+0x48>)
 * 8000db8: f003 f960   bl  800407c <micro_os_plus::rtos::ready_threads_list::unlink_head()>
 * 8000dbc: 6020        str r0, [r4, #0]
 * 8000dbe: 6823        ldr r3, [r4, #0]
 * 8000dc0: 6ed8        ldr r0, [r3, #108]  ; 0x6c
 * 8000dc2: f385 8811   msr BASEPRI, r5
 * 8000dc6: bd38        pop {r3, r4, r5, pc}
 * 8000dc8: e000ed00  .word 0xe000ed00
 * 8000dcc: 20002ac4  .word 0x20002ac4
 * 8000dd0: 20002abc  .word 0x20002abc
 *
 * In case you wonder, the link() and unlink() functions are also
 * relatively simple, with link() slightly more complicated, since
 * it needs to preserve the ready list order.
 */
// clang-format on

void __attribute__ ((section (".after_vectors"), naked, used, optimize ("1")))
PendSV_Handler (void)
{
  // The naked attribute is used to fully control the function entry/exit
  // code; be sure other registers are not used in the assembly parts.
  // Do NOT use push/pop, since the LR is from a different context.

  // The whole mystery of context switching, in one sentence. :-)
  port::scheduler::restore_from_stack (
      port::scheduler::switch_stacks (port::scheduler::save_on_stack ()));

  __asm__ volatile("bx lr");
}

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_INCLUDE_RTOS)

// ----------------------------------------------------------------------------

#endif // defined(__ARM_EABI__)

// ----------------------------------------------------------------------------
