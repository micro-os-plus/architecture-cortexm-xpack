/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2017 Liviu Ionescu.
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

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------

#include <micro-os-plus/architecture-cortexm/defines.h>

#include <stdint.h>

// ----------------------------------------------------------------------------

/*
 * Declarations of RISC-V functions to wrap architecture instructions.
 */

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
  cortexm_arch_nop (void);

  /**
   * `bkpt` instruction.
   */
  static void
  cortexm_arch_bkpt (void);

  /**
   * `wfi` instruction.
   */
  static void
  cortexm_arch_wfi (void);

  // --------------------------------------------------------------------------
  // Portable architecture assembly instructions in C.

  /**
   * `nop` instruction.
   */
  static void
  os_arch_nop (void);

  /**
   * `break` instruction.
   */
  static void
  os_arch_brk (void);

  /**
   * `wfi` instruction.
   */
  static void
  os_arch_wfi (void);

  // --------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ============================================================================

#if defined(__cplusplus)

namespace cortexm
{
  namespace arch
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
  } // namespace arch
} // namespace cortexm

namespace micro_os_plus
{
  namespace arch
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
  } // namespace arch
} // namespace micro_os_plus

#endif // defined(__cplusplus)

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------
