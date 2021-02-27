/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
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

#if defined(__ARM_EABI__)

/// ----------------------------------------------------------------------------

#include <micro-os-plus/device.h>
#include <micro-os-plus/architecture-cortexm/exception-handlers.h>
// #include <micro-os-plus/startup/defines.h>

#include <micro-os-plus/semihosting.h>
#include <micro-os-plus/diag/trace.h>

#include <string.h>

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

extern "C"
{
  extern unsigned int __heap_end__;
  extern unsigned int __stack;

  void __attribute__ ((noreturn, weak)) _start (void);
}

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

// The stack must be functional at this point and the pointer
// to it correctly set at 0x00000004.
// For debugging purposes, it is possible to set a breakpoint here.
// To create a proper stack frame, do not jump, but call `_start()`.

void __attribute__ ((section (".after_vectors"), noreturn))
Reset_Handler (void)
{
  // __disable_irq ();

  // Fill the main stack with a pattern, to detect usage and underflow.
  for (unsigned int* p = &__heap_end__; p < &__stack;)
    {
      *p++ = MICRO_OS_PLUS_INTEGER_STARTUP_STACK_FILL_MAGIC; // DEADBEEF
    }

  _start ();
}

void __attribute__ ((section (".after_vectors"), weak)) NMI_Handler (void)
{
#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

// ----------------------------------------------------------------------------

#if defined(TRACE)

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

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
  trace::printf (" R0 =  %08X\n", frame->r0);
  trace::printf (" R1 =  %08X\n", frame->r1);
  trace::printf (" R2 =  %08X\n", frame->r2);
  trace::printf (" R3 =  %08X\n", frame->r3);
  trace::printf (" R12 = %08X\n", frame->r12);
  trace::printf (" LR =  %08X\n", frame->lr);
  trace::printf (" PC =  %08X\n", frame->pc);
  trace::printf (" PSR = %08X\n", frame->psr);
  trace::printf ("FSR/FAR:\n");
  trace::printf (" CFSR =  %08X\n", cfsr);
  trace::printf (" HFSR =  %08X\n", SCB->HFSR);
  trace::printf (" DFSR =  %08X\n", SCB->DFSR);
  trace::printf (" AFSR =  %08X\n", SCB->AFSR);

  if (cfsr & (1UL << 7))
    {
      trace::printf (" MMFAR = %08X\n", mmfar);
    }
  if (cfsr & (1UL << 15))
    {
      trace::printf (" BFAR =  %08X\n", bfar);
    }
  trace::printf ("Misc\n");
  trace::printf (" LR/EXC_RETURN= %08X\n", lr);
}

#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(__ARM_ARCH_6M__)

void
dump_exception_stack (exception_stack_frame_s* frame, uint32_t lr)
{
  trace::printf ("Stack frame:\n");
  trace::printf (" R0 =  %08X\n", frame->r0);
  trace::printf (" R1 =  %08X\n", frame->r1);
  trace::printf (" R2 =  %08X\n", frame->r2);
  trace::printf (" R3 =  %08X\n", frame->r3);
  trace::printf (" R12 = %08X\n", frame->r12);
  trace::printf (" LR =  %08X\n", frame->lr);
  trace::printf (" PC =  %08X\n", frame->pc);
  trace::printf (" PSR = %08X\n", frame->psr);
  trace::printf ("Misc\n");
  trace::printf (" LR/EXC_RETURN= %08X\n", lr);
}

#endif // defined(__ARM_ARCH_6M__)

#endif // defined(TRACE)

// ----------------------------------------------------------------------------

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG)

int
is_semihosting_call (exception_stack_frame_s* frame, uint16_t opCode);

/**
 * This function provides the minimum functionality to make a
 * semihosting program execute even without the debugger present.
 * @param frame pointer to an exception stack frame.
 * @param opCode the 16-bin word of the BKPT instruction.
 * @return 1 if the instruction was a valid semihosting call; 0 otherwise.
 */
