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
 * Implementation routines for the CMSIS++ reference scheduler, mainly
 * the context switching and context creation.
 */

#if defined(__ARM_EABI__)

// ----------------------------------------------------------------------------

#include <string.h>
#include <cassert>

#include <cmsis-plus/rtos/os.h>

#include <cmsis-plus/cortexm/exception-handlers.h>

#include <cmsis_device.h>

// ----------------------------------------------------------------------------

#if !defined(__ARM_ARCH_7M__) && !defined(__ARM_ARCH_7EM__) && !defined(__ARM_ARCH_6M__)
#error "Architecture not supported."
#endif

#if defined(__ARM_ARCH_6M__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)
#error "OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY should not be used with Cortex-M0/M0+ devices."
#endif

#endif /* defined(__ARM_ARCH_6M__) */

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

#define __MAX_PRIO ((1 << __NVIC_PRIO_BITS) -1)

static_assert (OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY != 0,
    "OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY cannot be 0");
static_assert (OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY <= __MAX_PRIO,
    "OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY must be <= "
    OS_MACRO_STRINGIFY(__MAX_PRIO));

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#endif /* defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) */

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_6M__)
extern uint32_t __vectors_start;
#endif

// ----------------------------------------------------------------------------

extern "C"
{
  extern unsigned int _Heap_Limit;
  extern unsigned int __stack;
}

