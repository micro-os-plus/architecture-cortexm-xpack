/*
 * DO NOT EDIT! Automatically generated from template file:
 * npm-packages-helper/templates/common/scripts/clang-format-liquid.mjs
 *
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

// ----------------------------------------------------------------------------

import { execFileSync } from 'child_process'
import { globSync } from 'glob'

const files = [
  ...globSync('src/**/*.{cpp,c,h}'),
  ...globSync('include/**/*.h'),
  ...globSync('tests/sources/**/*.{cpp,c,h}'),
  ...globSync('tests/includes/**/*.{cpp,c,h}'),
  ...globSync('tests/platforms/**/*.{cpp,c,h}'),
]

const args = ['--style=file:config/.clang-format', '-i', '--verbose', ...files]

console.log()
console.log(`[clang-format ${args.join(' ')}]`)

execFileSync('clang-format', args, { stdio: 'inherit' })

// ----------------------------------------------------------------------------