int
is_semihosting_call (exception_stack_frame_s* frame, uint16_t opCode)
{
  uint16_t* pw = (uint16_t*)frame->pc;
  if (*pw == opCode)
    {
      uint32_t r0 = frame->r0;
#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS) \
    || defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)
      uint32_t r1 = frame->r1;
#endif
#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)
      uint32_t* blk = (uint32_t*)r1;
#endif

#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
      // trace::printf ("sh r0=%d\n", r0);
#endif

      switch (r0)
        {

#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS)

        case SEMIHOSTING_SYS_CLOCK:
        case SEMIHOSTING_SYS_ELAPSED:
        case SEMIHOSTING_SYS_FLEN:
        case SEMIHOSTING_SYS_GET_CMDLINE:
        case SEMIHOSTING_SYS_REMOVE:
        case SEMIHOSTING_SYS_RENAME:
        case SEMIHOSTING_SYS_SEEK:
        case SEMIHOSTING_SYS_SYSTEM:
        case SEMIHOSTING_SYS_TICKFREQ:
        case SEMIHOSTING_SYS_TMPNAM:
        case SEMIHOSTING_SYS_ISTTY:
          frame->r0
              = (uint32_t)-1; // the call is not successful or not supported
          break;

        case SEMIHOSTING_SYS_CLOSE:
          frame->r0 = 0; // call is successful
          break;

        case SEMIHOSTING_SYS_ERRNO:
          frame->r0 = 0; // the value of the C library errno variable.
          break;

        case SEMIHOSTING_SYS_HEAPINFO:
          blk[0] = 0; // heap_base
          blk[1] = 0; // heap_limit
          blk[2] = 0; // stack_base
          blk[3] = 0; // stack_limit
          break;

        case SEMIHOSTING_SYS_ISERROR:
          frame->r0 = 0; // 0 if the status word is not an error indication
          break;

        case SEMIHOSTING_SYS_READ:
          // If R0 contains the same value as word 3, the call has
          // failed and EOF is assumed.
          frame->r0 = blk[2];
          break;

        case SEMIHOSTING_SYS_READC:
          frame->r0 = '\0'; // the byte read from the console.
          break;

        case SEMIHOSTING_SYS_TIME:
          frame->r0 = 0; // the number of seconds since 00:00 January 1, 1970.
          break;

        case SEMIHOSTING_SYS_EXIT:

          NVIC_SystemReset ();
          // Should not reach here
          return 0;

#endif // defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS)

#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)

#define HANDLER_STDIN (1)
#define HANDLER_STDOUT (2)
#define HANDLER_STDERR (3)

        case SEMIHOSTING_SYS_OPEN:
          // Process only standard io/out/err and return 1/2/3
          if (strcmp ((char*)blk[0], ":tt") == 0)
            {
              if ((blk[1] == 0))
                {
                  frame->r0 = HANDLER_STDIN;
                  break;
                }
              else if (blk[1] == 4)
                {
                  frame->r0 = HANDLER_STDOUT;
                  break;
                }
              else if (blk[1] == 8)
                {
                  frame->r0 = HANDLER_STDERR;
                  break;
                }
            }
          frame->r0
              = (uint32_t)-1; // the call is not successful or not supported
          break;

        case SEMIHOSTING_SYS_WRITE:
          // Silently ignore writes to stdout/stderr, fail on all other
          // handler.
          if ((blk[0] == HANDLER_STDOUT) || (blk[0] == HANDLER_STDERR))
            {
#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
              frame->r0
                  = (uint32_t)blk[2] - trace_write ((char*)blk[1], blk[2]);
#else
              frame->r0 = 0; // all sent, no more.
#endif // defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
            }
          else
            {
              // If other handler, return the total number of bytes
              // as the number of bytes that are not written.
              frame->r0 = blk[2];
            }
          break;

#endif // defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) ||
       // defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT)

#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG)

        case SEMIHOSTING_SYS_WRITEC:
#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
          {
            char ch = *((char*)r1);
            trace_write (&ch, 1);
          }
#endif
          // Register R0 is corrupted.
          break;

        case SEMIHOSTING_SYS_WRITE0:
#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)
          {
            char* p = ((char*)r1);
            trace_write (p, strlen (p));
          }
#endif
          // Register R0 is corrupted.
          break;

#endif

        default:
          return 0;
        }

      // Alter the PC to make the exception returns to
      // the instruction after the faulty BKPT.
      frame->pc += 2;
      return 1;
    }
  return 0;
}

#endif

// Hard Fault handler wrapper in assembly.
// It extracts the location of stack frame and passes it to handler
// in C as a pointer. We also pass the LR value as second
// parameter.
// (Based on Joseph Yiu's, The Definitive Guide to ARM Cortex-M3 and
// Cortex-M4 Processors, Third Edition, Chap. 12.8, page 402).

