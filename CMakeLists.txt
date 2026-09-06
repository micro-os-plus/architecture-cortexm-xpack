# -----------------------------------------------------------------------------
# DO NOT EDIT! Automatically generated from template file: xcdl-package.jsonc
#
# This file is part of the µOS++ project (https://micro-os-plus.github.io/).
# Copyright (c) 2021-2026 Liviu Ionescu. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose is hereby granted, under the terms of the MIT license.
#
# If a copy of the license was not distributed with this file, it can be
# obtained from https://opensource.org/licenses/mit.
#
# -----------------------------------------------------------------------------

# This file is intended to be used by applications via:
#
# `add_subdirectory("xpacks/@micro-os-plus/architecture-cortexm")`
#
# This creates libraries that can be linked to a target with:
#
# ~~~
# target_link_libraries(your-target PUBLIC micro-os-plus::architectures-cortexm-debugger-entry-point)
# target_link_libraries(your-target PUBLIC micro-os-plus::architectures-cortexm-reset-handler)
# target_link_libraries(your-target PUBLIC micro-os-plus::architectures-cortexm-exception-handlers)
# target_link_libraries(your-target PUBLIC micro-os-plus::architectures-cortexm)
# ~~~

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/
cmake_minimum_required (VERSION 3.20)
project (
  micro-os-plus-architecture-cortexm
  DESCRIPTION "A source code library with the µOS++ Arm Cortex-M architecture"
  LANGUAGES C CXX ASM
)

# To access the optional functions, include the
# `cmake/micro-os-plus-build-helper.cmake` file from the
# `@micro-os-plus/build-helper` package.

if (COMMAND xpack_get_package_name_and_version)
  xpack_get_package_name_and_version (
    "${CMAKE_CURRENT_SOURCE_DIR}/package.json"
  )
  message (VERBOSE
           "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION}..."
  )
endif ()

# -----------------------------------------------------------------------------

# Interface libraries are the main targets, which consumers (like embedded
# applications) link against. It defines the public API and properties of the
# library; sources are propagated to and compiled by each consumer.

# They are aliased using namespaced syntax (Project::Target). This is the
# canonical pattern for consumption via target_link_libraries(), whether the
# dependency is added via add_subdirectory(), FetchContent, or an installed
# package via find_package(). Using "::" lets CMake detect an invalid or
# misspelled target name at configure time, rather than silently passing it
# through to the linker as a plain string or flag. Note: alias targets are
# read-only; properties must be set on the underlying target, not on the alias
# itself.

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-architectures-cortexm-debugger-entry-point-interface INTERFACE
  EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-architectures-cortexm-debugger-entry-point-interface
  INTERFACE "src/debugger-entry-point.S"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (
    micro-os-plus-architectures-cortexm-debugger-entry-point-interface
  )
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::architectures-cortexm-debugger-entry-point ALIAS
  micro-os-plus-architectures-cortexm-debugger-entry-point-interface
)
message (
  VERBOSE
  "> micro-os-plus::architectures-cortexm-debugger-entry-point -> micro-os-plus-architectures-cortexm-debugger-entry-point-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-architectures-cortexm-reset-handler-interface INTERFACE
  EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-architectures-cortexm-reset-handler-interface
  INTERFACE "src/reset-handler.c"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (
    micro-os-plus-architectures-cortexm-reset-handler-interface
  )
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::architectures-cortexm-reset-handler ALIAS
  micro-os-plus-architectures-cortexm-reset-handler-interface
)
message (
  VERBOSE
  "> micro-os-plus::architectures-cortexm-reset-handler -> micro-os-plus-architectures-cortexm-reset-handler-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-architectures-cortexm-exception-handlers-interface INTERFACE
  EXCLUDE_FROM_ALL
)

target_sources (
  micro-os-plus-architectures-cortexm-exception-handlers-interface
  INTERFACE "src/exception-handlers.cpp"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (
    micro-os-plus-architectures-cortexm-exception-handlers-interface
  )
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::architectures-cortexm-exception-handlers ALIAS
  micro-os-plus-architectures-cortexm-exception-handlers-interface
)
message (
  VERBOSE
  "> micro-os-plus::architectures-cortexm-exception-handlers -> micro-os-plus-architectures-cortexm-exception-handlers-interface"
)

# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.20/command/add_library.html?highlight=interface#interface-libraries
# Visibility keywords: PRIVATE applies to the target itself; INTERFACE passes
# properties up to targets that link against it; PUBLIC does both.
add_library (
  micro-os-plus-architectures-cortexm-interface INTERFACE EXCLUDE_FROM_ALL
)

target_include_directories (
  micro-os-plus-architectures-cortexm-interface INTERFACE "include"
)

target_sources (
  micro-os-plus-architectures-cortexm-interface
  INTERFACE "src/_init_fini.c" "src/semihosting.cpp" "src/show-cpuid.cpp"
)

target_link_libraries (
  micro-os-plus-architectures-cortexm-interface
  INTERFACE "micro-os-plus::architectures-cortexm-debugger-entry-point"
            "micro-os-plus::architectures-cortexm-reset-handler"
            "micro-os-plus::architectures-cortexm-exception-handlers"
)

if (COMMAND xpack_display_target_lists)
  xpack_display_target_lists (micro-os-plus-architectures-cortexm-interface)
endif ()

# -----------------------------------------------------------------------------
# Alias.

# https://cmake.org/cmake/help/v3.20/command/add_library.html#alias-libraries
add_library (
  micro-os-plus::architectures-cortexm ALIAS
  micro-os-plus-architectures-cortexm-interface
)
message (
  VERBOSE
  "> micro-os-plus::architectures-cortexm -> micro-os-plus-architectures-cortexm-interface"
)

add_library (
  micro-os-plus::architecture ALIAS
  micro-os-plus-architectures-cortexm-interface
)
message (
  VERBOSE
  "> micro-os-plus::architecture -> micro-os-plus-architectures-cortexm-interface"
)

# -----------------------------------------------------------------------------
