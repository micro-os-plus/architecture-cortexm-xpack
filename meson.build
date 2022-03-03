# -----------------------------------------------------------------------------
#
# This file is part of the µOS++ distribution.
#   (https://github.com/micro-os-plus/)
# Copyright (c) 2022 Liviu Ionescu
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
#
# If a copy of the license was not distributed with this file, it can
# be obtained from https://opensource.org/licenses/MIT/.
#
# -----------------------------------------------------------------------------

# This file is intended to be consumed by applications with:
#
# `subdir('xpacks/micro-os-plus-architecture-cortexm')`
#
# The result is a dependency that can be refered as:
#
# `dependencies: [micro_os_plus_architecture_dependency],`

# Note: the meson configuration is provided only as a proof of concept,
# for production it might need some refinements.

# -----------------------------------------------------------------------------

message('Processing xPack @micro-os-plus/architecture-cortexm...')

# https://mesonbuild.com/Reference-manual_functions.html#declare_dependency
micro_os_plus_architecture_dependency = declare_dependency(
  include_directories: include_directories(
    'include',
  )
)

message('+ -I include')
message('> micro_os_plus_architecture_dependency')

# -----------------------------------------------------------------------------