void __attribute__ ((section (".after_vectors"), weak, naked))
HardFault_Handler (void)
{
  __asm__ volatile(

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
#if defined(TRACE)
  uint32_t mmfar = SCB->MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB->BFAR; // Bus Fault Address
  uint32_t cfsr = SCB->CFSR; // Configurable Fault Status Registers
#endif

#if defined(MICRO_OS_PLUS_USE_SEMIHOSTING_SYSCALLS) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_STDOUT) \
    || defined(MICRO_OS_PLUS_USE_TRACE_SEMIHOSTING_DEBUG)

  // If the BKPT instruction is executed with C_DEBUGEN == 0 and MON_EN == 0,
  // it will cause the processor to enter a HardFault exception, with DEBUGEVT
  // in the Hard Fault Status register (HFSR) set to 1, and BKPT in the
  // Debug Fault Status register (DFSR) also set to 1.

  if (((SCB->DFSR & SCB_DFSR_BKPT_Msk) != 0)
      && ((SCB->HFSR & SCB_HFSR_DEBUGEVT_Msk) != 0))
    {
      if (is_semihosting_call (frame, 0xBE00 + (AngelSWI & 0xFF)))
        {
          // Clear the exception cause in exception status.
          SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk;

          // Continue after the BKPT
          return;
        }
    }

#endif // semihosting

#if defined(TRACE)
  trace::printf ("[HardFault]\n");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);
#endif // defined(TRACE)

#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

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
  __asm__ volatile(

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
  // There is no semihosting support for Cortex-M0, since on ARMv6-M
  // faults are fatal and it is not possible to return from the handler.

#if defined(TRACE)
  trace::printf ("[HardFault]\n");
  dump_exception_stack (frame, lr);
#endif // defined(TRACE)

#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif // defined(__ARM_ARCH_6M__)

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

void __attribute__ ((section (".after_vectors"), weak))
MemManage_Handler (void)
{
#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

void __attribute__ ((section (".after_vectors"), weak, naked))
BusFault_Handler (void)
{
  __asm__ volatile(

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
#if defined(TRACE)
  uint32_t mmfar = SCB->MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB->BFAR; // Bus Fault Address
  uint32_t cfsr = SCB->CFSR; // Configurable Fault Status Registers

  trace::printf ("[BusFault]\n");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);
#endif // defined(TRACE)

#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

void __attribute__ ((section (".after_vectors"), weak, naked))
UsageFault_Handler (void)
{
  __asm__ volatile(

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
#if defined(TRACE)
  uint32_t mmfar = SCB->MMFAR; // MemManage Fault Address
  uint32_t bfar = SCB->BFAR; // Bus Fault Address
  uint32_t cfsr = SCB->CFSR; // Configurable Fault Status Registers
#endif

#if defined(MICRO_OS_PLUS_DEBUG_SEMIHOSTING_FAULTS)

  if ((cfsr & (1UL << 16)) != 0) // UNDEFINSTR
    {
      // For testing purposes, instead of BKPT use 'setend be'.
      if (is_semihosting_call (frame, AngelSWITestFaultOpCode))
        {
          return;
        }
    }

#endif

#if defined(TRACE)
  trace::printf ("[UsageFault]\n");
  dump_exception_stack (frame, cfsr, mmfar, bfar, lr);
#endif // defined(TRACE)

#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif

void __attribute__ ((section (".after_vectors"), weak)) SVC_Handler (void)
{
#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

void __attribute__ ((section (".after_vectors"), weak)) DebugMon_Handler (void)
{
#if defined(DEBUG)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

#endif

void __attribute__ ((section (".after_vectors"), weak)) PendSV_Handler (void)
{
#if defined(DEBUG)
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
  if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
      cortexm::architecture::bkpt ();
    }
#else
  cortexm::architecture::bkpt ();
#endif // defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#endif // defined(DEBUG)

  while (1)
    {
      cortexm::architecture::wfi ();
    }
}

void __attribute__ ((section (".after_vectors"), weak)) SysTick_Handler (void)
{
  // DO NOT loop, just return.
  // Useful in case someone (like STM HAL) inadvertently enables SysTick.
  ;
}

// ----------------------------------------------------------------------------

#endif // defined(__ARM_EABI__)

// ----------------------------------------------------------------------------
