/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2023 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

// ----------------------------------------------------------------------------

void
_init (void);

void
_fini (void);

// ----------------------------------------------------------------------------

// Newlib calls `_fini()` in `__libc_fini_array`; `_init()` was
// added for completeness.

// libg.a(libc_a-fini.o): in function `__libc_fini_array':
// (.text.__libc_fini_array+0x1c): undefined reference to `_fini'

__attribute__ ((weak)) void
_init (void)
{
}

__attribute__ ((weak)) void
_fini (void)
{
}

// ----------------------------------------------------------------------------
