---
name: code-review
description: 'Perform a thorough, uncompromising code review of C++ sources, headers, and documentation in the µOS++ Intrusive Lists project. Use when the user asks for a "code review", to "review the code", to "review changes", or to check a file, folder, or pull request for style, naming, documentation, and correctness issues.'
argument-hint: 'Optional: path or folder to review; defaults to the whole repository'
---

# Code Review — Architecture: Cortex-M

## When to Use

- The user asks for a code review of a file, folder, diff, or the whole
  repository.
- The user asks to check adherence to the project's coding standards before
  a commit or release.

## Review Scope

If the user specifies a path, review only that path. Otherwise review
`include/`, `src/`, and `tests/` (excluding `tests/build/`).

## Checklist

Review every item below. Do not soften findings; state flaws directly and
specifically, with file and line references.

### Naming and style

- C/C++ identifiers use `snake_case` (types, functions, variables, members).
- File names are lower-case words separated by dashes (e.g.
  `doubly-list-links.h`).
- Formatting matches `.clang-format` (run `xpm run clang-format` mentally;
  flag anything that looks inconsistent with it).
- No lines exceed 80 characters, including comments, except possibly the
  file path in a `DO NOT EDIT` generated-file comment, link URLs.

### Folder structure

- Declarations live in `include/`, out-of-line definitions in `src/`,
  inline (header-only) definitions in `inlines/`.
- Usually there is only one public header (like
  `include/micro-os-plus/utils/lists.h`);
  internal headers are located in a subfolder named after the public header
  (e.g. `include/micro-os-plus/utils/lists/`).
- New source files are registered in the top-level `CMakeLists.txt` and
  `meson.build` (generated from xcdl-package.jsonc).

### Documentation (Doxygen)

- Every class, method, property, parameter, and return value has a
  Doxygen comment, including `private` and `protected` members.
- Each declaration/definition has both `@brief` and `@details`.
- `@details` expands on `@brief` rather than repeating it; it is located
  immediately after `@brief` in the comment block, or, if the declaration
  and definition are in separate files, in the comment immediately before
  the definition.
- `@details` should provide a more in-depth explanation, including any relevant
  information about the implementation, usage, or edge cases.
- Documentation uses British English spelling (e.g. "behaviour",
  "colour", "initialise") and formal, contraction-free wording.
- If the code already includes documentation, review and possibly improve it.
- If Doxygen can be run, check its warning log (undocumented parameters,
  `@ref`/`@tparam` targets that do not resolve, mismatched signatures)
  rather than relying solely on a manual read.

### Licensing and metadata

- Every source and header file carries the identical MIT licence header
  block, with a plausible copyright year range for the project.
- If a change is significant enough to affect behaviour, check whether
  `CHANGELOG.md` and the package version should be updated.

### Conditional compilation

- Header guards are the only construct permitted to use `#ifndef`.
- All other conditional compilation uses `#if defined(...)` or
  `#if !defined(...)`, never bare `#ifdef` / `#ifndef`.
- Every `#endif` is followed by a C++ comment (`// ...`) repeating the
  exact expression from its paired `#if`/`#if defined`/`#if !defined`,
  so nested and distant guards remain traceable.
- The above applies to pragmas where `#if defined(__GNUC__)`, 
  `#if defined(__clang__)` or similar are used.

### Modern C++

- The codebase targets C++20; flag code written in an older style where
  a modern equivalent is clearly preferable.
- Prefer `concepts` and `requires` clauses over SFINAE and
  `std::enable_if`.
- Prefer `constexpr`/`consteval` over macros and runtime checks where the
  value is known at compile time.
- Prefer range-based `for`, structured bindings, `auto`, and `<=>`
  (three-way comparison) where they simplify the code without harming
  readability.
- Prefer `[[nodiscard]]`, `[[maybe_unused]]`, and other standard
  attributes over compiler-specific equivalents.
- Flag unnecessary use of raw pointers, casts, or macros where a
  standard C++20 facility would be clearer and safer, while respecting
  the embedded/bare-metal constraints of the project (no exceptions,
  no heap allocation, no RTTI, unless already used elsewhere).
- `noexcept` is specified explicitly wherever it is warranted (e.g.
  move constructors/assignment, destructors, and functions that
  provably cannot throw); flag missing or incorrect `noexcept`.

### Correctness and maintainability

- Look for undefined behaviour, lifetime/ownership issues, missing
  `const`-correctness, and incorrect intrusive-list pointer handling
  (links, insertion/removal, iterator invalidation).
- Check for potential ABI or layout implications of changes to link
  structures.
- Flag weak assumptions, missing edge-case handling, and any deviation
  from existing patterns in the codebase without justification.
- Check that thread-safety/reentrancy expectations are documented where
  relevant (these lists provide no internal locking; any method that could
  be called from an ISR or from multiple threads without external
  synchronisation should say so explicitly).
- Check API symmetry between related class templates (e.g. `doubly_list`
  versus `intrusive_list`); if one offers a convenience method the other
  lacks (such as `unlink_head`/`unlink_tail`), confirm this is a deliberate
  design decision rather than an oversight, and note the justification.

### Toolchain verification

- Where feasible, prefer verifying claims by actually building rather than
  reading only: run the project with `-Wall -Wextra -Wpedantic` (or the
  `xpm run test*` variants) and report any warnings, rather than only
  flagging style issues that look suspicious.
- For pointer-arithmetic or type-punning idioms (e.g. `reinterpret_cast`
  member-offset tricks, null-pointer-based `offsetof` substitutes), check
  whether a sanitizer build (`-fsanitize=undefined`/`-fsanitize=address`)
  is available and flag any UB it reports.

### Tests

- Changes to behaviour are covered by tests in `tests/`.
- Note if `xpm run test -C tests` (and the clang/qemu variants mentioned
  in the top-level instructions) should be re-run.

## Procedure

1. Identify the review scope (explicit path, or the default scope above).
2. Read the relevant files fully; do not review based on partial context.
3. Go through the checklist section by section, collecting concrete
   findings (file, line, description, why it matters).
4. Explicitly note anything left out of the review because you were not
   certain enough to include it.
5. Write the results to `CODE-REVIEW.md` at the root of the project,
   with:
   - a summary of findings, and
   - specific, actionable recommendations, grouped by checklist section.
6. Index open issues numerically, to be refered when asking to close them.
7. Do not fix the issues automatically unless the user asks for that
   separately.
