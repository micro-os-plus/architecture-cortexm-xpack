/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2022 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_INLINES_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_INLINES_H_

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

    __asm__ volatile(

        "msr %0, msp"

        : "=r" (result) /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );

    return result;
  }

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
    // ------------------------------------------------------------------------

    inline __attribute__ ((always_inline)) register_t
    get_msp (void)
    {
      cortexm_architecture_get_msp ();
    }

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace cortexm

namespace micro_os_plus
{
  namespace architecture
  {
    // ------------------------------------------------------------------------

    inline __attribute__ ((always_inline)) register_t
    get_sp (void)
    {
      micro_os_plus_architecture_get_sp ();
    }

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace micro_os_plus

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_REGISTERS_INLINES_H_

// ----------------------------------------------------------------------------
