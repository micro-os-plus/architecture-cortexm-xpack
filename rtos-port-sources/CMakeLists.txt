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

# https://cmake.org/cmake/help/v3.20/

# PRIVATE: build definitions, used internally
# INTERFACE: usage definitions, passed up to targets linking to it
# PUBLIC: both

# -----------------------------------------------------------------------------

add_library(micro-os-plus-architecture-cortexm-rtos-port-interface INTERFACE EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Target settings.

target_sources(
  micro-os-plus-architecture-cortexm-rtos-port-interface

  INTERFACE
    "src/port.cpp"
)

message(VERBOSE "+ rtos-port-sources/src/port.cpp")

target_include_directories(
  micro-os-plus-architecture-cortexm-rtos-port-interface

  INTERFACE
    include
)
message(VERBOSE "+ rtos-port-sources/include")

target_compile_definitions(
  micro-os-plus-architecture-cortexm-rtos-port-interface

  INTERFACE
    # ...
)

target_link_libraries(
  micro-os-plus-architecture-cortexm-rtos-port-interface

  INTERFACE
    micro-os-plus::rtos-port
    micro-os-plus::diag-trace
)

# ---------------------------------------------------------------------------
# Aliases.

add_library(micro-os-plus::rtos-port-cortexm ALIAS micro-os-plus-architecture-cortexm-rtos-port-interface)
message(VERBOSE "micro-os-plus::rtos-port-cortexm -> micro-os-plus-architecture-cortexm-rtos-port-interface")
add_library(micro-os-plus::rtos-port ALIAS micro-os-plus-architecture-cortexm-rtos-port-interface)
message(VERBOSE "micro-os-plus::rtos-port -> micro-os-plus-architecture-cortexm-rtos-port-interface")

# -----------------------------------------------------------------------------
