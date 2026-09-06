/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2022-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

// ----------------------------------------------------------------------------

#include "micro-os-plus/architecture.h"

#include <stdint.h>

// ----------------------------------------------------------------------------
// SCB/FPU register addresses (used instead of the SCB->.../FPU->... accessors
// since CMSIS is not available here, being device specific).

#define SCB_CPACR (*(volatile uint32_t*)0xE000ED88)
#define SCB_SHCSR (*(volatile uint32_t*)0xE000ED24)
#define FPU_FPCCR (*(volatile uint32_t*)0xE000EF34)

#define SCB_CPACR_CP10_MASK (3UL << 20U)
#define SCB_CPACR_CP11_MASK (3UL << 22U)

#define SCB_SHCSR_USGFAULTENA_MASK (1UL << 18U)
#define SCB_SHCSR_BUSFAULTENA_MASK (1UL << 17U)
#define SCB_SHCSR_MEMFAULTENA_MASK (1UL << 16U)

#define FPU_FPCCR_ASPEN_MASK (1UL << 31U)
#define FPU_FPCCR_LSPEN_MASK (1UL << 30U)

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_RESET_HANDLER_ENABLED)

// ----------------------------------------------------------------------------

extern handler_ptr_t _interrupt_vectors[];

extern void __attribute__ ((noreturn, weak))
_start (void);

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

extern uintptr_t __stack_limit__;

#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
extern uintptr_t __stack_seal__;
#endif // defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

#endif // defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

// ----------------------------------------------------------------------------

// QEMU always uses the VTOR values to initialise the stack and the vector
// table, so the Reset_Handler() is always called.
void __attribute__ ((section (".after_vectors"), noreturn, naked))
Reset_Handler (void)
{
#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

  cortexm_architecture_set_msplim ((uint32_t)(&__stack_limit__));
  // Set PSPLIM when PSP is set, here PSP stack is not known.

#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  // TODO: implement __TZ_set_STACKSEAL_S
  // __TZ_set_STACKSEAL_S ((uint32_t*)(&__stack_seal__));
#endif // defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

#endif // defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

// Floating point instructions can be used early in the C/C++ startup
// sequence as a result of compiler optimisations, therefore the
// FPU must be enabled before calling any C/C++ functions, including main().
// (`SystemInit()` happens too late).
#if (defined(__ARM_PCS_VFP) && (__ARM_PCS_VFP > 0U)) \
    || (defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0U))

  // Enable CP10 and CP11 coprocessor.
  SCB_CPACR |= (SCB_CPACR_CP10_MASK // enable CP10 Full Access
                | SCB_CPACR_CP11_MASK); // enable CP11 Full Access

  // Lazy save.
  FPU_FPCCR |= FPU_FPCCR_ASPEN_MASK // enable automatic state preservation
               | FPU_FPCCR_LSPEN_MASK; // enable lazy context save
               
#endif // defined (__FPU_USED) ...

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_8M_MAIN__)

  // Enable faults.
  SCB_SHCSR |= SCB_SHCSR_USGFAULTENA_MASK // enable UsageFault
               | SCB_SHCSR_BUSFAULTENA_MASK // enable BusFault
               | SCB_SHCSR_MEMFAULTENA_MASK; // enable MemManage fault

#endif // defined(__ARM_ARCH_7M__) ...

  _start ();
  /* NOTREACHED */

  cortexm_architecture_bkpt ();
  while (1)
    {
      cortexm_architecture_wfi ();
    }
}

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_RESET_HANDLER_ENABLED)

// ----------------------------------------------------------------------------
