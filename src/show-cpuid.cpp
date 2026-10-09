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

/**
 * @file
 * @brief Cortex-M implementation of the CPU identification display.
 *
 * @details
 * Decodes the `SCB->CPUID` (CPUID Base Register) fields and writes a line
 * like `Arm Cortex-M3 r2p1` to the trace output channel.
 *
 * The implementation is kept consistent with the AArch32 and AArch64
 * packages, which decode the equivalent `MIDR` register.
 */

#include "micro-os-plus/architecture.h"

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ENABLED)

// The trace output is optional; without the `diag-trace` package, the
// function is empty.
#if __has_include("micro-os-plus/diag/trace.h")
#include "micro-os-plus/diag/trace.h"

#include <cstdint>

// ----------------------------------------------------------------------------

namespace
{
  /**
   * @brief Address of the `SCB->CPUID` register.
   *
   * @details
   * The register is at the same address on all Cortex-M cores. The CMSIS
   * Core headers are not used, since they may not be available in all
   * environments.
   */
  constexpr std::uintptr_t scb_cpuid_address = 0xE000ED00U;

  /**
   * @brief Get the name of a CPU implementer.
   *
   * @details
   * The implementer codes are assigned by Arm; only the most common ones
   * are listed. The list is the same as in the AArch32 and AArch64
   * packages.
   *
   * @param implementer The `CPUID.Implementer` field (bits [31:24]).
   * @return A pointer to a string literal with the implementer name, or
   * `nullptr` if the code is not known.
   */
  [[nodiscard]] const char*
  get_implementer_name (std::uint32_t implementer) noexcept
  {
    switch (implementer)
      {
      case 0x41U:
        return "Arm";
      case 0x42U:
        return "Broadcom";
      case 0x43U:
        return "Cavium";
      case 0x46U:
        return "Fujitsu";
      case 0x48U:
        return "HiSilicon";
      case 0x4EU:
        return "NVIDIA";
      case 0x50U:
        return "APM";
      case 0x51U:
        return "Qualcomm";
      case 0x61U:
        return "Apple";
      case 0x6DU:
        return "Microsoft";
      case 0xC0U:
        return "Ampere";
      default:
        return nullptr;
      }
  }

  /**
   * @brief Get the name of an Arm Ltd. Cortex-M core.
   *
   * @details
   * Only meaningful when the implementer is Arm (`0x41`). The part
   * number alone identifies the core, therefore the lookup does not
   * depend on the architecture the code was compiled for (for example
   * Armv6-M code running on a Cortex-M3, as in the QEMU tests).
   *
   * @param part_num The `CPUID.PartNo` field (bits [15:4]).
   * @return A pointer to a string literal with the core name, or
   * `nullptr` if the part number is not known.
   */
  [[nodiscard]] const char*
  get_arm_part_name (std::uint32_t part_num) noexcept
  {
    switch (part_num)
      {
      // Armv6-M.
      case 0xC20U:
        return "Cortex-M0";
      case 0xC21U:
        return "Cortex-M1";
      case 0xC60U:
        return "Cortex-M0+";
      // Armv7-M and Armv7E-M.
      case 0xC23U:
        return "Cortex-M3";
      case 0xC24U:
        return "Cortex-M4";
      case 0xC27U:
        return "Cortex-M7";
      // Armv8-M.
      case 0xD20U:
        return "Cortex-M23";
      case 0xD21U:
        return "Cortex-M33";
      case 0xD31U:
        return "Cortex-M35P";
      // Armv8.1-M.
      case 0xD22U:
        return "Cortex-M55";
      case 0xD23U:
        return "Cortex-M85";
      case 0xD24U:
        return "Cortex-M52";
      default:
        return nullptr;
      }
  }
} // namespace

#endif // __has_include("micro-os-plus/diag/trace.h")

// ----------------------------------------------------------------------------

/**
 * @details
 * Reads `SCB->CPUID`, decodes its fields, and writes them to the trace
 * output channel:
 *
 * - Implementer, bits [31:24], as a name when known (e.g. `Arm`);
 * - PartNo, bits [15:4], as a core name when the implementer is Arm and
 *   the part is known (e.g. `Cortex-M3`), otherwise in hexadecimal;
 * - Variant, bits [23:20], and Revision, bits [3:0], as `rVpR`;
 * - Architecture, bits [19:16], only when it is not `0xF`, the value
 *   used by Armv7-M and later cores; Armv6-M cores (Cortex-M0, M0+,
 *   and M1) report `0xC`.
 *
 * When the `diag-trace` package is not available, the function is empty.
 */
void
micro_os_plus_architecture_show_cpuid (void)
{
#if __has_include("micro-os-plus/diag/trace.h")
  using namespace micro_os_plus;

  const std::uint32_t cpuid
      = *reinterpret_cast<volatile std::uint32_t*> (scb_cpuid_address);

  // On arm-none-eabi `std::uint32_t` is `unsigned long`; the fields are
  // converted to `unsigned int`, to match the `%u` and `%X` formats.
  const auto implementer = static_cast<unsigned int> ((cpuid >> 24) & 0xFFU);
  const auto variant = static_cast<unsigned int> ((cpuid >> 20) & 0x0FU);
  const auto architecture = static_cast<unsigned int> ((cpuid >> 16) & 0x0FU);
  const auto part_num = static_cast<unsigned int> ((cpuid >> 4) & 0xFFFU);
  const auto revision = static_cast<unsigned int> (cpuid & 0x0FU);

  const char* implementer_name = get_implementer_name (implementer);
  const char* part_name
      = (implementer == 0x41U) ? get_arm_part_name (part_num) : nullptr;

  if (implementer_name != nullptr)
    {
      trace::printf ("%s ", implementer_name);
    }
  else
    {
      trace::printf ("Implementer 0x%02X ", implementer);
    }

  if (part_name != nullptr)
    {
      trace::printf ("%s", part_name);
    }
  else
    {
      trace::printf ("part 0x%03X", part_num);
    }

  trace::printf (" r%up%u", variant, revision);

  if (architecture != 0xFU)
    {
      trace::printf (" (architecture 0x%X)", architecture);
    }

  trace::printf ("\n");
#endif // __has_include("micro-os-plus/diag/trace.h")
}

// ----------------------------------------------------------------------------

#endif // defined(MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ENABLED)

// ----------------------------------------------------------------------------