// ----------------------------------------------------------------------------

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace stack
      {
        // Stack frame, as used by PendSV.
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
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
          stack::element_t r14_exec_return;
#endif
          // Restored automatically by exception return.
          stack::element_t r0;
          stack::element_t r1;
          stack::element_t r2;
          stack::element_t r3;

          stack::element_t r12;
          stack::element_t r14_lr; // r14
          stack::element_t r15_pc; // r15
          stack::element_t psr;
        } frame_t;
      } /* namespace stack */

      /**
       * @brief Create a new thread context on the stack.
       * @param [in] context Pointer to thread context.
       * @param [in] func Pointer to function to execute in the new context.
       * @param [in] args Function arguments.
       *
       * @details
       * Initialise the stack with a repetitive pattern; create an
       * exception stack frame (at stack top) such that a later
       * PendSV will pass control to the new context.
       */
      void
      context::create (void* context, void* func, void* args)
      {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
        trace::printf ("port::context::%s(%p)\n", __func__, context);
#endif
        class rtos::thread::context* th_ctx =
            static_cast<class rtos::thread::context*> (context);

        class rtos::thread::stack& stack = th_ctx->stack ();

        rtos::thread::stack::element_t* p = stack.top ();

        // Be sure the stack is large enough to hold at least
        // two exception frames.
        assert((p - stack.bottom ()) > (int )(2 * sizeof(stack::frame_t)));

        p -= (sizeof(stack::frame_t) / sizeof(rtos::thread::stack::element_t));

        // Align the frame to 8 bytes.
        if (((int) p & 7) != 0)
          {
            --p;
          }

        // For convenience, use the stack frame structure.
        stack::frame_t* f = reinterpret_cast<stack::frame_t*> (p);

        // The stack as if after a context save.

        // Thread starts in thumb state (T bit set).
        f->psr = 0x01000000; // xPSR +15*4=64

        // The address of the trampoline code. // PC/R15 +14*4=60
        f->r15_pc =
            (rtos::thread::stack::element_t) (((ptrdiff_t) func) & (~1));

        // Link register // LR/R14 +13*4=56
#if defined(OS_BOOL_RTOS_PORT_CONTEXT_CREATE_ZERO_LR)
        f->r14_lr = 0x00000000;
#else
        // 0x0 looks odd in the debugger, so try to hide it.
        // In Eclipse using 'func+2' will make the stack trace
        // start with 'func' (don't ask why).
        f->r14_lr = (rtos::thread::stack::element_t) (((ptrdiff_t) func + 2));
#endif
        // R13 is the SP; it is not present in the frame,
        // it is loaded separately as PSP.

        f->r12 = 0xCCCCCCCC;  // R12 +12*4=52

        // According to ARM ABI, the first 4 word parameters are
        // passed in R0-R3. Only 1 is used.
        f->r3 = 0x33333333; // R3 +11*4=48
        f->r2 = 0x22222222; // R2 +10*4=44
        f->r1 = 0x11111111; // R1 +9*4=40
        f->r0 = (rtos::thread::stack::element_t) args; // R0 +8*4=36

#if defined (__VFP_FP__) && !defined (__SOFTFP__)
        // This frame does not include initial FPU registers.
        // bit 4: 1 (8 words), 0 (26 words)
        // bit 3: 1 (return to thread), 0 (return to handler)
        // bit 2: 1 (return with PSP), 0 (return with MSP)
        // bit 1 = 0
        // bit 0 = 1
        f->r14_exec_return = 0xFFFFFFFD;
#endif

        f->r11_fp = 0xBBBBBBBB; // R11 +7*4=32
        f->r10_sl = 0xAAAAAAAA; // R10 +6*4=28
        f->r9 = 0x99999999; // R9 +5*4=24
        f->r8 = 0x88888888; // R8 +4*4=20
        f->r7 = 0x77777777; // R7 +3*4=16
        f->r6 = 0x66666666; // R6 +2*4=12
        f->r5 = 0x55555555; // R5 +1*4=8
        f->r4 = 0x44444444; // R4 +0*4=4

        // Store the current stack pointer in the context.
        th_ctx->port_.stack_ptr = p;
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
        assert(
            SysTick_Config (SystemCoreClock / rtos::clock_systick::frequency_hz)
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
         */

        void
        start (void)
        {
#if defined(OS_TRACE_RTOS_SCHEDULER)
          trace::printf ("port::scheduler::%s() \n", __func__);
#endif

#if defined (__VFP_FP__) && !defined (__SOFTFP__)
          // The FPU should have been enabled by
          // os_startup_initialize_hardware_early().
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

          __set_MSP (*((uint32_t*) SCB->VTOR));

#elif defined(__ARM_ARCH_6M__)

          // VTOR not available on this architecture.
          // Read the stack pointer from the first word
          // stored in the vectors table.
          __set_MSP (*((uint32_t*) (0x00000000)));

#else

#error Implement __set_MSP() on this architecture.

#endif

          // Set the beginning address and size of the interrupt stack.
          rtos::interrupts::stack()->set (
              reinterpret_cast<stack::element_t*> (&_Heap_Limit),
              (&__stack - &_Heap_Limit) * sizeof(__stack));

          // Set PendSV interrupt priority to the lowest level (highest value).
          NVIC_SetPriority (PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

          // One disadvantage of this simple method is that
          // the first context switch will want to save the initial SP
          // somewhere, so prepare a fake thread context.
          // Don't worry for being on the stack, this is used
          // only once and can be overridden later.
          os_thread_t fake_thread;
          memset (&fake_thread, 0, sizeof(os_thread_t));

          fake_thread.name = "fake_thread";
          rtos::thread* pth = (rtos::thread*) &fake_thread;

          // Make the fake thread look like the current thread.
          rtos::scheduler::current_thread_ = pth;

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
          os_assert_throw(!interrupts::in_handler_mode (), EPERM);

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
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
              trace::printf ("port::scheduler::%s() %s nop\n", __func__,
                  rtos::scheduler::current_thread_->name ());
#endif
              return;
            }

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s()\n", __func__);
#endif
          // Set PendSV to request a context switch.
          SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

          // The DSB/ISB are recommended by ARM after programming
          // the control registers.
          // (Application Note 321: ARM Cortex-M Programming Guide to
          // Memory Barrier Instructions)
          // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf
          __DSB ();
          __ISB ();
        }

        // --------------------------------------------------------------------

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

        inline stack::element_t*
        __attribute__((always_inline))
        save_on_stack (void)
        {
          register stack::element_t* sp_;

          asm volatile
          (
              // Get the thread stack
              " mrs %[r], PSP                       \n"
              " isb                                 \n"

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined (__VFP_FP__) && !defined (__SOFTFP__)

              // Is the thread using the FPU context?
              " tst lr, #0x10                       \n"
              " it eq                               \n"
              // If so, push high vfp registers.
              " vstmdbeq %[r]!, {s16-s31}           \n"
              // Save the core registers r4-r11,r14.
              // Also save EXC_RETURN to be able to test
              // again this condition in the restore sequence.
              " stmdb %[r]!, {r4-r9,sl,fp,lr}       \n"

#else

              // Save the core registers r4-r11. Decrement [r].
              " stmdb %[r]!, {r4-r9,sl,fp}          \n"

#endif

#elif defined(__ARM_ARCH_6M__)

              // With decrement not available, manually
              // move the pointer 8 words below.
              " sub %[r], %[r], #32                 \n"

              // Save the core registers r4-r7. Increment [r].
              " stmia %[r]!, {r4-r7}                \n"

              // Move the core registers r8-r11 to lower registers.
              " mov r4, r8                          \n"
              " mov r5, r9                          \n"
              " mov r6, sl                          \n"
              " mov r7, fp                          \n"
              // Save the core registers r8-r11. Increment [r].
              " stmia %[r]!, {r4-r7}                \n"

              // Move [r] down, to cancel increment.
              " sub %[r], %[r], #32                 \n"

#else

#error Implement registers save on this architecture.

#endif

              : [r] "=r" (sp_) /* out */
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

        inline void
        __attribute__((always_inline))
        restore_from_stack (stack::element_t* sp)
        {
          // Without enforcing optimisations, an intermediate variable
          // would be needed to avoid using R4, which collides with
          // the R4 in the list of ldmia.

          // register stack::element_t* sp_ asm ("r0") = sp;

          asm volatile
          (

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined (__VFP_FP__) && !defined (__SOFTFP__)

              // Pop the core registers r4-r11,r14.
              // R14 contains the EXC_RETURN value
              // and is restored for the next test.
              " ldmia %[r]!, {r4-r9,sl,fp,lr}       \n"
              // Is the thread using the FPU context?
              " tst lr, #0x10                       \n"
              " it eq                               \n"
              // If so, pop the high vfp registers too.
              " vldmiaeq %[r]!, {s16-s31}           \n"

#else

              // Restore the core registers r4-r11. Increment [r].
              " ldmia %[r]!, {r4-r9,sl,fp}          \n"

#endif

#elif defined(__ARM_ARCH_6M__)

              // Move [r] to upper registers location.
              " add %[r], %[r], #16                 \n"

              // Restore the core registers r8-r11 to lower registers.
              // Increment [r].
              " ldmia %[r]!, {r4-r7}                \n"

              // Move lower registers to upper registers r8-r11.
              " mov r8, r4                          \n"
              " mov r9, r5                          \n"
              " mov sl, r6                          \n"
              " mov fp, r7                          \n"

              // Move [r] down, to lower registers location.
              " sub %[r], %[r], #32                 \n"

              // Restore the core registers r4-r7. Increment [r].
              " ldmia %[r]!, {r4-r7}                \n"

              // Move [r] up, after upper registers.
              " add %[r], %[r], #16                 \n"

#else

#error Implement registers restore on this architecture.

#endif

              // Restore the thread stack register.
              " msr PSP, %[r]                       \n"
              " isb                                 \n"

              : /* out */
              : [r] "r" (sp) /* in */
              : /* clobber. DO NOT add anything here! */
          );
        }

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

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          pri = __get_BASEPRI ();
          __set_BASEPRI_MAX (
              OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
                  << ((8 - __NVIC_PRIO_BITS)));

#else

          // Read the current PRIMASK, to be returned and later restored.
          pri = __get_PRIMASK ();

          // Disable all interrupts.
          __disable_irq ();

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#elif defined(__ARM_ARCH_6M__)

          // Read the current PRIMASK, to be returned and later restored.
          pri = __get_PRIMASK ();

          // Disable all interrupts.
          __disable_irq ();

#endif

          // Clear the PendSV bit. This is done automatically,
          // but it seems that in some extreme conditions with
          // late arrival/preemption, it might trigger a new
          // exception.
          // (http://embeddedgurus.com/state-space/2011/09/whats-the-state-of-your-cortex/)
          // This shouldn't be a problem for this scheduler,
          // but without it the semaphore stress test sometimes fails
          // on debug, so better safe than sorry.
          SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;

          rtos::thread* old_thread = rtos::scheduler::current_thread_;

          // Save the current SP in the initial context.
          old_thread->context_.port_.stack_ptr = sp;

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s() leave %s\n", __func__, old_thread->name ());
#endif

          rtos::scheduler::internal_switch_threads ();

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("port::scheduler::%s() to %s\n", __func__,
              rtos::scheduler::current_thread_->name ());
#endif

          // Prepare a local copy of the new thread SP.
          stack::element_t* out_sp =
              rtos::scheduler::current_thread_->context_.port_.stack_ptr;

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          // Restore BASEPRI to the value saved at the beginning.
          __set_BASEPRI (pri);

#else

          // Restore PRIMASK to the value saved at the beginning.
          __set_PRIMASK (pri);

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#elif defined(__ARM_ARCH_6M__)

          // Restore PRIMASK to the value saved at the beginning.
          __set_PRIMASK (pri);

#endif

          // Return the new thread SP. Registers will be
          // restored in the assembly code.
          return out_sp;
        }

      // ----------------------------------------------------------------------

      } /* namespace scheduler */
    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

using namespace os::rtos;

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
 * The `optimize("s")` is needed to avoid the nightmarish code
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

/*
 * An actual version of the Cortex-M3 handler looks pretty cool:
 *
 * 08000198 <PendSV_Handler>:
 * 8000198: b500        push  {lr}
 * 800019a: f3ef 8009   mrs r0, PSP
 * 800019e: f3bf 8f6f   isb sy
 * 80001a2: e920 0ff0   stmdb r0!, {r4, r5, r6, r7, r8, r9, sl, fp}
 * 80001a6: f000 fe21   bl  8000dec <os::rtos::port::scheduler::switch_stacks(unsigned long*)>
 * 80001aa: e8b0 0ff0   ldmia.w r0!, {r4, r5, r6, r7, r8, r9, sl, fp}
 * 80001ae: f380 8809   msr PSP, r0
 * 80001b2: f3bf 8f6f   isb sy
 * 80001b6: bd00        pop {pc}
 *
 * The switch_stack() function (using BASEPRI) doesn't look bad either:
 *
 * 08000d88 <os::rtos::port::scheduler::switch_stacks(unsigned long*)>:
 * 8000d88: b538        push  {r3, r4, r5, lr}
 * 8000d8a: f3ef 8511   mrs r5, BASEPRI
 * 8000d8e: 2340        movs  r3, #64 ; 0x40
 * 8000d90: f383 8812   msr BASEPRI_MAX, r3
 * 8000d94: 4b0c        ldr r3, [pc, #48] ; (8000dc8 <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x40>)
 * 8000d96: f04f 6200   mov.w r2, #134217728  ; 0x8000000
 * 8000d9a: 4c0c        ldr r4, [pc, #48] ; (8000dcc <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x44>)
 * 8000d9c: 605a        str r2, [r3, #4]
 * 8000d9e: 6821        ldr r1, [r4, #0]
 * 8000da0: f891 3058   ldrb.w  r3, [r1, #88] ; 0x58
 * 8000da4: 66c8        str r0, [r1, #108]  ; 0x6c
 * 8000da6: 2b03        cmp r3, #3
 * 8000da8: d105        bne.n 8000db6 <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x2e>
 * 8000daa: 68cb        ldr r3, [r1, #12]
 * 8000dac: b91b        cbnz  r3, 8000db6 <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x2e>
 * 8000dae: 3108        adds  r1, #8
 * 8000db0: 4807        ldr r0, [pc, #28] ; (8000dd0 <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x48>)
 * 8000db2: f003 f935   bl  8004020 <os::rtos::ready_threads_list::link(os::rtos::waiting_thread_node&)>
 * 8000db6: 4806        ldr r0, [pc, #24] ; (8000dd0 <os::rtos::port::scheduler::switch_stacks(unsigned long*)+0x48>)
 * 8000db8: f003 f960   bl  800407c <os::rtos::ready_threads_list::unlink_head()>
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

void
__attribute__ ((section(".after_vectors"), naked, used, optimize("s")))
PendSV_Handler (void)
{
  // The naked attribute and the push/pop are used to fully control
  // the function entry/exit code; be sure other registers are not
  // used in the assembly parts.
  asm volatile ("push {lr}");

  // The whole mystery of context switching, in one sentence. :-)
  port::scheduler::restore_from_stack (
      port::scheduler::switch_stacks (port::scheduler::save_on_stack ()));

  asm volatile ("pop {pc}");
}

// ----------------------------------------------------------------------------

#endif /* defined(__ARM_EABI__) */
