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

# -----------------------------------------------------------------------------
## Preamble ##

# https://cmake.org/cmake/help/v3.20/
cmake_minimum_required(VERSION 3.20)

project(
  micro-os-plus-architecture-cortexm
  DESCRIPTION "µOS++ Arm Cortex-M architecture"
)

# PRIVATE: build definitions, used internally
# INTERFACE: usage definitions, passed up to targets linking to it
# PUBLIC: both

if(COMMAND xpack_get_package_name_and_version)
  xpack_get_package_name_and_version("${CMAKE_CURRENT_SOURCE_DIR}/package.json")
  message(VERBOSE "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION}...")
endif()

# -----------------------------------------------------------------------------
## The project library definitions ##

add_library(micro-os-plus-architecture-cortexm-interface INTERFACE EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
# Target settings.

target_sources(
  micro-os-plus-architecture-cortexm-interface

  PRIVATE
    # None so far, all are device dependent.
)

target_include_directories(
  micro-os-plus-architecture-cortexm-interface

  INTERFACE
    include
)

message(VERBOSE "+ include")

# -----------------------------------------------------------------------------
# Aliases.

add_library(micro-os-plus::architecture-cortexm ALIAS micro-os-plus-architecture-cortexm-interface)
message(VERBOSE "> micro-os-plus::architecture-cortexm -> micro-os-plus-architecture-cortexm-interface")
add_library(micro-os-plus::architecture ALIAS micro-os-plus-architecture-cortexm-interface)
message(VERBOSE "> micro-os-plus::architecture -> micro-os-plus-architecture-cortexm-interface")

# -----------------------------------------------------------------------------
