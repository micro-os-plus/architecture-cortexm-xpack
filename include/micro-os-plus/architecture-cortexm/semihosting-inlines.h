/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2020 Liviu Ionescu.
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

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_SEMIHOSTING_INLINES_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_SEMIHOSTING_INLINES_H_

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------

/*
 * Inline implementations for the Cortex-M semihosting call.
 */

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

// SWI numbers and reason codes for RDI (Angel) monitors.
#define AngelSWI_ARM 0x123456
#ifdef __thumb__
#define AngelSWI 0xAB
#else
#define AngelSWI AngelSWI_ARM
#endif
// For thumb only architectures use the BKPT instruction instead of SWI.
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_6M__)
#define AngelSWIInsn "bkpt"
#define AngelSWIAsm bkpt
#else
#define AngelSWIInsn "swi"
#define AngelSWIAsm swi
#endif

#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
// Testing the local semihosting handler cannot use another BKPT, since this
// configuration cannot trigger HaedFault exceptions while the debugger is
// connected, so we use an illegal op code, that will trigger an
// UsageFault exception.
#define AngelSWITestFault "setend be"
#define AngelSWITestFaultOpCode (0xB658)
#endif

  static inline __attribute__ ((always_inline)) int
  os_semihosting_call_host (int reason, void* arg)
  {
    int value;
    __asm__ volatile(

        " mov r0, %[rsn]  \n"
        " mov r1, %[arg]  \n"
#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
        " " AngelSWITestFault " \n"
#else
      " " AngelSWIInsn " %[swi] \n"
#endif
        " mov %[val], r0"

        : [val] "=r"(value) /* Outputs */
        : [rsn] "r"(reason), [arg] "r"(arg), [swi] "i"(AngelSWI) /* Inputs */
        : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
        // Clobbers r0 and r1, and lr if in supervisor mode
    );

    // Accordingly to page 13-77 of ARM DUI 0040D other registers
    // can also be clobbered. Some memory positions may also be
    // changed by a system call, so they should not be kept in
    // registers. Note: we are assuming the manual is right and
    // Angel is respecting the APCS.
    return value;
  }

  // --------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_SEMIHOSTING_INLINES_H_

// ----------------------------------------------------------------------------
