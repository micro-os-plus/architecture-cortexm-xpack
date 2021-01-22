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

# -----------------------------------------------------------------------------

function(target_sources_micro_os_plus_architecture_cortexm target)

  get_filename_component(xpack_root_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_sources(
    ${target}

    PRIVATE
      ${xpack_root_folder}/src/diag/trace-itm.cpp
      ${xpack_root_folder}/src/diag/trace-segger-rtt.cpp
      ${xpack_root_folder}/src/rtos/port/os-core.cpp
      ${xpack_root_folder}/src/startup/initialize-hardware-early.c
      ${xpack_root_folder}/src/startup/initialize-hardware.c
      ${xpack_root_folder}/src/startup/initialise-interrupts-stack.cpp
      ${xpack_root_folder}/src/exception-handlers.cpp
      ${xpack_root_folder}/src/terminate.cpp
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_include_directories_micro_os_plus_architecture_cortexm target)

  get_filename_component(xpack_root_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_include_directories(
    ${target}

    PUBLIC
      ${xpack_root_folder}/include
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_compile_definitions_micro_os_plus_architecture_cortexm target)

  # None

endfunction()

# -----------------------------------------------------------------------------
