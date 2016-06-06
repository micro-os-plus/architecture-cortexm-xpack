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
 * This file is part of the CMSIS++ proposal, intended as a CMSIS
 * replacement for C++ applications.
 *
 * If contains the implementation for all objects, using the
 * FreeRTOS API.
 *
 * This file is included in all src/os-*.cpp files.
 */

#ifndef CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_
#define CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_

// ----------------------------------------------------------------------------

#include <cmsis-plus/rtos/os-app-config.h>
#include <cmsis-plus/rtos/os-c-decls.h>

// ----------------------------------------------------------------------------

#ifdef  __cplusplus

#include <cmsis_device.h>

#include <cmsis-plus/diag/trace.h>
#include <cmsis-plus/iso/malloc.h>

namespace os
{
  namespace rtos
  {
    namespace port
    {
      // ----------------------------------------------------------------------

      namespace scheduler
      {

        inline void
        __attribute__((always_inline))
        greeting (void)
        {
          trace::printf ("µOS++ Cortex-M");
#if defined(__ARM_ARCH_7EM__)
          trace::printf ("4");
#if defined(__VFP_FP__) && !defined(__SOFTFP__)
          trace::printf (" FP");
#endif
#elif defined(__ARM_ARCH_7M__)
          trace::printf ("3");
#elif defined(__ARM_ARCH_6M__)
          trace::printf ("0/0+");
#endif
          trace::printf (" scheduler; preemptive.\n");
        }

        // Mandatory inline in main().
        inline result_t
        __attribute__((always_inline))
        initialize (void)
        {
          return result::ok;
        }

        inline bool
        __attribute__((always_inline))
        in_handler_mode (void)
        {
          // In Handler mode, IPSR holds the exception number.
          // If 0, the core is in thread mode.
          return (__get_IPSR () != 0);
        }

        inline void
        __attribute__((always_inline))
        lock (rtos::scheduler::status_t status __attribute__((unused)))
        {
          ;
        }

        inline void
        __attribute__((always_inline))
        _wait_for_interrupt (void)
        {
#if defined(OS_TRACE_RTOS_THREAD_CONTEXT)
          trace::printf ("%s() \n", __func__);
#endif
          __WFI ();
        }

      } /* namespace scheduler */

      // The DSB/ISB are recommended by ARM after priority level changes.
      // (Application Note 321: ARM Cortex-M Programming Guide to
      // Memory Barrier Instructions)
      // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf

      namespace interrupts
      {
#if !defined(__ARM_ARCH_7M__) && !defined(__ARM_ARCH_7EM__) && !defined(__ARM_ARCH_6M__)
#error Critical sections not implemented of this architecture
#endif

        // Enter an IRQ critical section
        inline rtos::interrupts::status_t
        __attribute__((always_inline))
        critical_section::enter (void)
        {
          uint32_t pri;

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          // Read the current BASEPRI, to be returned and later restored.
          pri = __get_BASEPRI ();

          // According to ARM "MSR{cond} spec_reg, Rn":
          // When you write to BASEPRI_MAX, the instruction writes to
          // BASEPRI only if either:
          // Rn is non-zero and the current BASEPRI value is 0
          // Rn is non-zero and less than the current BASEPRI value.
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

          // Apparently not required by architecture, but used by
          // FreeRTOS, with an unconvincing motivation ("...  ensure
          // the code is completely within the specified behaviour
          // for the architecture").
          __DSB ();
          __ISB ();

          return pri;
        }

        // Exit an IRQ critical section
        inline void
        __attribute__((always_inline))
        critical_section::exit (rtos::interrupts::status_t status)
        {
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          // Restore BASEPRI to the value saved by enter().
          __set_BASEPRI (status);

#else

          // Restore PRIMASK to the value saved by enter().
          __set_PRIMASK (status);

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#elif defined(__ARM_ARCH_6M__)

          // Restore PRIMASK to the value saved by enter().
          __set_PRIMASK (status);

#endif

          __DSB ();
          __ISB ();
        }

        // ====================================================================

        // Enter an IRQ uncritical section
        inline rtos::interrupts::status_t
        __attribute__((always_inline))
        uncritical_section::enter (void)
        {
          uint32_t pri;

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          pri = __get_BASEPRI ();

          // Setting BASEPRI to 0 makes it ineffective,
          // practically enabling all interrupts.
          __set_BASEPRI (0);

#else

          pri = __get_PRIMASK ();

          // Enable all interrupts.
          __enable_irq ();

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#elif defined(__ARM_ARCH_6M__)

          pri = __get_PRIMASK ();

          // Enable all interrupts.
          __enable_irq ();

#endif

          __DSB ();
          __ISB ();

          return pri;
        }

        // Exit an IRQ critical section
        inline void
        __attribute__((always_inline))
        uncritical_section::exit (rtos::interrupts::status_t status)
        {
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY)

          // Restore BASEPRI to the value saved by enter().
          __set_BASEPRI (status);

#else

          // Restore PRIMASK to the value saved by enter().
          __set_PRIMASK (status);

#endif /* defined(OS_INTEGER_RTOS_CRITICAL_SECTION_INTERRUPT_PRIORITY) */

#elif defined(__ARM_ARCH_6M__)

          // Restore PRIMASK to the value saved by enter().
          __set_PRIMASK (status);

#endif

          __DSB ();
          __ISB ();
        }

      } /* namespace interrupts */

      // ======================================================================

      namespace this_thread
      {
        inline void
        __attribute__((always_inline))
        prepare_suspend (void)
        {
          ;
        }

      } /* namespace this_thread */

    // ======================================================================

    } /* namespace port */
  } /* namespace rtos */
} /* namespace os */

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* CMSIS_PLUS_RTOS_PORT_OS_INLINES_H_ */
