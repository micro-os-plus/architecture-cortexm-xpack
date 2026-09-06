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

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_H_

// ----------------------------------------------------------------------------

#include "micro-os-plus/architecture-cortexm/defines.h"

#include <stdint.h>

// ----------------------------------------------------------------------------
// Declarations of Cortex-M functions to wrap architecture instructions.

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

  // --------------------------------------------------------------------------
  // Architecture registers accessorss in C.

  /**
   * Main Stack Pointer getter.
   */
  static cortexm_architecture_register_t
  cortexm_architecture_get_msp (void);

  /**
   * Main Stack Pointer setter.
   */
  static void
  cortexm_architecture_set_msp (
      cortexm_architecture_register_t top_of_main_stack);

  /**
   * Process Stack Pointer getter.
   */
  static cortexm_architecture_register_t
  cortexm_architecture_get_psp (void);

  /**
   * Process Stack Pointer setter.
   */
  static void
  cortexm_architecture_set_psp (
      cortexm_architecture_register_t top_of_process_stack);

  // --------------------------------------------------------------------------
  // Portable architecture assembly instructions in C.

  /**
   * Stack Pointer getter.
   */
  static micro_os_plus_architecture_register_t
  micro_os_plus_architecture_get_sp (void);

  /**
   * Stack Pointer setter.
   */
  static void
  micro_os_plus_architecture_set_sp (
      micro_os_plus_architecture_register_t top_of_stack);

  // --------------------------------------------------------------------------

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

  static cortexm_architecture_register_t
  cortexm_architecture_get_msplim (void);

  static void
  cortexm_architecture_set_msplim (
      cortexm_architecture_register_t bottom_of_main_stack);

  static cortexm_architecture_register_t
  cortexm_architecture_get_psplim (void);

  static void
  cortexm_architecture_set_psplim (
      cortexm_architecture_register_t bottom_of_process_stack);

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
  // Architecture MSP accessors in C++.

  /**
   * Main Stack Pointer getter.
   */
  register_t
  msp (void);

  /**
   * Main Stack Pointer setter.
   */
  void
  msp (register_t top_of_main_stack);

  // --------------------------------------------------------------------------
  // Architecture PSP accessors in C++.

  /**
   * Process Stack Pointer getter.
   */
  register_t
  psp (void);

  /**
   * Process Stack Pointer setter.
   */
  void
  psp (register_t top_of_process_stack);

  // --------------------------------------------------------------------------
} // namespace cortexm::architecture::registers

namespace micro_os_plus::architecture::registers
{
  // --------------------------------------------------------------------------
  // Portable architecture accessors in C++.

  /**
   * Stack Pointer getter.
   */
  register_t
  sp (void);

  /**
   * Stack Pointer setter.
   */
  void sp (register_t);

  // --------------------------------------------------------------------------
} // namespace micro_os_plus::architecture::registers

#endif // defined(__cplusplus)

// ============================================================================
// Templates, inlines & constexpr implementations.

#include "inlines/registers-inlines.h"

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_H_

// ----------------------------------------------------------------------------
