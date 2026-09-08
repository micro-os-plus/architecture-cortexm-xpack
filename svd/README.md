# Generic ARMv6-M / ARMv7-M system-peripheral SVDs

This folder contains two hand-synthesised CMSIS-SVD files describing the
**core system peripherals** of the Arm Cortex-M architecture — NVIC, SCB,
SysTick, CoreDebug, and an optional MPU — with no vendor-specific peripherals
attached:

- `armv6m-system-peripherals.svd` — Cortex-M0 / Cortex-M0+ / Cortex-M1 / SC000
- `armv7m-system-peripherals.svd` — Cortex-M3 / Cortex-M4 / Cortex-M7 / SC300

Neither file exists upstream. This document explains why, where the content
actually came from, and what to watch out for when using them.

## Why these had to be built rather than downloaded

The obvious places to look all came up short:

- **Arm's own generic device templates** (`Device/ARM/SVD/ARMCM*.svd` in the
  [CMSIS_5](https://github.com/ARM-software/CMSIS_5) repository) contain only
  a `<cpu>` metadata block — no `<peripherals>` section at all, so no NVIC,
  SCB, SysTick, or CoreDebug register definitions.
- **Vendor SVDs** (checked: ST's STM32F0/F4 family across both an old 2015
  pack and the current 2025 one, Nordic's nRF51, NXP's LPC11xx/LPC800) don't
  define these either, beyond a bare-bones NVIC in some of ST's files. This
  matches a long-standing, still-open complaint in Arm's own tracker —
  [CMSIS_5 issue #844](https://github.com/ARM-software/CMSIS_5/issues/844) —
  that vendors and Arm each treat this as the other's job.
- A few individual registers turn up in isolation elsewhere (e.g. a
  memfault.com blog extracted `SCB`/`CoreDebug`/`FPU_CPACR` from an ST file
  for a fault-debugging tutorial), but nothing complete or architecture-scoped
  exists as a ready-made file.

## What these files were actually built from

Rather than adapt a specific chip's SVD (the first attempt at this, for a
single Cortex-M0 file, produced several wrong bit-widths — see *Corrections*
below), these two files are synthesised directly from Arm's own **CMSIS_5
core header files**, which are the authoritative, hand-maintained C
definitions of these exact registers:

| Header | Used for |
|---|---|
| [`core_cm0.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm0.h) | ARMv6-M baseline: NVIC/SCB register set, confirms no MPU |
| [`core_cm0plus.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm0plus.h) | ARMv6-M optional extras: conditional VTOR, MPU layout (no alias registers) |
| [`core_cm3.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm3.h) | ARMv7-M baseline: full NVIC (8 banks), full SCB fault-handling set, MPU with 3 alias pairs |
| [`core_cm4.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm4.h) | Cross-check against `core_cm3.h` — confirmed identical at the SCB/NVIC/MPU level (M4 only adds a separate, deliberately-excluded FPU block) |

Each register and field's address offset, bit position, and access type in
the two `.svd` files was read directly from that header's `_Type` struct
definitions and its `#define <REG>_<FIELD>_Pos` / `_Msk` macros — not
retyped from memory. `build_generic.py` and `build_final.py` in this folder
are the actual synthesis scripts, kept for reproducibility; they parse the
downloaded headers only indirectly (the header-derived facts were transcribed
into the scripts as literal field tables after inspection), so re-running
them regenerates the same output without needing network access.

Two facts about ARMv6-M that shaped the file, both taken from explicit
comments in `core_cm0.h`/`core_cm0plus.h` rather than inferred:

1. `__VTOR_PRESENT`, `__MPU_PRESENT`, and `__NVIC_PRIO_BITS` are
   **implementation-defined per device**, not fixed by the architecture —
   confirmed independently against the [CMSIS-Core device capability
   defines](https://arm-software.github.io/CMSIS_6/main/Core/group__device__config.html).
2. On ARMv6-M, `SCB.SHCSR`, `SCB.DFSR`, and the whole `CoreDebug` block are
   **only accessible over the external Debug Access Port**, not from
   application code — the header literally says so: *"Core Debug Registers
   (DCB registers, SHCSR, and DFSR) are only accessible over DAP and not via
   processor."* This restriction does not apply on ARMv7-M.

## What's in each file

Both files carry the same four "always present" peripherals plus one that's
architecturally optional:

- **NVIC** (`0xE000E100`) — v6-M: one `ISER`/`ICER`/`ISPR`/`ICPR` bank + 8
  `IPR` registers (the architecture caps external interrupts at 32). v7-M:
  8 banks each of `ISER`/`ICER`/`ISPR`/`ICPR`/`IABR`, 60 `IPR` registers, and
  `STIR` — sized to the full 240-interrupt architectural maximum, matching
  how CMSIS's own `NVIC_Type` is declared generically. A real device only
  implements as many of these as it has interrupt lines; the rest read as 0.
- **SCB** (`0xE000ED00`) — v6-M: `CPUID`, `ICSR`, `VTOR` (optional),
  `AIRCR`, `SCR`, `CCR`, `SHPR2`, `SHPR3`, `SHCSR` (one bit only). v7-M adds
  `SHPR1`, the full `SHCSR`, `CFSR`/`HFSR`/`DFSR`/`MMFAR`/`BFAR`/`AFSR`, the
  `PFR`/`DFR`/`ADR`/`MMFR`/`ISAR` ID registers, and `CPACR`.
- **MPU** (`0xE000ED90`, optional on both architectures) — v6-M/M0+ shape has
  no alias registers; v7-M shape adds 3 `RBAR`/`RASR` alias pairs.
- **SysTick** (`0xE000E010`) — identical on both; `CTRL`/`LOAD`/`VAL`/`CALIB`.
- **CoreDebug** (`0xE000EDF0`) — identical on both; `DHCSR`/`DCRSR`/`DCRDR`/
  `DEMCR`. This is fixed ADIv5/CoreSight debug architecture, not part of the
  M-profile ISA, so the register layout doesn't change across the family —
  only its *accessibility* does (see above).

Deliberately **excluded** from both: the FPU/`FPU_CPACR` register block
(Cortex-M4/M7-specific, not part of plain ARMv7-M — M3 has no FPU), and
trace components (`ITM`/`DWT`/`TPIU`) — out of scope for "system
peripherals" as originally asked. Ask if you want either added.

## Corrections made along the way

An earlier, single-device attempt (`armcm0-with-system-peripherals.svd`,
also in this delivery history) adapted one real vendor's Cortex-M4 SCB down
to Cortex-M0 by hand and got three things wrong that the header-driven
rebuild here fixes:

- `ICSR.VECTACTIVE`/`VECTPENDING` were narrowed to 6 bits on the assumption
  that Cortex-M0 needs fewer than the 9 bits used on M3/M4 — wrong; both are
  9 bits on ARMv6-M too (confirmed via `SCB_ICSR_VECTACTIVE_Msk` in
  `core_cm0.h`).
- `RETTOBASE` was included in `ICSR` — it doesn't exist on ARMv6-M at all
  (`core_cm0.h` has no such define; `core_cm3.h` does).
- `SHCSR` was omitted entirely — it does exist on Cortex-M0, just with a
  single implemented bit, `SVCALLPENDED`.

If you're comparing against that earlier file, treat the two files in this
folder as the corrected, superseding versions.

## Known gaps / things not independently verified

- `CPUID` register field **layout** (`Implementer`/`Variant`/`Constant`/
  `PartNo`/`Revision`) is confirmed from the header; the actual **values**
  are implementation- and silicon-revision-specific and were deliberately
  left as a neutral `0x00000000` reset value rather than guessed.
- `SCB.DFSR`'s bit layout is taken from the ARMv7-M header and reused
  as-is for the ARMv6-M file's description text (it isn't exposed as a named
  struct field in `core_cm0.h`, only mentioned as existing and DAP-only) —
  the register itself isn't included as a defined peripheral register in the
  v6-M file at all; this is worth a closer look if you specifically need it.
- ID registers on the v7-M file (`PFR`, `DFR`, `ADR`, `MMFR`, `ISAR`) are
  included as address placeholders only, with no field breakdown — their
  content is silicon-specific and Arm doesn't publish a single canonical
  decode for them outside the full architecture reference manual.

## Licensing / provenance

Field names, offsets, and descriptions are transcribed from Arm's CMSIS_5
repository, which is Apache License 2.0. These two files are derivative of
that header content, not of any single silicon vendor's copyrighted SVD.

## Regenerating or extending

`build_generic.py` defines one Python builder function per peripheral
(`build_nvic_v6m`, `build_scb_v7m`, `build_mpu_v6m`, etc.); `build_final.py`
assembles them into the two complete `<device>` documents. To add a
peripheral (FPU, ITM/DWT, or a full ARMv8-M pass), add a new builder function
following the same pattern and call it from `build_final.py`.
