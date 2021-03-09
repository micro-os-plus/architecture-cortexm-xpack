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

if(micro-os-plus-architecture-cortexm-rtos-port-included)
  return()
endif()

set(micro-os-plus-architecture-cortexm-rtos-port-included TRUE)

message(STATUS "Processing xPack ${PACKAGE_JSON_NAME}@${PACKAGE_JSON_VERSION} rtos-port...")

# -----------------------------------------------------------------------------
# Dependencies.

find_package(micro-os-plus-rtos REQUIRED)
find_package(micro-os-plus-diag-trace REQUIRED)

# -----------------------------------------------------------------------------
# The current folder.

get_filename_component(xpack_current_folder ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

# -----------------------------------------------------------------------------

if(NOT TARGET micro-os-plus-architecture-cortexm-rtos-port-interface)

  add_library(micro-os-plus-architecture-cortexm-rtos-port-interface INTERFACE EXCLUDE_FROM_ALL)

  # ---------------------------------------------------------------------------
  # Target settings.

  xpack_glob_recurse_cxx(source_files "${xpack_current_folder}/src")
  xpack_display_relative_paths("${source_files}" "${xpack_current_folder}")

  target_sources(
    micro-os-plus-architecture-cortexm-rtos-port-interface

    INTERFACE
      ${source_files}
  )

  target_include_directories(
    micro-os-plus-architecture-cortexm-rtos-port-interface

    INTERFACE
      ${xpack_current_folder}/include
  )

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
  # message(STATUS "=> micro-os-plus::rtos-port-cortexm")
  add_library(micro-os-plus::rtos-port ALIAS micro-os-plus-architecture-cortexm-rtos-port-interface)
  message(STATUS "=> micro-os-plus::rtos-port")

endif()

# -----------------------------------------------------------------------------
