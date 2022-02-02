/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2022 Liviu Ionescu.
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
