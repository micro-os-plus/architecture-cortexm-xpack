/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2014-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

// ----------------------------------------------------------------------------

#include "micro-os-plus/architecture.h"
#include "micro-os-plus/architecture-cortexm/exception-handlers.h"
// #include "micro-os-plus/startup/defines.h"

#include "micro-os-plus/semihosting.h"
#include "micro-os-plus/diag/trace.h"

#include <string.h>
#include <inttypes.h>

// ----------------------------------------------------------------------------

#if defined( \
    MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_EXCEPTION_HANDLERS_ENABLED)

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

extern "C"
{
  extern unsigned int __heap_end__;
  extern unsigned int __stack;

  void __attribute__ ((noreturn, weak))
  _start (void);
}

#if defined(__GNUC__)
// core_cm4.h:1558:42: error: use of old-style cast to 'struct CoreDebug_Type*
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif // defined(__GNUC__)

// ----------------------------------------------------------------------------
// SCB register addresses (used instead of the SCB->... accessors, since CMSIS
// is not available here, being device specific).

#define SCB_CFSR (*(volatile uint32_t*)0xE000ED28)
#define SCB_HFSR (*(volatile uint32_t*)0xE000ED2C)
#define SCB_DFSR (*(volatile uint32_t*)0xE000ED30)
#define SCB_MMFAR (*(volatile uint32_t*)0xE000ED34)
#define SCB_BFAR (*(volatile uint32_t*)0xE000ED38)
#define SCB_AFSR (*(volatile uint32_t*)0xE000ED3C)

// ----------------------------------------------------------------------------
// Default exception handlers. Override the ones here by defining your own
// handler routines in your application code.

// The ARCH_7M exception handlers are:
// 0x00 stack
// 0x04 Reset
// 0x08 NMI
// 0x0C HardFault
// 0x10 MemManage
// 0x14 BusFault
// 0x18 UsageFault
// 0x1C 0
// 0x20 0
// 0x24 0
// 0x28 0
// 0x2C SVC
// 0x30 DebugMon
// 0x34 0
// 0x38 PendSV
// 0x3C SysTick

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak))
NMI_Handler (void)
{
  trace::puts ("[NMI_Handler]");
  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_8M_MAIN__)

// The values of BFAR and MMFAR stay unchanged if the BFARVALID or
// MMARVALID is set. However, if a new fault occurs during the
// execution of this fault handler, the value of the BFAR and MMFAR
// could potentially be erased. In order to ensure the fault addresses
// accessed are valid, the following procedure should be used:
// 1. Read BFAR/MMFAR.
// 2. Read CFSR to get BFARVALID or MMARVALID. If the value is 0, the
//    value of BFAR or MMFAR accessed can be invalid and can be discarded.
// 3. Optionally clear BFARVALID or MMARVALID.
// (See Joseph Yiu's book).

void
dump_exception_stack (exception_stack_frame_s* frame, uint32_t cfsr,
                      uint32_t mmfar, uint32_t bfar, uint32_t lr)
{
  trace::printf ("Stack frame:\n");
  trace::printf (" R0 =  %08" PRIX32 "\n", frame->r0);
  trace::printf (" R1 =  %08" PRIX32 "\n", frame->r1);
  trace::printf (" R2 =  %08" PRIX32 "\n", frame->r2);
  trace::printf (" R3 =  %08" PRIX32 "\n", frame->r3);
  trace::printf (" R12 = %08" PRIX32 "\n", frame->r12);
  trace::printf (" LR =  %08" PRIX32 "\n", frame->lr);
  trace::printf (" PC =  %08" PRIX32 "\n", frame->pc);
  trace::printf (" PSR = %08" PRIX32 "\n", frame->psr);
  trace::printf ("FSR/FAR:\n");
  trace::printf (" CFSR =  %08" PRIX32 "\n", cfsr);
  trace::printf (" HFSR =  %08" PRIX32 "\n", SCB_HFSR);
  trace::printf (" DFSR =  %08" PRIX32 "\n", SCB_DFSR);
  trace::printf (" AFSR =  %08" PRIX32 "\n", SCB_AFSR);

  if (cfsr & (1UL << 7))
    {
      trace::printf (" MMFAR = %08" PRIX32 "\n", mmfar);
    }
  if (cfsr & (1UL << 15))
    {
      trace::printf (" BFAR =  %08" PRIX32 "\n", bfar);
    }
  trace::printf ("Misc\n");
  trace::printf (" LR/EXC_RETURN= %08" PRIX32 "\n", lr);
}

