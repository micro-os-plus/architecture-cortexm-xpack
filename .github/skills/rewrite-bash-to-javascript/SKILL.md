---
name: rewrite-bash-to-javascript
description: 'Rewrite a bash/shell script (.sh) in this project as an ES module Node.js script (.mjs), following the conventions already used by jsonc-format.mjs and xcdl-export.mjs. Use when the user asks to "rewrite", "convert", "port", or "translate" a bash script into JavaScript/Node.js, or to replace a shell script with a .mjs equivalent.'
argument-hint: 'Path to the .sh script to convert, e.g. scripts/cmake-format.sh'
---

# Rewrite Bash Scripts to JavaScript — µTest++ Testing Framework

## When to Use

- The user asks to rewrite, convert, port, or translate a `scripts/*.sh`
  file into JavaScript/Node.js.
- The user wants a new `.mjs` equivalent of an existing shell script,
  matching the style of [jsonc-format.mjs](../../../scripts/jsonc-format.mjs)
  and [xcdl-export.mjs](../../../scripts/xcdl-export.mjs).

## Step 1 — Conversion Conventions

Follow the style already established by
[jsonc-format.mjs](../../../scripts/jsonc-format.mjs) and
[xcdl-export.mjs](../../../scripts/xcdl-export.mjs):

- **Module system**: use ES module syntax (`import`/`export`) with a
  `.mjs` extension; `package.json` does not set `"type": "module"`, so the
  extension is what enables ESM. Node `>=20` is required (see `engines` in
  `package.json`).
- **Header comment**: keep a `/* ... */` block comment with the same
  copyright and MIT permission notice used in the `.sh` version, adapted to
  block-comment style. Only include the "DO NOT EDIT! Automatically
  generated from template file" line if the script is genuinely generated
  from a Liquid template (see Step 1); do not add it to hand-written
  scripts.
- **Script location**: replace `$0` / `dirname "$0"` with
  `fileURLToPath(import.meta.url)` and `path.dirname(...)`.
- **Template processing**: if the script renders Liquid templates, use the
  `liquidjs` package rather than hand-rolled string substitution.
- **File discovery**: replace `find` invocations with the `glob` package
  (`globSync`), using the same ignore patterns already used elsewhere in
  this project (`**/node_modules/**`, `**/xpacks/**`, `**/build/**`,
  `**/website/**`, as appropriate to the script).
- **Error handling**: replace bash's `set -o errexit`/`set -o nounset`
  guards with explicit checks that `console.error(...)` a clear message and
  `process.exit(1)`.
- **Permission preservation**: if the script edits files that may be
  read-only, preserve permissions with `statSync`/`chmodSync` around the
  read/write, as done in `jsonc-format.mjs`.
- **Output**: use `console.log`/`console.error` instead of `echo`.
- **Functions**: use arrow functions rather than the `function` keyword.
- **Synchronous APIs**: prefer synchronous calls (e.g. `readFileSync`,
  `execFileSync`) over their async/callback or Promise-based equivalents
  when both are available, matching the script's non-interactive, linear
  execution style.
- **Running external tools**: replace the bash `run_verbose` helper (which
  echoes the command and runs it) with Node's `child_process`
  (`execFileSync` or `spawnSync`), passing `stdio: 'inherit'` so the
  external tool's output streams through directly.
- **Dependencies**: add any new npm package (e.g. `glob`) to
  `devDependencies` in `package.json`, matching the versions already
  pinned for existing dependencies.
- **Wiring**: update the corresponding entry under `xpack.actions` in
  `package.json` to invoke the script directly with `node`, e.g.
  `"cmake-format": "node scripts/cmake-format.mjs"`, mirroring the
  existing `xcdl-export` action. Remove the now-unused `.sh` file only if
  the user confirms it is no longer needed elsewhere.

## Step 2 — Verify

- Run the new `.mjs` script directly (`node scripts/<name>.mjs ...`) and
  compare its effect against the original `.sh` script on the same input.
- Run the corresponding `xpm run <action>` command to confirm the
  `package.json` wiring works end-to-end.
