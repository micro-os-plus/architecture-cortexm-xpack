/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2023-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

// ----------------------------------------------------------------------------

#include "micro-os-plus/architecture.h"
#include "micro-os-plus/diag/trace.h"

#include <cstdint>

// ----------------------------------------------------------------------------

using namespace micro_os_plus;

// ----------------------------------------------------------------------------

// Do not use CMSIS CORE header, as it may not be available in all environments.

#define SCB_CPUID_ADDR 0xE000ED00UL

// ----------------------------------------------------------------------------

/* PartNo  | Core         | Preprocessor macro
 * --------+--------------+------------------------
 * 0xC20   | Cortex-M0    | __ARM_ARCH_6M__
 * 0xC21   | Cortex-M1    | __ARM_ARCH_6M__
 * 0xC23   | Cortex-M3    | __ARM_ARCH_7M__
 * 0xC24   | Cortex-M4    | __ARM_ARCH_7EM__
 * 0xC27   | Cortex-M7    | __ARM_ARCH_7EM__
 * 0xC60   | Cortex-M0+   | __ARM_ARCH_6M__
 * 0xD20   | Cortex-M23   | __ARM_ARCH_8M_BASE__
 * 0xD21   | Cortex-M33   | __ARM_ARCH_8M_MAIN__
 * 0xD22   | Cortex-M55   | __ARM_ARCH_8_1M_MAIN__
 * 0xD23   | Cortex-M85   | __ARM_ARCH_8_1M_MAIN__
 * 0xD24   | Cortex-M52   | __ARM_ARCH_8_1M_MAIN__
 * 0xD31   | Cortex-M35P  | __ARM_ARCH_8M_MAIN__
 */

/* Returns a pointer to a string literal holding the digits (and suffix,
 * where applicable) that follow "Cortex-M" for the detected core. The
 * compile-time __ARM_ARCH_* macro narrows the candidate cores; the
 * runtime PartNo field disambiguates within that architecture. */
static const char*
get_cortex_m_core_suffix (uint16_t part_no)
{
#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_7M__) 

  // Group them since on QEMU the M0 tests run on M3.
  switch (part_no)
    {
    case 0xC20U:
      return "0";
    case 0xC60U:
      return "0+";
    case 0xC21U:
      return "1";
    case 0xC23U:
      return "3";
    default:
      return "?";
    }

#elif defined(__ARM_ARCH_7EM__)

  switch (part_no)
    {
    case 0xC24U:
      return "4";
    case 0xC27U:
      return "7";
    default:
      return "?";
    }

#elif defined(__ARM_ARCH_8M_BASE__)

  switch (part_no)
    {
    case 0xD20U:
      return "23";
    default:
      return "?";
    }

#elif defined(__ARM_ARCH_8M_MAIN__)

  switch (part_no)
    {
    case 0xD21U:
      return "33";
    case 0xD31U:
      return "35P";
    default:
      return "?";
    }

#elif defined(__ARM_ARCH_8_1M_MAIN__)

  switch (part_no)
    {
    case 0xD22U:
      return "55";
    case 0xD23U:
      return "85";
    case 0xD24U:
      return "52";
    default:
      return "?";
    }

#else
#error "Unknown architecture"
#endif // defined(__ARM_ARCH_*__)
}

void
micro_os_plus_architecture_show_cpuid (void)
{
  uint32_t cpuid = *reinterpret_cast<volatile uint32_t*> (SCB_CPUID_ADDR);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#endif // defined(__GNUC__)

  // CPUID fields
  struct
  {
    uint8_t implementer; /* bits [31:24] */
    uint8_t variant; /* bits [23:20] - "r" in rNpM */
    uint8_t architecture; /* bits [19:16] */
    uint16_t part_no; /* bits [15:4]  */
    uint8_t revision; /* bits [3:0]   - "p" in rNpM */
  } fields;

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // defined(__GNUC__)

  fields.implementer = static_cast<uint8_t> ((cpuid >> 24) & 0xFFU);
  fields.variant = static_cast<uint8_t> ((cpuid >> 20) & 0x0FU);
  fields.architecture = static_cast<uint8_t> ((cpuid >> 16) & 0x0FU);
  fields.part_no = static_cast<uint16_t> ((cpuid >> 4) & 0xFFFU);
  fields.revision = static_cast<uint8_t> (cpuid & 0x0FU);

  const char* variant_str = get_cortex_m_core_suffix (fields.part_no);

  trace::printf ("Cortex-M%s r%up%u\n", variant_str, fields.variant,
                 fields.revision);
}

// ----------------------------------------------------------------------------