#endif // defined(__ARM_ARCH_7M__) ...

#if defined(__ARM_ARCH_6M__)

void
dump_exception_stack (exception_stack_frame_s* frame, uint32_t lr)
{
  trace::printf ("Stack frame:\n");
  trace::printf (" R0 =  %08" PRIX32 "\n", frame->r0);
  trace::printf (" R1 =  %08" PRIX32 "\n", frame->r1);
  trace::printf (" R2 =  %08" PRIX32 "\n", frame->r2);
  trace::printf (" R3 =  %08" PRIX32 "\n", frame->r3);
  trace::printf (" R12 = %08" PRIX32 "\n", frame->r12);
  trace::printf (" LR =  %08" PRIX32 "\n", frame->lr);
  trace::printf (" PC =  %08" PRIX32 "\n", frame->pc);
  trace::printf (" PSR = %08" PRIX32 "\n", frame->psr);
  trace::printf ("Misc\n");
  trace::printf (" LR/EXC_RETURN= %08" PRIX32 "\n", lr);
}

#endif // defined(__ARM_ARCH_6M__)

#endif // defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_8M_MAIN__)

// Hard Fault handler wrapper in assembly.
// It extracts the location of stack frame and passes it to handler
// in C as a pointer. We also pass the LR value as second
// parameter.
// (Based on Joseph Yiu's, The Definitive Guide to ARM Cortex-M3 and
// Cortex-M4 Processors, Third Edition, Chap. 12.8, page 402).

void __attribute__ ((section (".after_vectors"), weak, naked))
HardFault_Handler (void)
{
  __asm__ volatile (

      " tst lr,#4       \n"
      " ite eq          \n"
      " mrseq r0,msp    \n"
      " mrsne r0,psp    \n"
      " mov r1,lr       \n"
      " ldr r2,=hard_fault_handler_c \n"
      " bx r2"

      : /* Outputs */
      : /* Inputs */
      : /* Clobbers */
  );
}

void __attribute__ ((section (".after_vectors"), weak, used))
hard_fault_handler_c (exception_stack_frame_s* frame __attribute__ ((unused)),
                      uint32_t lr __attribute__ ((unused)))
{
#if defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  uint32_t mmfar = SCB_MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB_BFAR; // Bus Fault Address
  uint32_t cfsr = SCB_CFSR; // Configurable Fault Status Registers

  trace::puts ("[HardFault]");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);

#endif // defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
// || defined(__ARM_ARCH_8M_MAIN__)

#if defined(__ARM_ARCH_6M__)

// Hard Fault handler wrapper in assembly.
// It extracts the location of stack frame and passes it to handler
// in C as a pointer. We also pass the LR value as second
// parameter.
// (Based on Joseph Yiu's, The Definitive Guide to ARM Cortex-M0
// First Edition, Chap. 12.8, page 402).

void __attribute__ ((section (".after_vectors"), weak, naked))
HardFault_Handler (void)
{
  __asm__ volatile (

      " movs r0,#4      \n"
      " mov r1,lr       \n"
      " tst r0,r1       \n"
      " beq 1f          \n"
      " mrs r0,psp      \n"
      " b   2f          \n"
      "1:               \n"
      " mrs r0,msp      \n"
      "2:"
      " mov r1,lr       \n"
      " ldr r2,=hard_fault_handler_c \n"
      " bx r2"

      : /* Outputs */
      : /* Inputs */
      : /* Clobbers */
  );
}

