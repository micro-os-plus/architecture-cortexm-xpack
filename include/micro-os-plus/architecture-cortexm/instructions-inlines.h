/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2017 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_INLINES_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_INLINES_H_

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------
// Inline implementations for the Cortex-M architecture instructions.

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

  // --------------------------------------------------------------------------

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_nop (void)
  {
    __asm__ volatile(

        " nop "

        : /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_bkpt (void)
  {
    __asm__ volatile(

        " bkpt 0 "

        : /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );
  }

  static inline __attribute__ ((always_inline)) void
  cortexm_architecture_wfi (void)
  {
    __asm__ volatile(

        " wfi "

        : /* Outputs */
        : /* Inputs */
        : /* Clobbers */
    );
  }

  static inline __attribute__ ((always_inline)) void
  micro_os_plus_architecture_nop (void)
  {
    cortexm_architecture_nop ();
  }

  /**
   * `break` instruction.
   */
  static inline __attribute__ ((always_inline)) void
  micro_os_plus_architecture_brk (void)
  {
    cortexm_architecture_bkpt ();
  }

  /**
   * `wfi` instruction.
   */
  static inline __attribute__ ((always_inline)) void
  micro_os_plus_architecture_wfi (void)
  {
    cortexm_architecture_wfi ();
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

    inline __attribute__ ((always_inline)) void
    nop (void)
    {
      cortexm_architecture_nop ();
    }

    inline __attribute__ ((always_inline)) void
    bkpt (void)
    {
      cortexm_architecture_bkpt ();
    }

    inline __attribute__ ((always_inline)) void
    wfi (void)
    {
      cortexm_architecture_wfi ();
    }

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace cortexm

namespace micro_os_plus
{
  namespace architecture
  {
    // ------------------------------------------------------------------------

    inline __attribute__ ((always_inline)) void
    nop (void)
    {
      cortexm::architecture::nop ();
    }

    inline __attribute__ ((always_inline)) void
    brk (void)
    {
      cortexm::architecture::bkpt ();
    }

    inline __attribute__ ((always_inline)) void
    wfi (void)
    {
      cortexm::architecture::wfi ();
    }

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace micro_os_plus

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_INLINES_H_

// ----------------------------------------------------------------------------
