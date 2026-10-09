/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2017-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

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
  // Architecture assembly instructions in C.

  /**
   * `nop` instruction.
   */
  static void
  cortexm_architecture_nop (void);

  /**
   * `bkpt` instruction.
   *
   * It also acts as a compiler memory barrier, so that a debugger
   * stopped at the breakpoint sees all the preceding stores in memory.
   */
  static void
  cortexm_architecture_bkpt (void);

  /**
   * `wfi` instruction.
   *
   * It also acts as a compiler memory barrier, so that variables changed
   * by interrupt handlers are read again after it returns.
   */
  static void
  cortexm_architecture_wfi (void);

  /**
   * `dsb` instruction (Data Synchronization Barrier).
   *
   * @details
   * Ensures all explicit memory accesses (and, in the µOS++ usage in the
   * reset handler, prior system register writes such as `SCB->CPACR`)
   * issued before this instruction complete before any instruction after
   * it. Used together with `cortexm_architecture_isb()` to guarantee a
   * coprocessor access enabled via `CPACR` is visible before any
   * subsequent instruction that might use it, otherwise an instruction
   * fetched or issued while the write is still in flight may still fault.
   */
  static void
  cortexm_architecture_dsb (void);

  /**
   * `isb` instruction (Instruction Synchronization Barrier).
   *
   * @details
   * Flushes the pipeline, so instructions fetched after this point are
   * fetched only after the preceding `dsb` (and whatever it ordered) has
   * completed. Always used immediately after
   * `cortexm_architecture_dsb()` when enabling a coprocessor, so no
   * already-fetched instruction can run against the stale, pre-enable
   * state.
   */
  static void
  cortexm_architecture_isb (void);

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

  /**
   * @brief Data synchronisation barrier.
   *
   * @details
   * Ensures that all explicit memory accesses, and all system register
   * writes, issued before this call complete before any instruction
   * after it executes. It also acts as a compiler memory barrier.
   *
   * On Cortex-M, it is implemented with the `dsb` instruction.
   */
  static void
  micro_os_plus_architecture_data_barrier (void);

  /**
   * @brief Instruction synchronisation barrier.
   *
   * @details
   * Ensures that the instructions after this call are fetched and
   * executed only after the effects of the preceding context-changing
   * operations (such as system register writes, or code written to
   * memory) are visible. Usually called right after
   * `micro_os_plus_architecture_data_barrier()`.
   *
   * On Cortex-M, it is implemented with the `isb` instruction.
   */
  static void
  micro_os_plus_architecture_instruction_barrier (void);

  // --------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)

// ============================================================================

#if defined(__cplusplus)

namespace cortexm::architecture
{
  // --------------------------------------------------------------------------
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

  /**
   * The assembler `dsb` (Data Synchronization Barrier) instruction.
   */
  void
  dsb (void);

  /**
   * The assembler `isb` (Instruction Synchronization Barrier) instruction.
   */
  void
  isb (void);

  // --------------------------------------------------------------------------
} // namespace cortexm::architecture

namespace micro_os_plus::architecture
{
  // --------------------------------------------------------------------------
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

  /**
   * @brief Data synchronisation barrier.
   *
   * @details
   * The C++ equivalent of `micro_os_plus_architecture_data_barrier()`;
   * on Cortex-M, it is implemented with the `dsb` instruction.
   */
  void
  data_barrier (void);

  /**
   * @brief Instruction synchronisation barrier.
   *
   * @details
   * The C++ equivalent of
   * `micro_os_plus_architecture_instruction_barrier()`; on Cortex-M,
   * it is implemented with the `isb` instruction.
   */
  void
  instruction_barrier (void);

  // ------------------------------------------------------------------------
} // namespace micro_os_plus::architecture

#endif // defined(__cplusplus)

// ============================================================================
// Templates, inlines & constexpr implementations.

#include "inlines/instructions-inlines.h"

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_INSTRUCTIONS_H_

// ----------------------------------------------------------------------------