void __attribute__ ((section (".after_vectors"), weak, used))
hard_fault_handler_c (exception_stack_frame_s* frame __attribute__ ((unused)),
                      uint32_t lr __attribute__ ((unused)))
{
#if defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  trace::printf ("[HardFault]\n");
  dump_exception_stack (frame, lr);

#endif // defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_6M__)

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_8M_MAIN__)

void __attribute__ ((section (".after_vectors"), weak))
MemManage_Handler (void)
{
  trace::puts ("[MemManage_Handler]");
  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak, naked))
BusFault_Handler (void)
{
  __asm__ volatile (

      " tst lr,#4       \n"
      " ite eq          \n"
      " mrseq r0,msp    \n"
      " mrsne r0,psp    \n"
      " mov r1,lr       \n"
      " ldr r2,=bus_fault_handler_c \n"
      " bx r2"

      : /* Outputs */
      : /* Inputs */
      : /* Clobbers */
  );
}

void __attribute__ ((section (".after_vectors"), weak, used))
bus_fault_handler_c (exception_stack_frame_s* frame __attribute__ ((unused)),
                     uint32_t lr __attribute__ ((unused)))
{
#if defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  uint32_t mmfar = SCB_MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB_BFAR; // Bus Fault Address
  uint32_t cfsr = SCB_CFSR; // Configurable Fault Status Registers

  trace::puts ("[BusFault]");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);

#endif // defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak, naked))
UsageFault_Handler (void)
{
  __asm__ volatile (

      " tst lr,#4       \n"
      " ite eq          \n"
      " mrseq r0,msp    \n"
      " mrsne r0,psp    \n"
      " mov r1,lr       \n"
      " ldr r2,=usage_fault_handler_c \n"
      " bx r2"

      : /* Outputs */
      : /* Inputs */
      : /* Clobbers */
  );
}

void __attribute__ ((section (".after_vectors"), weak, used))
usage_fault_handler_c (exception_stack_frame_s* frame __attribute__ ((unused)),
                       uint32_t lr __attribute__ ((unused)))
{
#if defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  uint32_t mmfar = SCB_MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB_BFAR; // Bus Fault Address
  uint32_t cfsr = SCB_CFSR; // Configurable Fault Status Registers

  trace::puts ("[UsageFault]");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);

#endif // defined(MICRO_OS_PLUS_DIAG_TRACE_ENABLED)

  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_7M__)...

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

// CoreDebug register address (used instead of the CoreDebug->... accessor,
// since CMSIS is not available here, being device specific).

#define CORE_DEBUG_DHCSR (*(volatile uint32_t*)0xE000EDF0)
#define CORE_DEBUG_DHCSR_C_DEBUGEN_MSK (1UL << 0)

void __attribute__ ((section (".after_vectors"), weak))
SecureFault_Handler (void)
{

#if defined(MICRO_OS_PLUS_DEBUG_ENABLED)

  if ((CORE_DEBUG_DHCSR & CORE_DEBUG_DHCSR_C_DEBUGEN_MSK) != 0)
    {
      cortexm::architecture::bkpt ();
    }

#endif // defined(MICRO_OS_PLUS_DEBUG_ENABLED)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_8M_MAIN__) || defined(__ARM_ARCH_8M_BASE__)

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak))
SVC_Handler (void)
{
  trace::puts ("[SVC_Handler]");
  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) \
    || defined(__ARM_ARCH_8M_MAIN__)

void __attribute__ ((section (".after_vectors"), weak))
DebugMon_Handler (void)
{
  // Don't trigger another BKPT.
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_7M__) ...

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak))
PendSV_Handler (void)
{
  cortexm::architecture::bkpt ();
  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

void __attribute__ ((section (".after_vectors"), weak))
SysTick_Handler (void)
{
  // DO NOT loop, just return.
  // Useful in case someone (like STM HAL) inadvertently enables SysTick.
}

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_INCLUDE_ARCHITECTURES_CORTEXM_EXCEPTION_HANDLERS_ENABLED)

// ----------------------------------------------------------------------------