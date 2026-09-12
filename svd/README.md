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

All four are kept current against **CMSIS_6** (the
[ARM-software/CMSIS_6](https://github.com/ARM-software/CMSIS_6) repository,
which replaced CMSIS_5 as Arm's canonical Core component and reorganised the
per-core headers along the way — see [Updated for
CMSIS_6](#updated-for-cmsis_6) below for exactly what changed versus the
CMSIS_5-based files these started as). Specifically verified against
`ARM-software/CMSIS_6@main` commit
[`26206e4`](https://github.com/ARM-software/CMSIS_6/commit/26206e47dcf0abfbdc64eb753a0b6334b24439f6)
(2026-09-11), i.e. CMSIS-Core(M) **v6.3.1-dev** per that commit's
`cmsis_version.h`/`ARM.CMSIS.pdsc` — the latest tagged stable release at
that point was
[v6.3.0](https://github.com/ARM-software/CMSIS_6/releases/tag/v6.3.0).

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
below), these four files are synthesised directly from Arm's own **CMSIS_6
core header files**, which are the authoritative, hand-maintained C
definitions of these exact registers:

| Header | Used for |
|---|---|
| [`core_cm0.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm0.h) | ARMv6-M baseline: NVIC/SCB register set, confirms no MPU |
| [`core_cm0plus.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm0plus.h) | ARMv6-M optional extras: conditional VTOR, MPU layout (no alias registers) |
| [`core_cm3.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm3.h) | ARMv7-M baseline: full NVIC (8 banks), full SCB fault-handling set, SCnSCB, MPU with 3 alias pairs |
| [`core_cm4.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm4.h), [`core_cm7.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm7.h) | Cross-check against `core_cm3.h` — confirmed identical at the SCB/NVIC/MPU level (M4/M7 only add a separate, deliberately-excluded FPU block, plus M7-specific cache extras also excluded) |
| [`core_cm23.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm23.h) | ARMv8-M Baseline (Cortex-M23): NVIC/SCB/MPU/SAU/DCB register set. CMSIS_6 dropped the old generic `core_armv8mbl.h` in favor of this per-core header (see [Updated for CMSIS_6](#updated-for-cmsis_6)) |
| [`core_cm33.h`](https://github.com/ARM-software/CMSIS_6/blob/main/CMSIS/Core/Include/core_cm33.h) | ARMv8-M Mainline (Cortex-M33): full NVIC (16 banks), full SCB fault-handling set plus Security Extension registers, SCnSCB, MPU with 3 alias pairs, SAU, DCB. Replaces the old generic `core_armv8mml.h`, same as above |

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

All four files carry NVIC, SCB, SysTick, and DCB, plus an architecturally
optional MPU; v7-M and v8-M Mainline add SCnSCB; the two v8-M files add the
optional SAU peripheral on top of that:

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
  `ID_PFR`/`ID_DFR`/`ID_AFR`/`ID_MMFR`/`ID_ISAR` ID registers, and `CPACR`.
  v8-M Baseline
  keeps v6-M's minimal set (plus `RETTOBASE` and the Security Extension
  fields — see below); v8-M Mainline matches v7-M's full set (`ID_ISAR`
  grows to 6 words) plus `NSACR`, `SFSR`, and `SFAR`. **v8-M-only fields**, RAZ/WI or
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
  Mainline's `RLAR` also has a `PXN` (Privileged eXecute Never) bit that
  Baseline's doesn't — not RAZ/WI there, genuinely absent from the register.
- **SAU** (`0xE000EDD0`, v8-M only, optional — present only with the Security
  Extension) — `CTRL`, `TYPE`, and, only if `__SAUREGION_PRESENT`, `RNR`/
  `RBAR`/`RLAR`. Identical on both v8-M profiles. `SFSR`/`SFAR` are
  physically the same registers CMSIS also exposes here, but are modeled
  once, under SCB, to avoid declaring one physical register in two
  peripherals (see the SCB entry above) — CMSIS_6 now says so explicitly,
  commenting SAU's own copies "deprecated: use SCB->SFSR"/"SCB->SFAR".
- **SCnSCB** (`0xE000E000`, v7-M and v8-M Mainline only) — `ICTR`
  (Interrupt Controller Type Register, read-only `INTLINESNUM` field) and
  `ACTLR` (Auxiliary Control Register, wholly implementation-defined —
  modeled with no fields, same treatment as the ID registers). v8-M
  Mainline adds `CPPWR` (Coprocessor Power Control — `SU10`/`SUS10`/`SU11`/
  `SUS11`, only meaningful with an FPU). Absent on v6-M and v8-M Baseline —
  neither `core_cm0.h`/`core_cm0plus.h` nor `core_cm23.h` define this
  struct at all. **This is a real, longstanding part of the architecture
  that earlier revisions of these files simply missed** — it already
  existed in CMSIS_5's `core_cm3.h`, not something CMSIS_6 added; it
  surfaced when cross-checking against CMSIS_6 for this update.
- **SysTick** (`0xE000E010`) — identical on all four files; `CTRL`/`LOAD`/
  `VAL`/`CALIB`.
- **DCB** (`0xE000EDF0`, named `CoreDebug` prior to CMSIS_6 — see [Updated
  for CMSIS_6](#updated-for-cmsis_6)) — `DHCSR`/`DCRSR`/`DCRDR`/`DEMCR` on
  every profile; v8-M adds `DAUTHCTRL`/`DSCSR` (Security Extension).
  `DCRSR.REGSEL` is 7 bits wide on every file here (CMSIS_6 widened it from
  5 bits, a change that turned out to apply to v7-M too, not just v8-M).
  This is fixed ADIv5/CoreSight debug architecture, not part of the
  M-profile ISA, so the core register layout is stable across the whole
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

## Updated for CMSIS_6

All four files were originally built from CMSIS_5. They were subsequently
re-verified against **CMSIS_6** (Arm's successor repository, which
reorganised the Core headers along the way — see the header table above) as
of `main` commit
[`26206e4`](https://github.com/ARM-software/CMSIS_6/commit/26206e47dcf0abfbdc64eb753a0b6334b24439f6)
(2026-09-11) — CMSIS-Core(M) v6.3.1-dev, one point release ahead of the
last tagged stable, v6.3.0. That pass surfaced both genuine renames/
widenings and one outright gap that CMSIS_5 wouldn't have revealed. In
order of how they were found:

- **`SCB.AIRCR.ENDIANESS` → `ENDIANNESS`** (all four files). CMSIS_6 fixed
  the historical misspelling; `SCB_AIRCR_ENDIANESS_Pos`/`_Msk` survive only
  as deprecated aliases. The field's bit position (15) and meaning are
  unchanged.
- **`CoreDebug` → `DCB`** (all four files). CMSIS_6 renamed the whole debug
  block from "CoreDebug"/`CoreDebug_Type` to "DCB"/`DCB_Type` (Debug Control
  Block) across the *entire* M-profile line, not just ARMv8-M as originally
  modeled here — `core_cm3.h`/`core_cm4.h`/`core_cm7.h` all made the same
  rename, keeping `CoreDebug`/`CoreDebug_Type` only as deprecated aliases.
  The v6-M/v7-M files' peripheral is renamed to match, even though Cortex-M0/
  M0+'s own header still doesn't define this struct at all (see the DCB
  entry above).
- **`DCB.DCRSR.REGSEL` widened from 5 to 7 bits** (all four files). Found
  already applied on ARMv8-M in the CMSIS_5-based version of this file;
  CMSIS_6 turns out to widen it identically on `core_cm3.h`/`core_cm4.h`/
  `core_cm7.h`, so the v6-M/v7-M files now match.
- **ARMv7-M's SCB ID registers renamed**: `PFR`/`DFR`/`ADR`/`MMFR`/`ISAR` →
  `ID_PFR`/`ID_DFR`/`ID_AFR`/`ID_MMFR`/`ID_ISAR` (`armv7m-system-peripherals.svd`
  only). This makes the v7-M file's naming match what CMSIS has always used
  for the identical registers on ARMv8-M — CMSIS_6 back-ported the naming
  to `core_cm3.h` itself. Offsets, widths, and array lengths are unchanged.
- **`MPU.RLAR.PXN` added** (`armv8m-mainline-system-peripherals.svd` only).
  A genuinely new field (bit 4, Privileged eXecute Never) in
  `core_cm33.h`'s `MPU_RLAR_PXN_Pos`/`_Msk` that doesn't exist in CMSIS_5's
  `core_armv8mml.h` at all. Confirmed Mainline-only: `core_cm23.h` (Baseline)
  has no such macro, so the Baseline file's `RLAR` is unchanged.
- **`SAU`'s `SFSR`/`SFAR` now explicitly marked deprecated** in CMSIS_6's
  own `SAU_Type` (commented "deprecated: use SCB->SFSR"/"SCB->SFAR").
  This validates a design choice already made in the CMSIS_5-based version
  of the Mainline file — SFSR/SFAR were already modeled once, under SCB,
  specifically because SAU_Type and SCB_Type expose the same physical
  registers at the same address; CMSIS_6 confirms that's the intended
  reading, so no register content changed, only the description text now
  cites CMSIS_6's own wording.
- **New peripheral: `SCnSCB`** (`armv7m-system-peripherals.svd` and
  `armv8m-mainline-system-peripherals.svd`) — `ICTR` and `ACTLR`, plus
  `CPPWR` on Mainline. This was **not** a CMSIS_6 addition — `SCnSCB_Type`
  with `ICTR`/`ACTLR` already existed in CMSIS_5's `core_cm3.h` — it's a
  real, longstanding part of the architecture that earlier revisions of
  these files simply missed, caught only because this update cross-checked
  every struct in the reference headers rather than only the ones already
  modeled. Confirmed absent on ARMv6-M and ARMv8-M Baseline: neither
  `core_cm0.h`/`core_cm0plus.h` nor `core_cm23.h` define `SCnSCB_Type` at
  all, so those two files are unaffected by this addition.

Registers and fields not listed above (NVIC, SysTick, the rest of SCB/MPU/
SAU/DCB, all CFSR/HFSR/DFSR/SHCSR bit layouts) were re-diffed against
CMSIS_6 and found unchanged from the CMSIS_5-based content already in these
files.

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
- ID registers on the v7-M/v8-M-Mainline files (`ID_PFR`, `ID_DFR`,
  `ID_AFR`, `ID_MMFR`, `ID_ISAR`) are included as address placeholders only,
  with no field breakdown — their content is silicon-specific and Arm
  doesn't publish a single canonical decode for them outside the full
  architecture reference manual. `SCnSCB.ACTLR` (v7-M and v8-M Mainline)
  gets the same treatment for the same reason, and its bit meanings vary
  even more widely across Arm's own reference cores than the ID registers
  do (see the SCnSCB entry above).
- The v7-M and both v8-M files' `<register>` elements put `<dim>`/
  `<dimIncrement>` *after* `<name>` (used for `ISER`/`ICER`/`ISPR`/`ICPR`/
  `IABR`/`ITNS`/`IPR` register arrays), and leave a couple of
  implementation-defined registers (`ID_DFR`, `ID_AFR`, `ACTLR`) as a
  self-closing `<fields/>`. Both are technically stricter-than-necessary
  violations of the official `CMSIS-SVD.xsd` schema (which wants the `dim*`
  group before `name`, and `<fields>` to contain at least one `<field>`
  when present) — `xmllint --schema CMSIS-SVD.xsd` flags them on all three
  multi-register files (only the dim-free v6-M file passes outright). Left
  as-is for consistency across all four rather than fixed piecemeal; worth
  a follow-up pass if strict schema validation ever matters for your
  toolchain.
- `NSACR.CP` (coprocessors 0-7, one bit each) and `SAU.RNR`/`RBAR`/`RLAR`'s
  "only present if implemented" conditions are modeled as always-present
  registers, per this repo's convention of keeping optional-per-device
  content addressable and documenting the condition in prose rather than
  omitting the register outright (same treatment MPU and VTOR already get
  in the v6-M/v7-M files).

## Licensing / provenance

Field names, offsets, and descriptions are transcribed from Arm's CMSIS_5
and CMSIS_6 repositories, both Apache License 2.0. These four files are
derivative of that header content, not of any single silicon vendor's
copyrighted SVD.

## Regenerating or extending

`build_generic.py` defines one Python builder function per peripheral
(`build_nvic_v6m`, `build_scb_v7m`, `build_scnscb_v7m`, `build_nvic_v8m`,
`build_scb_v8m_mainline`, `build_sau_v8m`, `build_scnscb_v8m_mainline`,
`build_dcb_v8m`, etc. — the v8-M ones take a `mainline: bool` where
baseline/mainline differ); `build_final.py` assembles them into the four
complete `<device>` documents, embedding the shared CMSIS license text and
each device's `<cpu>` block directly (no local CMSIS_5/CMSIS_6 checkout
needed). Run `python3 build_final.py` from this folder to regenerate all
four files in place; it also re-parses and prints a quick per-peripheral
register summary for each as a sanity check. To add a peripheral (FPU,
ITM/DWT, PMU, or ARMv8.1-M's PAC/BTI extras), add a new builder function
following the same pattern and call it from `build_final.py`.
