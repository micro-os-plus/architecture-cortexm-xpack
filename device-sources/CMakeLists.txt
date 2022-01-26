#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus)
# Copyright (c) 2021 Liviu Ionescu
#
# This Source Code Form is subject to the terms of the MIT License.
# If a copy of the license was not distributed with this file, it can
# be obtained from https://opensource.org/licenses/MIT/.
#
# -----------------------------------------------------------------------------

# https://cmake.org/cmake/help/v3.19/

# PRIVATE: build definitions, used internally
# INTERFACE: usage definitions, passed up to targets linking to it
# PUBLIC: both

# -----------------------------------------------------------------------------

add_library(micro-os-plus-architecture-cortexm-device-interface INTERFACE EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Target settings.

target_sources(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    "src/diag/trace-itm.cpp"
    "src/diag/trace-segger-rtt.cpp"

    "src/startup/initialize-hardware-early.c"
    "src/startup/initialize-hardware.c"
    "src/startup/initialize-interrupts-stack.cpp"
    "src/startup/terminate.cpp"

    "src/exception-handlers.cpp"
  )

message(VERBOSE "+ device-sources/src/diag/trace-itm.cpp")
message(VERBOSE "+ device-sources/src/diag/trace-segger-rtt.cpp")

message(VERBOSE "+ device-sources/src/startup/initialize-hardware-early.c")
message(VERBOSE "+ device-sources/src/startup/initialize-hardware.c")
message(VERBOSE "+ device-sources/src/startup/initialize-interrupts-stack.cpp")
message(VERBOSE "+ device-sources/src/startup/terminate.cpp")

message(VERBOSE "+ device-sources/src/exception-handlers.cpp")

target_include_directories(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    "include"
)

message(VERBOSE "+ device-sources/include")

target_link_libraries(
  micro-os-plus-architecture-cortexm-device-interface

  INTERFACE
    micro-os-plus::semihosting
)

# ---------------------------------------------------------------------------
# Aliases.

add_library(micro-os-plus::architecture-cortexm-device ALIAS micro-os-plus-architecture-cortexm-device-interface)
message(VERBOSE "micro-os-plus::architecture-cortexm-device -> micro-os-plus-architecture-cortexm-device-interface")

# -----------------------------------------------------------------------------
