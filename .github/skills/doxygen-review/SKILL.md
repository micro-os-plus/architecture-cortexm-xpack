---
name: doxygen-review
description: 'Verify and fix the Doxygen documentation comments of the C++ sources in the Architecture: Cortex-M project, checking both completeness (@brief/@details on every entity) and accuracy (the text matches what the code actually does). Use when the user asks to "verify", "review", "check", or "fix" the Doxygen comments or the API documentation, or to bring the documentation in line with the project rules.'
argument-hint: 'Optional: file or folder to review; defaults to include/ and src/'
---

# Doxygen Review — Architecture: Cortex-M

## When to Use

- The user asks to verify, review, or fix the Doxygen comments, or the
  API documentation in general.
- After significant code changes, to bring the documentation back in line
  with the code.

This skill fixes documentation only. Code defects found while reading the
code are reported to the user, not fixed, unless the user asks for that
separately.

## Scope

If the user specifies a path, review only that path. Otherwise review
`include/` (including `inlines/`) and `src/`. Generated files (those
with a `DO NOT EDIT` header) are fixed in their templates, not in place.

## The Rules

The rules are those in `.github/copilot-instructions.md` (Documentation,
Language and Tone sections); in short:

- Every class, method, data member, type alias, macro, parameter, and
  return value is documented, including `private` and `protected`
  members, deleted and defaulted special members, and deduction guides.
- Every entity has both a `@brief` and a `@details`. Declarations carry
  the `@brief` (plus `@param`, `@tparam`, `@return`); when the
  definition is in a separate file (`inlines/` or `src/`), its comment
  carries the `@details`. Doxygen merges the two.
- `@details` expands on `@brief`, without repeating it.
- Functions without parameters use `@par Parameters` / `None.`;
  functions returning `void` use `@par Returns` / `Nothing.`;
  `[[noreturn]]` functions use `Does not return.`
- British English, formal tone, no contractions, the Oxford comma,
  "folder" rather than "directory", and "program" (not "programme")
  for computer programs.
- Lines shorter than 80 characters; no tab characters; comment lines
  aligned with their `/**`.

## Step 1 — Automatic Checks

Run all three checks before reading anything, to get a baseline, and run
them again at the end. Use a scratch folder outside the project for the
outputs (shown below as `<scratch>`).

### 1.1 Doxygen warnings, with strict settings

Create `<scratch>/check.doxyfile`, overlaying the project configuration
(run Doxygen from `website/doxygen`, so that the relative paths in the
project configuration resolve):

```text
@INCLUDE = ./config.doxyfile
OUTPUT_DIRECTORY = <scratch>/out
GENERATE_HTML = NO
GENERATE_XML = YES
XML_OUTPUT = xml
EXTRACT_ALL = NO
WARN_IF_UNDOCUMENTED = YES
WARN_IF_DOC_ERROR = YES
WARN_IF_INCOMPLETE_DOC = YES
WARN_NO_PARAMDOC = YES
WARN_AS_ERROR = NO
WARN_LOGFILE = <scratch>/warnings.txt
QUIET = YES
```

```sh
cd website/doxygen && doxygen <scratch>/check.doxyfile
```

`EXTRACT_ALL = NO` is essential; with the project default (`YES`)
Doxygen does not report undocumented entities.

### 1.2 Merged completeness

Doxygen does not check for missing `@details`, and a per-block check
over-reports, since definitions legitimately have no `@brief`. Check the
merged result in the XML output instead:

```sh
node .github/skills/doxygen-review/scripts/check-doxygen-merged.mjs <scratch>/out/xml
```

It lists every entity in `include/` and `src/` without a brief or
without a details paragraph (`@param`, `@return`, `@par`, `@since`,
`@deprecated` do not count as details; `@note` and `@warning` do).

### 1.3 Style rules

```sh
node .github/skills/doxygen-review/scripts/check-doxygen-style.mjs [path...]
```

It reports long lines, tabs, empty or misaligned comment lines, American
spellings, contractions, "directory"/"programme", `@details` repeating
the `@brief`, and declaration comments in the public headers without a
`@brief`.

## Step 2 — Read and Verify Against the Code

The automatic checks find missing text, not wrong text; most real
problems are inaccurate descriptions. For each header, read it together
with its `inlines/<name>-inlines.h` and `src/<name>.cpp`, and the code
they describe, and check every comment against the implementation.

Known sources of inaccuracy, to be checked explicitly:

- **Stale design descriptions.** Comments written for an earlier design:
  removed variables or helpers still mentioned, a class template that is
  now a plain class, a namespace that moved (e.g. `detail::op` versus
  `type_traits::op`), a responsibility that moved to another class.
- **Behaviour claims.** Words such as "immediately", "default",
  "automatically", "always", "never", "owns", "does not own",
  "allocation-free", "safe", "zero-initialises", "clears", "flushes".
  Confirm each one in the code; follow the call chain if needed (e.g.
  whether a method runs something now or only registers it for later).
- **Copied documentation.** Sibling classes (e.g. two reporters) often
  share copied comments that describe only one of them (symbols, colours,
  output format).
- **Shared enumerations and options.** Describe the behaviour of every
  implementation (e.g. what each verbosity level displays in every
  reporter), not of one.
- **Platform statements.** Claims about compilers, libraries, and
  architectures (e.g. type sizes on Arm, AArch64, RISC-V, x86-64).
- **Examples.** Every `@code` example must compile and be correct (e.g.
  a numeric literal operator template does not accept a character
  literal).
- **Return values and parameters.** The described type and meaning must
  match the signature (e.g. a function returning a comparator object does
  not return "an output stream").
- **Numbers and complexity.** Re-derive any stated formula, scale, or
  complexity from the code.
- **Undefined behaviour.** Never document an undefined behaviour as a
  feature (e.g. modifying a `const` object through `const_cast`); add a
  `@warning` instead.
- **Padding.** Remove filler phrases that add no information (e.g.
  "across all test cases and folders", "enhancing the professionalism
  of the reports").

When a claim cannot be verified, ask the user rather than guess.

## Step 3 — Fix

- Fix the comments only; do not change the code. Keep a list of the code
  defects noticed (e.g. declared but undefined functions, stale forward
  declarations) to report at the end.
- Match the existing style: the same block layout, the same `@par`
  conventions, and the indentation of the surrounding code (the comment
  lines start with a space and `*`, aligned under the `/**`).
- For mechanical edits over many blocks, match the blocks exactly,
  including the leading whitespace; check that every intended block was
  matched before writing, and never rely on output that has the
  whitespace collapsed (e.g. through `awk`) to compose the replacement.
- Keep the `@details` meaningful and short for trivial members (e.g. why
  a special member is deleted, what sets a data member); do not pad.

## Step 4 — Verify

1. Re-run the three checks of Step 1; Doxygen and the merged check must
   report no problems. Any remaining style problems must be justified
   (e.g. long `#endif` comments that repeat a long condition).
2. Check that `clang-format` would not change any of the edited files:

   ```sh
   xpacks/.bin/clang-format --style=file:maintenance/config/.clang-format <file> | diff <file> -
   ```

3. Build and run the tests, at least with the system compiler, with
   clang (whose `-Wdocumentation` checks the comments), and with one
   cross target (see the Testing section of
   `.github/copilot-instructions.md`).

## Step 5 — Report

Summarise for the user:

- the before/after counts of the automatic checks;
- the inaccurate descriptions that were corrected, grouped by kind, with
  the files involved;
- the code defects noticed but not fixed;
- anything left unchanged, and why (e.g. generated documentation under
  `website/` that is updated only when the documentation is rebuilt).
