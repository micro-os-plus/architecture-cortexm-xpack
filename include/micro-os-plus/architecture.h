/*
 * This file is part of the µOS++ project (https://micro-os-plus.github.io/).
 * Copyright (c) 2017-2026 Liviu Ionescu. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can be
 * obtained from https://opensource.org/licenses/mit.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ARCHITECTURE_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ARCHITECTURE_H_

// ----------------------------------------------------------------------------

#if defined(__cplusplus)
#if !(__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
#error "C++20 or higher is required"
#endif
#endif // defined(__cplusplus)

#if __has_include(<micro-os-plus/project-config.h>)
#include <micro-os-plus/project-config.h>
#elif __has_include(<micro-os-plus/config.h>)
#pragma message \
    "micro-os-plus/config.h is deprecated, rename to micro-os-plus/project-config.h and include it instead of micro-os-plus/config.h"
#include <micro-os-plus/config.h>
#endif // __has_include(<micro-os-plus/project-config.h>)

#if __has_include(<micro-os-plus/architecture-defines.h>)
#include <micro-os-plus/architecture-defines.h>
#endif // __has_include(<micro-os-plus/architecture-defines.h>)

#include <micro-os-plus/architecture-cortexm/defines.h>

#include <micro-os-plus/architecture-cortexm/types.h>
// #include <micro-os-plus/architecture-cortexm/declarations.h>

#include <micro-os-plus/architecture-cortexm/instructions.h>

#include <micro-os-plus/architecture-cortexm/registers.h>

#include <micro-os-plus/architecture-cortexm/inlines/semihosting-inlines.h>

// ----------------------------------------------------------------------------

#endif // MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ARCHITECTURE_H_

// ----------------------------------------------------------------------------
