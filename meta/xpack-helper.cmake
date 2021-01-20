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

message(STATUS "Including micro-os-plus-architecture-cortexm...")

function(target_sources_micro_os_plus_architecture_cortexm target)

  get_filename_component(PARENT_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_sources(
    ${target}

    PRIVATE
      ${PARENT_DIR}/src/diag/trace-itm.cpp
      ${PARENT_DIR}/src/diag/trace-segger-rtt.cpp
      ${PARENT_DIR}/src/rtos/port/os-core.cpp
      ${PARENT_DIR}/src/startup/initialize-hardware-early.c
      ${PARENT_DIR}/src/startup/initialize-hardware.c
      ${PARENT_DIR}/src/exception-handlers.cpp
      ${PARENT_DIR}/src/terminate.cpp
    )
endfunction()

function(target_include_directories_micro_os_plus_architecture_cortexm target)

  get_filename_component(PARENT_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_include_directories(
    ${target}

    PUBLIC
      ${PARENT_DIR}/include
  )
endfunction()

# -----------------------------------------------------------------------------
