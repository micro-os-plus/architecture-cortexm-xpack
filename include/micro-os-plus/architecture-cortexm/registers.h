/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2022 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------

#include <micro-os-plus/architecture-cortexm/defines.h>

#include <stdint.h>

// ----------------------------------------------------------------------------
// Declarations of Cortex-M functions to wrap architecture instructions.

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

  // --------------------------------------------------------------------------
  // Architecture registers getters and mutators in C.

  /**
   * Main Stack Pointer getter.
   */
  static cortexm_architecture_register_t
  cortexm_architecture_get_msp (void);

  // --------------------------------------------------------------------------
  // Portable architecture assembly instructions in C.

  /**
   * Stack Pointer getter.
   */
  static micro_os_plus_architecture_register_t
  micro_os_plus_architecture_get_sp (void);

  // --------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ============================================================================

#if defined(__cplusplus)

namespace cortexm
{
  namespace architecture
  {
    namespace registers
    {
      // ----------------------------------------------------------------------
      // Architecture getters in C++.

      /**
       * Main Stack Pointer getter.
       */
      register_t
      msp (void);

      // ----------------------------------------------------------------------
    } // namespace registers
  } // namespace architecture
} // namespace cortexm

namespace micro_os_plus
{
  namespace architecture
  {
    namespace registers
    {
      // ------------------------------------------------------------------------
      // Portable architecture assembly instructions in C++.

      /**
       * Main Stack Pointer getter.
       */
      register_t
      sp (void);
    }

    // ------------------------------------------------------------------------
  }
} // namespace architecture
} // namespace micro_os_plus

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------
