# Generic ARMv6-M / ARMv7-M / ARMv8-M system-peripheral SVDs

This folder contains four hand-synthesised CMSIS-SVD files describing the
**core system peripherals** of the Arm Cortex-M architecture — NVIC, SCB,
SysTick, CoreDebug/DCB, and an optional MPU — with no vendor-specific
peripherals attached:

- `armv6m-system-peripherals.svd` — Cortex-M0 / Cortex-M0+ / Cortex-M1 / SC000
- `armv7m-system-peripherals.svd` — Cortex-M3 / Cortex-M4 / Cortex-M7 / SC300
- `armv8m-baseline-system-peripherals.svd` — Cortex-M23
- `armv8m-mainline-system-peripherals.svd` — Cortex-M33 / Cortex-M35P /
  Cortex-M55 / Cortex-M85

Unlike ARMv6-M/ARMv7-M (each a single profile), ARMv8-M splits into two
quite different profiles -- Baseline (extends ARMv6-M's register set) and
Mainline (extends ARMv7-M's) -- so it gets two files rather than one. Both
ARMv8-M files also model the optional Security Extension (TrustZone): the
SAU peripheral, NVIC's ITNS register bank, SCB's NSACR/SFSR/SFAR, and the
Secure-only fields scattered across ICSR/AIRCR/SCR/SHCSR/DCB -- all
documented as RAZ/WI (or fixed) when the Security Extension isn't
implemented, same treatment as the other implementation-defined bits below.

None of these four files exist upstream. This document explains why, where
the content actually came from, and what to watch out for when using them.

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
below), these four files are synthesised directly from Arm's own **CMSIS_5
core header files**, which are the authoritative, hand-maintained C
definitions of these exact registers:

| Header | Used for |
|---|---|
| [`core_cm0.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm0.h) | ARMv6-M baseline: NVIC/SCB register set, confirms no MPU |
| [`core_cm0plus.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm0plus.h) | ARMv6-M optional extras: conditional VTOR, MPU layout (no alias registers) |
| [`core_cm3.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm3.h) | ARMv7-M baseline: full NVIC (8 banks), full SCB fault-handling set, MPU with 3 alias pairs |
| [`core_cm4.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_cm4.h) | Cross-check against `core_cm3.h` — confirmed identical at the SCB/NVIC/MPU level (M4 only adds a separate, deliberately-excluded FPU block) |
| [`core_armv8mbl.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_armv8mbl.h) | ARMv8-M Baseline (Cortex-M23): NVIC/SCB/MPU/SAU/DCB register set |
| [`core_armv8mml.h`](https://github.com/ARM-software/CMSIS_5/blob/develop/CMSIS/Core/Include/core_armv8mml.h) | ARMv8-M Mainline (Cortex-M33): full NVIC (16 banks), full SCB fault-handling set plus Security Extension registers, MPU with 3 alias pairs, SAU, DCB |

Each register and field's address offset, bit position, and access type in
the four `.svd` files was read directly from that header's `_Type` struct
definitions and its `#define <REG>_<FIELD>_Pos` / `_Msk` macros — not
retyped from memory. `build_generic.py` and `build_final.py` in this folder
are the actual synthesis scripts, kept for reproducibility; `build_final.py`
embeds the (identical, boilerplate) CMSIS license text and `<cpu>` block
inline rather than reading them from a local CMSIS_5 clone, and
`build_generic.py`'s builder functions encode the header-derived facts as
literal field tables after inspection — so re-running `python3
build_final.py` regenerates all four files byte-for-byte without needing
network access or a CMSIS_5 checkout.

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
   processor."* This restriction does not apply on ARMv7-M or ARMv8-M.

## What's in each file

The v6-M/v7-M files carry the same four "always present" peripherals plus
one that's architecturally optional; the two v8-M files add a fifth
(optional) peripheral, the SAU, and rename `CoreDebug` to `DCB`:

- **NVIC** (`0xE000E100`) — v6-M: one `ISER`/`ICER`/`ISPR`/`ICPR` bank + 8
  `IPR` registers (the architecture caps external interrupts at 32). v7-M:
  8 banks each of `ISER`/`ICER`/`ISPR`/`ICPR`/`IABR`, 60 `IPR` registers, and
  `STIR` — sized to the full 240-interrupt architectural maximum. v8-M: 16
  banks each of `ISER`/`ICER`/`ISPR`/`ICPR`/`IABR`/`ITNS`, 124 `IPR`
  registers — sized to the full 496-interrupt architectural maximum — and,
  on Mainline only, `STIR` (no `STIR` on Baseline, matching v6-M). `ITNS`
  (Interrupt Target Non-Secure) is new in v8-M and only meaningful with the
  Security Extension. All three profiles' `NVIC_Type` are declared to their
  architectural maximum in CMSIS itself; a real device only implements as
  many of these as it has interrupt lines, the rest read as 0.
- **SCB** (`0xE000ED00`) — v6-M: `CPUID`, `ICSR`, `VTOR` (optional),
  `AIRCR`, `SCR`, `CCR`, `SHPR2`, `SHPR3`, `SHCSR` (one bit only). v7-M adds
  `SHPR1`, the full `SHCSR`, `CFSR`/`HFSR`/`DFSR`/`MMFAR`/`BFAR`/`AFSR`, the
  `PFR`/`DFR`/`ADR`/`MMFR`/`ISAR` ID registers, and `CPACR`. v8-M Baseline
  keeps v6-M's minimal set (plus `RETTOBASE` and the Security Extension
  fields — see below); v8-M Mainline matches v7-M's full set (`ID_ISAR` grows
  to 6 words, and `ID_AFR`/`ID_DFR` replace `ADR`/`DFR` as the current CMSIS
  names) plus `NSACR`, `SFSR`, and `SFAR`. **v8-M-only fields**, RAZ/WI or
  fixed-value without the Security Extension: `ICSR.STTNS`,
  `AIRCR.SYSRESETREQS`/`BFHFNMINS`/`PRIS`, `SCR.SLEEPDEEPS`,
  `SHCSR.SECUREFAULT*` (Mainline only). `CCR.NONBASETHRDENA`/`STKALIGN` no
  longer exist as configurable bits on v8-M (both behaviors are
  unconditional); v8-M Baseline additionally has no `AIRCR.PRIGROUP` (fixed
  priority split, like v6-M). SCB's own registers are not contiguous on the
  Mainline file: MPU and SAU are separate peripherals whose address ranges
  fall *inside* SCB's own `0xE000ED00`-`0xE000EDEF` page (an actual CMSIS
  quirk, not a modeling choice), so that file's SCB peripheral has two
  `<addressBlock>` entries instead of one.
