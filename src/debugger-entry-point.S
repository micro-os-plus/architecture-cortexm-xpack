/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2014-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

// ----------------------------------------------------------------------------

#define __ASSEMBLY__ 1
#include "micro-os-plus/architecture.h"

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_DEBUGGER_ENTRY_POINT_ENABLED)

    .equ VTOR, 0xE000ED08

    .section .debugger_entry_point, "ax", %progbits
    .type _debugger_entry_point, %function
    .syntax unified
    .thumb_func
    .global _debugger_entry_point

    .fnstart

_debugger_entry_point:

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__)

    ldr     r1, =VTOR

#if defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_DEBUGGER_ENTRY_POINT_INITIALISE_VTOR_ENABLED)

    ldr     r0, =_interrupt_vectors   // r0 = &_interrupt_vectors
    bic     r0, r0, #0x3F             // r0 &= ~0x3F (clear low 6 bits)
    str     r0, [r1]                  // SCB->VTOR = r0
    
#endif // defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_DEBUGGER_ENTRY_POINT_INITIALISE_VTOR_ENABLED)

#elif defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_8M_BASE__)

    ldr     r1, =0x00000000           // Arm v6m has no VTOR.

#else
#error "Unsupported architecture."
#endif // defined(__ARM_ARCH_7M__) ...

    str     r0, [r1]
    ldmia   r0!, {r1, r2}
    msr     msp, r1
    bx      r2

    .fnend
    .size    _debugger_entry_point, . - _debugger_entry_point

#endif // defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_DEBUGGER_ENTRY_POINT_ENABLED)

// ----------------------------------------------------------------------------
