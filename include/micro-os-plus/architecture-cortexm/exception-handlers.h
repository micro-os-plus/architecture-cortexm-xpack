/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_EXCEPTION_HANDLERS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_EXCEPTION_HANDLERS_H_

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C"
{
#endif

  // External references to Cortex-M exception_handlers.c

  extern void
  Reset_Handler (void);
  extern void
  NMI_Handler (void);
  extern void
  HardFault_Handler (void);

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  extern void
  MemManage_Handler (void);
  extern void
  BusFault_Handler (void);
  extern void
  UsageFault_Handler (void);
  extern void
  DebugMon_Handler (void);
#endif

  extern void
  SVC_Handler (void);

  extern void
  PendSV_Handler (void);
  extern void
  SysTick_Handler (void);

  // Exception Stack Frame of the Cortex-M3 or Cortex-M4 processors.
  typedef struct
  {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
#if defined(__ARM_ARCH_7EM__)
    uint32_t s[16];
#endif
  } exception_stack_frame_s;

#if defined(MICRO_OS_PLUS_TRACE)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  void
  dump_exception_stack (exception_stack_frame_s* frame, uint32_t cfsr,
                        uint32_t mmfar, uint32_t bfar, uint32_t lr);
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#if defined(__ARM_ARCH_6M__)
  void
  dump_exception_stack (exception_stack_frame_s* frame, uint32_t lr);
#endif // defined(__ARM_ARCH_6M__)
#endif // defined(MICRO_OS_PLUS_TRACE)

  void
  hard_fault_handler_c (exception_stack_frame_s* frame, uint32_t lr);

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  void
  usage_fault_handler_c (exception_stack_frame_s* frame, uint32_t lr);
  void
  bus_fault_handler_c (exception_stack_frame_s* frame, uint32_t lr);
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(__cplusplus)
}
#endif

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_EXCEPTION_HANDLERS_H_

// ----------------------------------------------------------------------------
