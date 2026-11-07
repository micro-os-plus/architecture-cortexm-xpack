---
name: consistency-check
description: 'Check the consistency of the Architecture: Cortex-M project: file and folder names, whether names match their content, the build and test configurations (actions, configurations, platform folders, launchers), and whether the READMEs and the instruction files are up to date. Report the problems found, each with a permanent number for later reference. Use when the user asks for a "consistency check", to "check the consistency", or to find stale names, stale READMEs, or mismatched configurations.'
argument-hint: 'Optional: folder to check; defaults to the whole project'
---

# Consistency Check — Architecture: Cortex-M

## When to Use

- The user asks for a consistency check of the project, or of a part
  of it.
- After renaming files, folders, platforms, build configurations, or
  actions, to find the places left behind.
- Before a release, to find stale documentation.

This skill reports problems; it does not fix them, unless the user asks
for that separately (e.g. "fix #4").

## Scope

If the user specifies a folder, check only that folder. Otherwise check
the whole project, excluding `website/`, `node_modules/`, `xpacks/`, the
build folders (e.g. `build/`, `tests/build/`), and vendor-generated
folders (e.g. `stm32cubemx/`).

State the scope and the commit (`git rev-parse --short HEAD`) at the top
of the report.

## Generated Files

Many files carry a `DO NOT EDIT! Automatically generated from template
file` header, naming their template (e.g. in `build-helper` or
`npm-packages-helper`). For each problem in such a file, name the
template that must be fixed; the generated file is regenerated, never
edited in place. When a problem is found in a generated file, also
check the template, since other projects may share it.

## Checks

Read the files; do not rely on names alone. For every claim in a
README or an instruction file, verify it against the project (the file
exists, the command or action exists, the option is accepted, the
version is still used).

### 1. Functional consistency

Problems that make something fail, or would make it fail.

- **Build and test configurations** (e.g. `tests/package.json`): the
  actions, the build configurations, their `platformName` properties,
  the platform folders, and the lists of configurations used by the
  composite actions (e.g. `configs-ci`, `configs-boards`) must use the
  same names. Every platform must have the same set of per-platform
  actions as its siblings (e.g. `install-*`, `link-dependencies-*`,
  `build-*`, `test-*`, `clean-*`, `clear-*`); report the missing ones.
- **Build system integration**: the source files listed in
  `xcdl-package.jsonc`, `CMakeLists.txt`, and `meson.build` match the
  files in `src/`; the include folders exist.
- **Editor and debugger configurations** (e.g. `.vscode/launch.json`,
  `.vscode/tasks.json`): paths, build folder names, configuration names,
  and executable names match the current build layout.
- **CI workflows** (`.github/workflows/`): the actions they invoke
  exist.
- **Scripts**: the paths and actions referred to by the maintenance
  and test scripts exist.

### 2. Documentation

Stale or incorrect content in:

- the top `README.md` and the READMEs in all sub-folders (tests,
  platforms, devices, scripts, cmake);
- `CLAUDE.md`, `.github/copilot-instructions.md`, and the skills in
  `.github/skills/`.

Look for: commands and actions that do not exist; paths, files, and
folders that were renamed or removed; descriptions of tests, platforms,
or features that no longer match; obsolete versions and toolchains;
broken links; references to other projects (copy-paste leftovers);
headings or file names with the wrong case; typos.

### 3. Names

- File and folder names follow the project conventions (lower case,
  words separated by dashes; `-inlines.h` for the inline headers) and
  are consistent between siblings (e.g. platforms, devices).
- File names represent their content; the `@file` `@brief` of each
  source file matches its name and content.
- Include guards match the file names.
- Each header in `include/` has its matching `inlines/` and `src/`
  files where applicable, with the same base name.
- Stray, duplicated, or unused files (e.g. copies in the wrong folder,
  files for obsolete toolchains, generated files nothing refers to).
- Identifiers in the tests and sources that were copied without being
  renamed (e.g. the same suite variable reused in several files).
- Similar items placed inconsistently (e.g. a platform-level file kept
  in a device folder in one platform only).

### 4. Verified as consistent

Also record the areas checked and found consistent, so that the next
check knows what was covered.

## Numbering

Every problem gets a permanent number, `#1`, `#2`, and so on, counting
through the whole report (not restarting in each section), in the order
of the sections above.

- Each problem is a separate numbered item; do not group several
  independent problems under one number. Several occurrences of the same
  problem (e.g. the same typo in four generated READMEs) are one item.
- The numbers never change once assigned. When the report already
  exists, read it first: keep the numbers of the problems already
  listed, mark those that were fixed meanwhile with `(fixed)` (and a
  one-line note on how), keep those still open, and number the new
  problems after the highest existing number. Never reuse a number,
  even if its problem was withdrawn.
- If a problem turns out to be wrong, mark it `(withdrawn)`, with the
  reason; do not delete it.
- When the user later refers to a number (e.g. "fix #7"), act on that
  item and update its status in the report.

## Report

Write the report to `maintenance/docs/CONSISTENCY-CHECK.md`, in British
English and in a formal tone, with lines shorter than 80 characters:

```markdown
# Consistency check

Date: <date>, on branch `<branch>`, commit `<hash>`.

Scope: <scope and exclusions>.

Summary: <n> problems (<open> open, <fixed> fixed, <withdrawn>
withdrawn): <counts per section>.

## Note on generated files

<which problems are in generated files, and which templates to fix>

## 1. Functional consistency

- **#1 — <short title>.** <description, with file and line references,
  why it matters, and the template to fix, if generated>
- **#2 — <short title> (fixed).** <description>; fixed by <how>.

## 2. Documentation

### `<file>`

- **#3 — <short title>.** <description>

## 3. Names

- **#4 — <short title>.** <description>

## 4. Verified as consistent

- <area checked and found consistent>

## 5. Proposed next steps

- <grouped, actionable steps, referring to the problem numbers>
```

Each item states the problem directly and specifically: the file, the
line or the setting, what is wrong, and what is expected. When unsure
whether something is a problem or a deliberate choice (e.g. a platform
intentionally left out of a list), say so in the item, and ask the user
in the summary rather than guessing.

## Procedure

1. Determine the scope; read the existing report, if any.
2. List the files in scope, and read the configuration files, the
   READMEs, and the instruction files fully.
3. Go through the checks section by section, verifying each finding in
   the files (and in the templates, for generated files).
4. Number the findings as described above, and write the report.
5. Give the user a short summary: the counts, the most serious
   problems by number, and the questions that need a decision.
6. Do not fix anything unless asked.
