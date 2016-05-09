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

#include <string.h>
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

          // The address of the trampoline code. // PCL +14*4=60
          *--p = (rtos::stack::element_t) (((ptrdiff_t) func) & (~1));

          // The stack as if after a context save.

          // Link register // LR   +13*4=56
#if defined(OS_BOOL_RTOS_PORT_CONTEX_CREATE_ZERO_LR)
          *--p = 0x00000000;
#else
          // 0x0 looks odd in the debugger, so try to hide it.
          // In Eclipse using 'func+2' will make the stack trace
          // start with 'func' (don't ask why).
          *--p = (rtos::stack::element_t) (((ptrdiff_t) func + 2));
#endif

          *--p = 12;  // R12 +12*4=52

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
          // somewhere, so prepare a fake thread.
          os_thread_t fake_thread;
          memset (&fake_thread, 0, sizeof(os_thread_t));

          fake_thread.name = "none";
          rtos::Thread* pth = (rtos::Thread*) &fake_thread;

          // Make the fake thread look like the current thread.
          rtos::scheduler::current_thread_ = pth;

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

        // The DSB/ISB are recommended by ARM after programming
        // the control registers.
        // (Application Note 321: ARM Cortex-M Programming Guide to
        // Memory Barrier Instructions)
        // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf

        // Must be called in a critical section.
        void
        reschedule (void)
        {
          if (rtos::scheduler::locked ())
            {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
              trace::printf ("%s() %s nop\n", __func__,
                             rtos::scheduler::current_thread_->name ());
#endif
              return;
            }

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s()\n", __func__);
#endif
          // Set PendSV to request a context switch.
          SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

          __DSB ();
          __ISB ();
        }

        // Called from PendSV_Handler to perform the context switch.
        rtos::stack::element_t*
        switch_stacks (rtos::stack::element_t* sp)
        {
          // Enter a local critical section to protect the lists.
          uint32_t pri = __get_BASEPRI ();
          __set_BASEPRI_MAX (
              OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY
                  << ((8 - __NVIC_PRIO_BITS)));

          rtos::Thread* old_thread = rtos::scheduler::current_thread_;

          // Save the current SP in the initial context.
          old_thread->context ().port_.stack_ptr = sp;

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s() leave %s\n", __func__, old_thread->name ());
#endif

          // Clear the PendSV bit.
          SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;

          if (old_thread->sched_state () == rtos::thread::state::running)
            {
              // If the current thread is running, save it to the
              // ready list, so that it will be resumed later.
              Waiting_thread_node& crt_node = old_thread->ready_node_;
              if (crt_node.next == nullptr)
                {
                  rtos::scheduler::ready_threads_list_.link (crt_node);
                  // Ready state set in above link().
                }
            }

          // The top of the ready list gives the next thread to run.
          rtos::scheduler::current_thread_ =
              rtos::scheduler::ready_threads_list_.remove_top ();

#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s() switch to %s\n", __func__,
                         rtos::scheduler::current_thread_->name ());
#endif

          // Prepare a local copy of the new thread SP.
          stack::element_t* out_sp =
              rtos::scheduler::current_thread_->context ().port_.stack_ptr;

          // Restore priorities.
          __set_BASEPRI (pri);

          // Return the new thread SP. Registers will be
          // restored in the assembly code.
          return out_sp;
        }

      } /* namespace scheduler */

      void
      Systick_clock::start (void)
      {
        assert(
            SysTick_Config (SystemCoreClock / rtos::Systick_clock::frequency_hz)
                == 0);

        // Set SysTick interrupt priority to the lowest level (highest value).
        NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
      }

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

extern "C" void
PendSV_Handler (void);

// Credits: inspired by FreeRTOS Cortex-M3/M4 xPortPendSVHandler(),
// but simplified by moving the stack pointer save/restore to C++,
// so that it is no longer needed to keep a separate global
// pointer to the context.

void
__attribute__ ((section(".after_vectors"), naked, used))
PendSV_Handler (void)
{
  asm volatile
  (
      // Registers R0-R3 are saved as part of the exception frame
      // and are free to use. The rest must be manually saved/restored.

      "       stmdb sp!, {lr}                     \n"// Save LR on MSP (not on PSP!)
      "                                           \n"
      "       mrs r0, PSP                         \n"// Get the thread stack in R0
      "       isb                                 \n"
      "                                           \n"
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
      "       tst r14, #0x10                      \n" // Is the task using the FPU context?
      "       it eq                               \n"
      "       vstmdbeq r0!, {s16-s31}             \n"// If so, push high vfp registers.
#endif
      "                                           \n"
      "       stmdb r0!, {r4-r9,sl,fp}            \n" // Save the core registers r4-r11.
      "                                           \n"
      "       bl %[gnc]                           \n"// r0 = switch_stacks(r0)
      "                                           \n"
      "       ldmia r0!, {r4-r9,sl,fp}            \n"// Pop the core registers r4-r11.
      "                                           \n"
#if defined (__VFP_FP__) && !defined (__SOFTFP__)
      "       tst r14, #0x10                      \n" // Is the task using the FPU context?
      "       it eq                               \n"
      "       vldmiaeq r0!, {s16-s31}             \n"// If so, pop the high vfp registers too.
#endif
      "                                           \n"
      "       msr PSP, r0                         \n" // Restore the thread stack register.
      "       isb                                 \n"
      "                                           \n"
      "       ldmia sp!, {pc}                     \n"// Restore PC from MSP, where LR was saved
      "                                           \n"// This will return from exception.
      : /* out */
      : /* in */
      [gnc] "i"(&os::rtos::port::scheduler::switch_stacks)
      : /* clobber */
  );
}
