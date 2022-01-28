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

# =============================================================================

add_library(micro-os-plus-architecture-cortexm-device-interface INTERFACE EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
# Target settings.

target_sources(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    "sources-device/src/diag/trace-itm.cpp"
    "sources-device/src/diag/trace-segger-rtt.cpp"

    "sources-device/src/startup/initialize-hardware-early.c"
    "sources-device/src/startup/initialize-hardware.c"
    "sources-device/src/startup/initialize-interrupts-stack.cpp"
    "sources-device/src/startup/terminate.cpp"

    "sources-device/src/exception-handlers.cpp"
  )

message(VERBOSE "+ sources-device/src/diag/trace-itm.cpp")
message(VERBOSE "+ sources-device/src/diag/trace-segger-rtt.cpp")

message(VERBOSE "+ sources-device/src/startup/initialize-hardware-early.c")
message(VERBOSE "+ sources-device/src/startup/initialize-hardware.c")
message(VERBOSE "+ sources-device/src/startup/initialize-interrupts-stack.cpp")
message(VERBOSE "+ sources-device/src/startup/terminate.cpp")

message(VERBOSE "+ sources-device/src/exception-handlers.cpp")

target_include_directories(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    "sources-device/include"
)

message(VERBOSE "+ sources-device/include")

target_link_libraries(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    micro-os-plus::semihosting
)

# ---------------------------------------------------------------------------
# Aliases.

add_library(micro-os-plus::architecture-cortexm-device ALIAS micro-os-plus-architecture-cortexm-device-interface)
message(VERBOSE "> micro-os-plus::architecture-cortexm-device -> micro-os-plus-architecture-cortexm-device-interface")

# -----------------------------------------------------------------------------
