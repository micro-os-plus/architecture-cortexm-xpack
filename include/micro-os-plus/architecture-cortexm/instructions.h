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
  // Architecture assembly instructions in C.

  /**
   * `nop` instruction.
   */
  static void
  cortexm_architecture_nop (void);

  /**
   * `bkpt` instruction.
   */
  static void
  cortexm_architecture_bkpt (void);

  /**
   * `wfi` instruction.
   */
  static void
  cortexm_architecture_wfi (void);

  // --------------------------------------------------------------------------
  // Portable architecture assembly instructions in C.

  /**
   * `nop` instruction.
   */
  static void
  micro_os_plus_architecture_nop (void);

  /**
   * `break` instruction.
   */
  static void
  micro_os_plus_architecture_brk (void);

  /**
   * `wfi` instruction.
   */
  static void
  micro_os_plus_architecture_wfi (void);

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
    // Architecture assembly instructions in C++.

    /**
     * The assembler `nop` instruction.
     */
    void
    nop (void);

    /**
     * The assembler `bkpt` instruction.
     */
    void
    bkpt (void);

    /**
     * The assembler `wfi` instruction.
     */
    void
    wfi (void);

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace cortexm

namespace micro_os_plus
{
  namespace architecture
  {
    // ------------------------------------------------------------------------
    // Portable architecture assembly instructions in C++.

    /**
     * The assembler `nop` instruction.
     */
    void
    nop (void);

    /**
     * The assembler `bkpt` instruction.
     */
    void
    brk (void);

    /**
     * The assembler `wfi` instruction.
     */
    void
    wfi (void);

    // ------------------------------------------------------------------------
  } // namespace architecture
} // namespace micro_os_plus

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------
