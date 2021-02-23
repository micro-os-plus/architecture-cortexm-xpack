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

include("${CMAKE_CURRENT_LIST_DIR}/../device/meta/xpack-helper.cmake")

# -----------------------------------------------------------------------------

function(add_libraries_micro_os_plus_architecture_cortexm)

  get_filename_component(xpack_current_folder ${CMAKE_CURRENT_FUNCTION_LIST_DIR} DIRECTORY)

  # ---------------------------------------------------------------------------

  if(NOT TARGET micro-os-plus-architecture-cortexm-interface)

    add_library(micro-os-plus-architecture-cortexm-interface INTERFACE EXCLUDE_FROM_ALL)

    # -------------------------------------------------------------------------
    # Target settings.

    target_sources(
      micro-os-plus-architecture-cortexm-interface
  
      PRIVATE
        # None so far, all are device dependent.
    )

    target_include_directories(
      micro-os-plus-architecture-cortexm-interface
  
      INTERFACE
        ${xpack_current_folder}/include
    )

    # -------------------------------------------------------------------------
    # Aliases

    add_library(micro-os-plus::architecture-cortexm ALIAS micro-os-plus-architecture-cortexm-interface)
    message(STATUS "micro-os-plus::architecture-cortexm")
    add_library(micro-os-plus::architecture ALIAS micro-os-plus-architecture-cortexm-interface)
    message(STATUS "micro-os-plus::architecture")

  endif()

  # ---------------------------------------------------------------------------

  add_libraries_micro_os_plus_architecture_cortexm_device()

  # ---------------------------------------------------------------------------

endfunction()

# -----------------------------------------------------------------------------
