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

#include <cassert>

#include <cmsis-plus/rtos/os.h>
#include <cmsis-plus/rtos/port/os-inlines.h>

// Better be the last, to undef putchar()
#include <cmsis-plus/diag/trace.h>

#include <cmsis-plus/iso/malloc.h>

#include <cmsis_device.h>

/*
 * After power up, the processor hardware automatically
 * initialises the MSP by reading the vector table.
 * The core has CONTROL.SPSEL(bit2) = 0, always using MSP.
 *
 */

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace thread
      {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

        void
        Context::create (rtos::thread::Context* context, void* func, void* args)
        {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s(%p)\n", __func__, context);
#endif
          rtos::thread::Stack& stack = context->stack ();
          rtos::stack::element_t* bottom = stack.bottom ();

          rtos::stack::element_t* p;

          // Initialise the entire stack with the magic word.
          for (p = bottom;
              p < bottom + stack.size () / sizeof(rtos::stack::element_t); ++p)
            {
              *p = stack::magic;
            }

          // Compute top of stack.
          p = bottom + stack.size () / sizeof(rtos::stack::element_t);

          // This magic should always be present here. If it is not,
          // someone else damaged the thread stack.
          *--p = stack::magic;           // magic

          // To be safe, align the stack frames to 8. In total there are
          // 16 words to store, so if the current address is not even,
          // descend an extra word.
          if (((int) p & 7) != 0)
            {
              *--p = stack::magic;       // one more magic
            }

          // Simulate the Cortex-M exception stack frame, i.e. how the stack
          // would look after a call to yield().

          // Thread starts with interrupts enabled (T bit set).
          *--p = 0x01000000; // xPSR +15*4=64

          // The address of the trampoline code.
          *--p = (rtos::stack::element_t) (((ptrdiff_t) func) & (~1)); // PCL +14*4=60

          // The stack as if after a context save.

          // Link register
          *--p = 0x00000000; // LR   +13*4=56
          *--p = 12;  // R12  +12*4=52

          // According to ARM ABI, the first 4 word parameters are
          // passed in R0-R3. Only 1 is used.
          *--p = 3; // R3 +11*4=48
          *--p = 2; // R2 +10*4=44
          *--p = 1; // R1 +9*4=40
          *--p = (rtos::stack::element_t) args; // R0 +8*4=36

          *--p = 11; // R11  +7*4=32
          *--p = 10; // R10  +6*4=28
          *--p = 9; // R9 +5*4=24
          *--p = 8; // R8 +4*4=20
          *--p = 7; // R7 +3*4=16
          *--p = 6; // R6 +2*4=12
          *--p = 5; // R5 +1*4=8
          *--p = 4; // R4 +0*4=4

          // Be sure the stack is large enough to hold at least
          // two more exception frames.
          assert((p - bottom) > (2 * 16));

          // Store the current stack pointer in the context.
          context->port_.stack_ptr = p;
        }

#pragma GCC diagnostic pop

      } /* namespace thread */

      namespace scheduler
      {

        rtos::stack::element_t** stack_ptr_ptr;

        void
        start (void)
        {
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
          // The FPU should have been enabled by __initialize_hardware_early().
#endif

          // The traditional way to switch to start
          // the first thread is to use SVC 0.

          // However, if the entire OS remains in privileged mode,
          // this seems not necessary, and Chapter 12.9.1
          // "Running a system with two stacks" in "The Definitive
          // Guide to ARM Cortex!-M3 and Cortex-M4 Processors" by
          // Joseph Yiu, provides a simpler solution.

          // Disable all interrupts, to safely change the stack.
          __disable_irq ();

          // The main trick is to switch the current SP from MSP to PSP
          // without breaking the running code. This is simply done by
          // initialising PSP to be the same as MSP.
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
#if defined(__ARM_ARCH_7M__)
          __set_MSP (*((uint32_t*) SCB->VTOR));
#else
#error VTOR not available on this architecture!
#endif

          // Set PendSV interrupt priority to the lowest level (highest value).
          NVIC_SetPriority (PendSV_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

          // The first context switch will want to save the initial SP
          // somewhere, so prepare a fake pointer.
          stack::element_t* fake;
          stack_ptr_ptr = &fake;

          // Trigger the PendSV; the exception will happen later,
          // after re-enabling the interrupts.
          scheduler::reschedule ();

          // Disable the base priority (allow all interrupts).
#if defined(__ARM_ARCH_7M__)
          __set_BASEPRI (0);
#endif

          // Enable all interrupts; allow PendSV to occur.
          __enable_irq ();

          // The context switch should occur somewhere here.
          for (;;)
            __NOP ();

          /* NOTREACHED */
        }

        // The DSB/ISB are recommended by ARM after after programming
        // the control registers.
        // (Application Note 321: ARM Cortex-M Programming Guide to
        // Memory Barrier Instructions)
        // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf

        // Must be called in a critical section.
        void
        reschedule (bool save __attribute__((unused)))
        {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s()\n", __func__);
#endif
          if (rtos::scheduler::locked ())
            {
              return;
            }

          // Set PendSV to request a context switch.
          SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

          __DSB ();
          __ISB ();
        }

        // Called from PendSV_Handler to perform the context switch.
        void
        get_next_context (void)
        {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s() leave %s (pps=%p,%p)\n", __func__,
                         rtos::scheduler::current_thread_->name (),
                         stack_ptr_ptr, *stack_ptr_ptr);
#endif
          // Clear the PendSV bit.
          SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;

          // Determine the next thread.
          rtos::scheduler::current_thread_ =
              rtos::scheduler::ready_threads_list_.remove_top ();

          // Update the pointer to the thread stack.
          stack_ptr_ptr =
              &rtos::scheduler::current_thread_->context ().port_.stack_ptr;

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s() in %s (pps=%p,%p)\n", __func__,
                         rtos::scheduler::current_thread_->name (),
                         stack_ptr_ptr, *stack_ptr_ptr);
#endif
        }

      } /* namespace scheduler */

      void
      Systick_clock::start (void)
      {
        assert(
            SysTick_Config (SystemCoreClock / rtos::Systick_clock::frequency_hz) == 0);

        // Set SysTick interrupt priority to the lowest level (highest value).
        NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
      }

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