- **MPU** (`0xE000ED90`, optional on every profile) — v6-M/M0+ shape has no
  alias registers; v7-M shape adds 3 `RBAR`/`RASR` alias pairs. v8-M changes
  the region-defining registers from base+size-encoded `RBAR`/`RASR` to
  base+limit `RBAR`/`RLAR`, and adds `MAIR0`/`MAIR1`; Baseline has no alias
  registers (like v6-M), Mainline has 3 `RBAR`/`RLAR` alias pairs (like v7-M).
- **SAU** (`0xE000EDD0`, v8-M only, optional — present only with the Security
  Extension) — `CTRL`, `TYPE`, and, only if `__SAUREGION_PRESENT`, `RNR`/
  `RBAR`/`RLAR`. Identical on both v8-M profiles. `SFSR`/`SFAR` are
  physically the same registers CMSIS also exposes here, but are modeled
  once, under SCB, to avoid declaring one physical register in two
  peripherals (see the SCB entry above).
- **SysTick** (`0xE000E010`) — identical on all four files; `CTRL`/`LOAD`/
  `VAL`/`CALIB`.
- **CoreDebug** / **DCB** (`0xE000EDF0`) — `DHCSR`/`DCRSR`/`DCRDR`/`DEMCR` on
  every profile; v8-M (where CMSIS renames the block `DCB`) adds
  `DAUTHCTRL`/`DSCSR` (Security Extension) and widens `DCRSR.REGSEL` from 5
  to 7 bits. This is fixed ADIv5/CoreSight debug architecture, not part of
  the M-profile ISA, so the core register layout is stable across the whole
  family — only its *accessibility* (v6-M) and *extent* (v8-M) change.

