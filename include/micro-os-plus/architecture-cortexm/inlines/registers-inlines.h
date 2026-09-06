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

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INLINES_REGISTERS_INLINES_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INLINES_REGISTERS_INLINES_H_

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------
// Inline implementations for the Cortex-M architecture instructions.

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

  // --------------------------------------------------------------------------

  static inline __attribute__ ((always_inline)) cortexm_architecture_register_t
  cortexm_architecture_get_msp (void)
  {
    uint32_t result;

    __asm__ volatile (

        "msr %0, msp"

        : "=r"(result) /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );

    return result;
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_set_msp (
      cortexm_architecture_register_t top_of_main_stack)
  {
    __asm__ volatile ("msr msp, %0"

                      : /* Outputs */
                      : "r"(top_of_main_stack) /* Inputs */
                      : /* Clobbers */
    );
  }

  static inline __attribute__ ((always_inline)) cortexm_architecture_register_t
  cortexm_architecture_get_psp (void)
  {
    uint32_t result;

    __asm__ volatile (

        "msr %0, psp"

        : "=r"(result) /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );

    return result;
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_set_psp (
      cortexm_architecture_register_t top_of_process_stack)
  {
    __asm__ volatile ("msr psp, %0"

                      : /* Outputs */
                      : "r"(top_of_process_stack) /* Inputs */
                      : /* Clobbers */
    );
  }

  static inline
      __attribute__ ((always_inline)) micro_os_plus_architecture_register_t
      micro_os_plus_architecture_get_sp (void)
  {
    return cortexm_architecture_get_msp ();
  }

  static inline __attribute__ ((always_inline)) void
  micro_os_plus_architecture_set_sp (
      micro_os_plus_architecture_register_t top_of_stack)
  {
    cortexm_architecture_set_msp (top_of_stack);
  }

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

  static inline __attribute__ ((always_inline)) cortexm_architecture_register_t
  cortexm_architecture_get_msplim (void)
  {
    uint32_t result;

    __asm__ volatile (

        "msr %0, msplim"

        : "=r"(result) /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );

    return result;
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_set_msplim (
      cortexm_architecture_register_t bottom_of_main_stack)
  {
    __asm__ volatile ("msr msplim, %0"

                      : /* Outputs */
                      : "r"(bottom_of_main_stack) /* Inputs */
                      : /* Clobbers */
    );
  }

  static inline __attribute__ ((always_inline)) cortexm_architecture_register_t
  cortexm_architecture_get_psplim (void)
  {
    uint32_t result;

    __asm__ volatile (

        "msr %0, psplim"

        : "=r"(result) /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );

    return result;
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_set_psplim (
      cortexm_architecture_register_t bottom_of_process_stack)
  {
    __asm__ volatile ("msr psplim, %0"

                      : /* Outputs */
                      : "r"(bottom_of_process_stack) /* Inputs */
                      : /* Clobbers */
    );
  }

#endif // defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

  // --------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ============================================================================

#if defined(__cplusplus)

namespace cortexm::architecture::registers
{
  // --------------------------------------------------------------------------

  inline __attribute__ ((always_inline)) register_t
  msp (void)
  {
    return cortexm_architecture_get_msp ();
  }

  inline __attribute__ ((always_inline)) void
  msp (register_t top_of_main_stack)
  {
    cortexm_architecture_set_msp (top_of_main_stack);
  }

  // --------------------------------------------------------------------------

  inline __attribute__ ((always_inline)) register_t
  psp (void)
  {
    return cortexm_architecture_get_psp ();
  }

  inline __attribute__ ((always_inline)) void
  psp (register_t top_of_process_stack)
  {
    cortexm_architecture_set_psp (top_of_process_stack);
  }

  // --------------------------------------------------------------------------
} // namespace cortexm::architecture::registers

namespace micro_os_plus::architecture::registers
{
  // --------------------------------------------------------------------------

  inline __attribute__ ((always_inline)) register_t
  sp (void)
  {
    return cortexm::architecture::registers::msp ();
  }

  inline __attribute__ ((always_inline)) void
  sp (register_t top_of_main_stack)
  {
    cortexm::architecture::registers::msp (top_of_main_stack);
  }

  // --------------------------------------------------------------------------
} // namespace micro_os_plus::architecture::registers

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

namespace cortexm::architecture::registers
{
  // --------------------------------------------------------------------------

  inline __attribute__ ((always_inline)) register_t
  msplim (void)
  {
    return cortexm_architecture_get_msplim ();
  }

  inline __attribute__ ((always_inline)) void
  msplim (register_t bottom_of_main_stack)
  {
    cortexm_architecture_set_msplim (bottom_of_main_stack);
  }

  // --------------------------------------------------------------------------

  inline __attribute__ ((always_inline)) register_t
  psplim (void)
  {
    return cortexm_architecture_get_psplim ();
  }

  inline __attribute__ ((always_inline)) void
  psplim (register_t bottom_of_process_stack)
  {
    cortexm_architecture_set_psplim (bottom_of_process_stack);
  }

  // --------------------------------------------------------------------------
} // namespace cortexm::architecture::registers

#endif // defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INLINES_REGISTERS_INLINES_H_

// ----------------------------------------------------------------------------
