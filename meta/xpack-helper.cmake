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

if(micro-os-plus-architecture-cortexm-included)
  return()
endif()

set(micro-os-plus-architecture-cortexm-included TRUE)

message(STATUS "Including micro-os-plus-architecture-cortexm...")

# -----------------------------------------------------------------------------

# Not used.
if(false)
function(_target_sources_micro_os_plus_architecture_cortexm target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_sources(
    ${target}

    PRIVATE
      # None so far, all are device dependent.
  )

endfunction()
endif()

# -----------------------------------------------------------------------------

function(target_include_directories_micro_os_plus_architecture_cortexm target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_include_directories(
    ${target}

    INTERFACE
      ${xpack_current_folder}/include
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_compile_definitions_micro_os_plus_architecture_cortexm target)

  # None

endfunction()

# =============================================================================

function(target_sources_micro_os_plus_architecture_cortexm_device target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_sources(
    ${target}

    PRIVATE
      ${xpack_current_folder}/device/src/diag/trace-itm.cpp
      ${xpack_current_folder}/device/src/diag/trace-segger-rtt.cpp
      ${xpack_current_folder}/device/src/rtos/port/os-core.cpp
      ${xpack_current_folder}/device/src/startup/initialize-hardware-early.c
      ${xpack_current_folder}/device/src/startup/initialize-hardware.c
      ${xpack_current_folder}/device/src/startup/initialise-interrupts-stack.cpp
      ${xpack_current_folder}/device/src/exception-handlers.cpp
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_include_directories_micro_os_plus_architecture_cortexm_device target)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  target_include_directories(
    ${target}

    PUBLIC
      ${xpack_current_folder}/device/include
  )

endfunction()

# -----------------------------------------------------------------------------

function(target_compile_definitions_micro_os_plus_architecture_cortexm_device target)

  # None

endfunction()

# =============================================================================

function(add_libraries_micro_os_plus_architecture_cortexm)

  # ---------------------------------------------------------------------------

  if(NOT TARGET micro-os-plus-architecture-cortexm-interface)

    add_library(micro-os-plus-architecture-cortexm-interface INTERFACE EXCLUDE_FROM_ALL)

    # target_sources_micro_os_plus_architecture_cortexm(micro-os-plus-architecture-cortexm-object)
    target_include_directories_micro_os_plus_architecture_cortexm(micro-os-plus-architecture-cortexm-interface)
    target_compile_definitions_micro_os_plus_architecture_cortexm(micro-os-plus-architecture-cortexm-interface)

    add_library(micro-os-plus::architecture-cortexm ALIAS micro-os-plus-architecture-cortexm-interface)
    add_library(micro-os-plus::architecture ALIAS micro-os-plus-architecture-cortexm-interface)
    message(STATUS "micro-os-plus::architecture")

    target_link_libraries(
      micro-os-plus-architecture-cortexm-interface

      INTERFACE
        micro-os-plus::common
    )

  endif()

  # ---------------------------------------------------------------------------

endfunction()

# -----------------------------------------------------------------------------