Deliberately **excluded** from all four: the FPU/`FPU_CPACR`/`MVFR0-2`
register blocks (Cortex-M4/M7/M33-with-FPU-specific, not part of the plain
architecture), the cache-maintenance registers that exist on cache-capable
implementations (`CLIDR`/`CTR`/`CCSIDR`/`CSSELR`, `ICIALLU`..`BPIALL`,
`CCR.DC`/`IC`/`BP` — Cortex-M7/M55/M85-specific), and trace components
(`ITM`/`DWT`/`TPIU`) — out of scope for "system peripherals" as originally
asked. Ask if you want any of these added.

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
- ID registers on the v7-M/v8-M-Mainline files (`PFR`/`ID_PFR`, `DFR`/
  `ID_DFR`, `ADR`/`ID_AFR`, `MMFR`/`ID_MMFR`, `ISAR`/`ID_ISAR`) are included
  as address placeholders only, with no field breakdown — their content is
  silicon-specific and Arm doesn't publish a single canonical decode for
  them outside the full architecture reference manual.
- The two v8-M files' `<register>` elements put `<dim>`/`<dimIncrement>`
  *after* `<name>`, and leave a couple of implementation-defined registers
  (`ID_DFR`, `ID_AFR`) as a self-closing `<fields/>`. Both match the existing
  v7-M file's convention exactly (checked by diffing this session's
  regenerated output against the previously-committed v6-M/v7-M files byte
  for byte) but both are technically stricter-than-necessary violations of
  the official `CMSIS-SVD.xsd` schema (which wants the `dim*` group before
  `name`, and `<fields>` to contain at least one `<field>` when present) —
  `xmllint --schema CMSIS-SVD.xsd` flags them on all three multi-register
  files (only the dim-free v6-M file passes outright). Left as-is for
  consistency with the pre-existing files rather than fixed only in the new
  ones; worth a follow-up pass across all four if strict schema validation
  ever matters for your toolchain.
- `NSACR.CP` (coprocessors 0-7, one bit each) and `SAU.RNR`/`RBAR`/`RLAR`'s
  "only present if implemented" conditions are modeled as always-present
  registers, per this repo's convention of keeping optional-per-device
  content addressable and documenting the condition in prose rather than
  omitting the register outright (same treatment MPU and VTOR already get
  in the v6-M/v7-M files).

## Licensing / provenance

Field names, offsets, and descriptions are transcribed from Arm's CMSIS_5
repository, which is Apache License 2.0. These four files are derivative of
that header content, not of any single silicon vendor's copyrighted SVD.

## Regenerating or extending

`build_generic.py` defines one Python builder function per peripheral
(`build_nvic_v6m`, `build_scb_v7m`, `build_nvic_v8m`, `build_scb_v8m_mainline`,
`build_sau_v8m`, `build_dcb_v8m`, etc. — the v8-M ones take a `mainline: bool`
where baseline/mainline differ); `build_final.py` assembles them into the
four complete `<device>` documents, embedding the shared CMSIS license text
and each device's `<cpu>` block directly (no local CMSIS_5 checkout needed).
Run `python3 build_final.py` from this folder to regenerate all four files
in place; it also re-parses and prints a quick per-peripheral register
summary for each as a sanity check. To add a peripheral (FPU, ITM/DWT, or
ARMv8.1-M's extra registers), add a new builder function following the same
pattern and call it from `build_final.py`.
