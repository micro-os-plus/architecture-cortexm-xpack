/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2020 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_SEMIHOSTING_INLINES_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_SEMIHOSTING_INLINES_H_

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------
// Inline implementations for the Cortex-M semihosting call.

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
// configuration cannot trigger HardFault exceptions while the debugger is
// connected, so we use an illegal op code, that will trigger an
// UsageFault exception.
#define AngelSWITestFault "setend be"
#define AngelSWITestFaultOpCode (0xB658)
#endif

  // Type of each entry in a parameter block.
  typedef micro_os_plus_architecture_register_t
      micro_os_plus_semihosting_param_block_t;
  // Type of result.
  typedef micro_os_plus_architecture_register_t
      micro_os_plus_semihosting_response_t;

  static inline __attribute__ ((always_inline))
  micro_os_plus_semihosting_response_t
  micro_os_plus_semihosting_call_host (
      int reason, micro_os_plus_semihosting_param_block_t* arg)
  {
    micro_os_plus_semihosting_response_t value;
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