extern "C" void
PendSV_Handler (void);

void
__attribute__ ((section(".after_vectors"), naked, used))
PendSV_Handler (void)
{
  // Credits: inspired by FreeRTOS Cortex-M3/M4 xPortPendSVHandler().

  asm volatile
  (
      // Registers R0-R3 are saved as part of the exception frame
      // and are free to use. The rest must be manually saved/restored.

      "       mrs r0, PSP                         \n"// Get the main stack in R0
      "       isb                                 \n"
      "                                           \n"
      "       ldr r3, ppstack                     \n"// Get the address of stack_ptr_ptr.
      "       ldr r2, [r3, #0]                    \n"// Get the address of the context stack pointer.
      "                                           \n"
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
      "       tst r14, #0x10                      \n" // Is the task using the FPU context?
      "       it eq                               \n"
      "       vstmdbeq r0!, {s16-s31}             \n"// If so, push high vfp registers.
#endif
      "                                           \n"
      "       stmdb r0!, {r4-r9,sl,fp}            \n" // Save the core registers r4-r11.
      "                                           \n"
      "       str r0, [r2, #0]                    \n"// Save the new top of stack into the context.
      "                                           \n"
      "       stmdb sp!, {r3, lr}                 \n"
      "                                           \n"
      "       mov r0, %[pri]                      \n"// Disable interrupts allowed to make system calls
      "       msr BASEPRI, r0                     \n"
      "                                           \n"
      "       bl %[gnc]                           \n"// get_next_context()
      "                                           \n"
      "       mov r0, #0                          \n"// Setting a value of 0 will cancel masking completely,
      "       msr BASEPRI, r0                     \n"// enabling all interrupts.
      "                                           \n"
      "       ldmia sp!, {r3, lr}                 \n"
      "                                           \n"
      "       ldr r1, [r3, #0]                    \n"// At *stack_ptr_ptr is the new stack pointer.
      "       ldr r0, [r1, #0]                    \n"// Get the SP from context.
      "                                           \n"
      "       ldmia r0!, {r4-r9,sl,fp}            \n"// Pop the core registers r4-r11.
      "                                           \n"
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
      "       tst r14, #0x10                      \n" // Is the task using the FPU context?
      "       it eq                               \n"
      "       vldmiaeq r0!, {s16-s31}             \n"// If so, pop the high vfp registers too.
#endif
      "                                           \n"
      "       msr PSP, r0                         \n" // Restore the main stack register.
      "       isb                                 \n"
      "                                           \n"
      "       bx lr                               \n"// Branch to LR (R14) to return from exception.
      "                                           \n"
      "       .align 2                            \n"
      "ppstack: .word _ZN2os4rtos4port9scheduler13stack_ptr_ptrE \n"

      : /* out */
      : /* in */
      [pri] "i"(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
          << ((8 - __NVIC_PRIO_BITS))),
      [gnc] "i"(&os::rtos::port::scheduler::get_next_context)
      : "memory", "cc"

      // Note: An XMC4000 specific errata workaround recommends
      // a 'push { r14 } / pop { pc }' before 'bx lr'.
  );

}
