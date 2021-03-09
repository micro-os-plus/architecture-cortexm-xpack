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

# https://cmake.org/cmake/help/v3.18/
# https://cmake.org/cmake/help/v3.18/manual/cmake-packages.7.html#package-configuration-file

if(micro-os-plus-architecture-cortexm-included)
  return()
endif()

set(micro-os-plus-architecture-cortexm-included TRUE)

message(STATUS "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION}...")

# -----------------------------------------------------------------------------
# Local dependencies.

include("${CMAKE_CURRENT_LIST_DIR}/../device/meta/config.cmake")
if(MICRO_OS_PLUS_INCLUDE_RTOS)
include("${CMAKE_CURRENT_LIST_DIR}/../rtos-port/meta/config.cmake")
endif()

# -----------------------------------------------------------------------------
# The current folder.

get_filename_component(xpack_current_folder ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

# -----------------------------------------------------------------------------

if(NOT TARGET micro-os-plus-architecture-cortexm-interface)

  add_library(micro-os-plus-architecture-cortexm-interface INTERFACE EXCLUDE_FROM_ALL)

  # ---------------------------------------------------------------------------
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

  # ---------------------------------------------------------------------------
  # Aliases.

  add_library(micro-os-plus::architecture-cortexm ALIAS micro-os-plus-architecture-cortexm-interface)
  # message(STATUS "=> micro-os-plus::architecture-cortexm")
  add_library(micro-os-plus::architecture ALIAS micro-os-plus-architecture-cortexm-interface)
  message(STATUS "=> micro-os-plus::architecture")

endif()

# -----------------------------------------------------------------------------
