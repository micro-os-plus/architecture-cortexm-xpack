#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus)
# Copyright (c) 2021 Liviu Ionescu
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
#
# -----------------------------------------------------------------------------

# This file is intended to be consumed by applications with:
#
# `add_subdirectory("xpacks/micro-os-plus-architecture-cortexm")`
#
# The result several libraries that can be added to the linker with:
#
# `target_link_libraries(your-target PUBLIC micro-os-plus::architecture-cortexm)`
#
# To avoid poluting this file with test related details, there is a separate
# CMake configuration in the `tests` folder.

# -----------------------------------------------------------------------------
## Preamble ##

# https://cmake.org/cmake/help/v3.19/
cmake_minimum_required(VERSION 3.19)

project(
  micro-os-plus-architecture-cortexm
  DESCRIPTION "µOS++ Arm Cortex-M architecture"
)

# Must be called from the project root folder.
enable_testing()

# -----------------------------------------------------------------------------
## Project wide setup ##

enable_language(C)
enable_language(CXX)
enable_language(ASM)

# Specify the C/C++ standards.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------

if ("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")

  # Tests, only when not part of a parent application
  add_subdirectory("tests")

else()

  ## Main targets built by this project ##
  add_subdirectory("sources")

  add_subdirectory("device-sources")

  if(MICRO_OS_PLUS_INCLUDE_RTOS)
    add_subdirectory("rtos-port-sources")
  endif()

endif()

# -----------------------------------------------------------------------------
