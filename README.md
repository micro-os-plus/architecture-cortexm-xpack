[![GitHub package.json version](https://img.shields.io/github/package-json/v/micro-os-plus/architecture-cortexm-xpack)](https://github.com/micro-os-plus/architecture-cortexm-xpack/blob/xpack/package.json)
[![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/micro-os-plus/architecture-cortexm-xpack)](https://github.com/micro-os-plus/architecture-cortexm-xpack/tags/)
[![npm (scoped)](https://img.shields.io/npm/v/@micro-os-plus/architecture-cortexm.svg?color=blue)](https://www.npmjs.com/package/@micro-os-plus/architecture-cortexm/)
[![license](https://img.shields.io/github/license/micro-os-plus/architecture-cortexm-xpack)](https://github.com/micro-os-plus/architecture-cortexm-xpack/blob/xpack/LICENSE)
[![CI on Push](https://github.com/micro-os-plus/architecture-cortexm-xpack/actions/workflows/ci.yml/badge.svg)](https://github.com/micro-os-plus/architecture-cortexm-xpack/actions/workflows/ci.yml)

# A source code library with the µOS++ architecture definitions for Arm Cortex-M

This project provides the **architecture-cortexm** source library as an `xpm`
dependency and includes architecture definitions for embedded projects
running on Arm Cortex-M devices.

Cortex-A and Cortex-R devices are **not** supported by this package; they
are supported by the separate
[architecture-aarch32](https://github.com/micro-os-plus/architecture-aarch32-xpack)
package.

The project is hosted on GitHub as
[micro-os-plus/architecture-cortexm-xpack](https://github.com/micro-os-plus/architecture-cortexm-xpack).

This page is addressed to developers who plan to include this source
library into their own projects.

For maintainer information, please see the
[README-MAINTAINER](README-MAINTAINER.md) file.

## Install

As a source library xpm package, the easiest way to add it to a project is via
**xpm**, but it can also be used as any Git project, for example as a submodule.

### Prerequisites

A recent [xpm](https://xpack.github.io/xpm/),
which is a portable [Node.js](https://nodejs.org/) command line application.

It is recommended to update to the latest version with:

```sh
npm install --global xpm@latest
```

For details please follow the instructions in the
[xPack install](https://xpack.github.io/install/) page.

### xpm

This package is available as
[`@micro-os-plus/architecture-cortexm`](https://www.npmjs.com/package/@micro-os-plus/architecture-cortexm)
from the `npmjs.com` registry:

```sh
cd my-project
xpm init # Unless a package.json is already present

xpm install @micro-os-plus/architecture-cortexm@latest

ls -l xpacks/@micro-os-plus/architecture-cortexm
```

### Git submodule

If, for any reason, **xpm** is not available, the next recommended
solution is to link it as a Git submodule below an `xpacks` folder.

```sh
cd my-project
git init # Unless already a Git project
mkdir -p xpacks

git submodule add https://github.com/micro-os-plus/architecture-cortexm-xpack.git \
  xpacks/@micro-os-plus/architecture-cortexm
```

## Branches

Apart from the unused `master` branch, there are two active branches:

- `xpack`, with the latest stable version (default)
- `xpack-development`, with the current development version

All development is done in the `xpack-development` branch, and contributions via
Pull Requests should be directed to this branch.

When new releases are published, the `xpack-development` branch is merged
into `xpack`.

## Developer info

### Overview

This source xpm package provides general Cortex-M definitions, the
reset handler, the default exception handlers, and the debugger entry
point. It complements the device specific CMSIS Core headers, but it
does not depend on them; the few system registers used are accessed
by their architectural addresses.

### Supported devices

Arm Cortex-M (M-profile) devices are supported, for the Armv6-M,
Armv7-M, Armv7E-M, Armv8-M Baseline, Armv8-M Mainline, and
Armv8.1-M Mainline architectures (for example Cortex-M0, M0+, M3, M4,
M7, M23, M33, and M55).

Cortex-A and Cortex-R devices differ in their exception model, stack
pointers, and semihosting trap instruction, and are supported by the
separate
[architecture-aarch32](https://github.com/micro-os-plus/architecture-aarch32-xpack)
package.

### Status

The **architecture-cortexm** source library is fully functional.

### Build & integration info

The project is written in C++, C, and assembly, and it is
expected to be used in C and C++ projects.

The source code is tested with arm-none-eabi-gcc 11 to 15,
and should be warning free.

To ease the integration of this package into user projects,
ready-made CMake and meson configuration files are provided (see below).

For other build systems, consider the following details:

#### Include folders

The following folders should be passed to the compiler during the build:

- `include`

The header files to be included in user projects are:

```c++
#include "micro-os-plus/architecture.h"
```

#### Source files

The source files to be added to user projects are:

- `src/show-cpuid.cpp` - display the core name and revision, read from
  the CPUID register, on the trace output
- `src/semihosting.cpp` - the semihosting call for this platform
- `src/_init_fini.c` - empty definitions required by newlib
- `src/reset-handler.c` - the `Reset_Handler`, which enables the FPU and
  the configurable faults, and jumps to `_start`
- `src/exception-handlers.cpp` - weak default exception handlers, which
  display the exception stack frame on the trace output
- `src/debugger-entry-point.S` - the `_debugger_entry_point`, used when
  starting the application from a debugger

#### Preprocessor definitions

The configuration definitions are expected either in the specific header file
`micro-os-plus/architecture-defines.h` or in the common project configuration
file `micro-os-plus/project-config.h`:

- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ENABLED` - enable the inclusion of the
  architecture files in the build; without it, the headers and the source
  files are empty; it must be defined only for Cortex-M devices
- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_RESET_HANDLER_ENABLED` - enable
  the reset handler in `src/reset-handler.c`
- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_EXCEPTION_HANDLERS_ENABLED` - enable
  the default exception handlers in `src/exception-handlers.cpp`
- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_DEBUGGER_ENTRY_POINT_ENABLED` -
  enable the debugger entry point in `src/debugger-entry-point.S`
- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_DEBUGGER_ENTRY_POINT_INITIALISE_VTOR_ENABLED` -
  make the debugger entry point set VTOR to `_interrupt_vectors`
- `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_VTOR_PRESENT` - define it for
  Armv6-M and Armv8-M Baseline devices that implement the optional VTOR
- `MICRO_OS_PLUS_SEMIHOSTING_ENABLED` - enable the semihosting call in
  `src/semihosting.cpp`; requires the `semihosting` package

#### Compiler options

- `-std=c++20` or higher for C++ sources
- `-std=c11` for C sources

#### C++ Namespaces

Portable:

- `micro_os_plus::architecture`
- `micro_os_plus::architecture::registers`

Architecture specific:

- `cortexm::architecture`
- `cortexm::architecture::registers`

#### C++ Classes

- none

#### Dependencies

- `@micro-os-plus/diag-trace` - required; its header is included by
  `micro-os-plus/architecture.h` (via `exception-handlers.h`)
- `@micro-os-plus/semihosting` - required; its header is included by
  `src/semihosting.cpp` and `src/exception-handlers.cpp`

These packages are not listed in `package.json`, and are not linked by the
CMake and meson configurations; the application must add them to the build.

#### CMake

To integrate the architecture-cortexm source library into a CMake application,
add this folder to the build:

```cmake
add_subdirectory("xpacks/@micro-os-plus/architecture-cortexm")
```

The result is an interface library that can be added as an application
dependency with:

```cmake
target_link_libraries(your-target PRIVATE

  micro-os-plus::architectures-cortexm
)
```

Architecture agnostic applications that refer to the generic
`micro-os-plus::architecture` library must define it in their platform
specific part, as an alias to `micro-os-plus::architectures-cortexm`.

The library includes the separate interface libraries
`micro-os-plus::architectures-cortexm-reset-handler`,
`micro-os-plus::architectures-cortexm-exception-handlers`, and
`micro-os-plus::architectures-cortexm-debugger-entry-point`, which are
also available individually.

#### meson

To integrate the architecture-cortexm source library into a meson application,
add this folder to the build:

```meson
subdir('xpacks/@micro-os-plus/architecture-cortexm')
```

The result is a dependency object that can be added
to an application with:

```meson
exe = executable(
  'your-target',
  c_args: micro_os_plus_architecture_cortexm_dependency_compile_c_args,
  cpp_args: micro_os_plus_architecture_cortexm_dependency_compile_cpp_args,
  link_with: [
    # Nothing, not static.
  ],
  dependencies: [
    micro_os_plus_architecture_cortexm_dependency,
  ]
)
```

Architecture agnostic applications that refer to the generic
`micro_os_plus_architecture_dependency` object (with the corresponding
`_compile_c_args` and `_compile_cpp_args` variables) must define them in
their platform specific part, as copies of
`micro_os_plus_architecture_cortexm_dependency` and its variables.

The dependency includes the separate
`micro_os_plus_architecture_cortexm_reset_handler_dependency`,
`micro_os_plus_architecture_cortexm_exception_handlers_dependency`, and
`micro_os_plus_architecture_cortexm_debugger_entry_point_dependency`
objects, which are also available individually.

#### xCDL

The package metadata for the xCDL build configuration tools is
available in `xcdl-package.jsonc`.

#### Linker scripts

The `linker-scripts` folder includes `sections-flash.ld`, a generic
linker script for configurations running from flash and RAM, and
`sections-ram.ld`, for configurations running entirely from RAM; they
may be redefined at device level.

#### SVD files

The `svd` folder includes SVD files with the system peripherals of each
architecture (Armv6-M, Armv7-M, Armv8-M Baseline, and Armv8-M Mainline),
for use with debuggers.

### Examples

With `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ENABLED` defined:

```c++
#include "micro-os-plus/architecture.h"

using namespace micro_os_plus;

architecture::register_t reg;

reg = architecture::registers::sp();

reg = cortexm::architecture::registers::psp();
```

### Known problems

- none

### Tests

There are no tests in this package yet; the functionality is exercised
by the tests of the packages that depend on it, like
[micro-test-plus](https://github.com/micro-os-plus/micro-test-plus-xpack).

## Change log - incompatible changes

According to [semver](https://semver.org) rules:

> Major version X (X.y.z | X > 0) MUST be incremented if any
> backwards incompatible changes are introduced to the public API.

The incompatible changes, in reverse chronological order,
are:

- v9.x:
  - `MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_ENABLED` must be defined to
    enable this library
- v8.x:
  - the reset handler, the exception handlers (moved from the
    `devices-qemu-cortexm` package), the debugger entry point, and
    `src/show-cpuid.cpp` were added
- v7.x:
  - the CMake alias was renamed `micro-os-plus::architectures-cortexm`
    (the generic `micro-os-plus::architecture` was preserved)
  - the specific meson dependency is
    `micro_os_plus_architecture_cortexm_dependency`
  - `src/semihosting.cpp` and `src/_init_fini.c` were added
    and must be compiled
  - the inline headers were moved to the `inlines` folder
- v6.x: the linker script was renamed `sections-flash.ld`
- v5.x: the TRACE macro was renamed MICRO_OS_PLUS_TRACE
- v4.x: move RTOS port sources to separate package

## License

Unless otherwise stated, the content is released under the terms of the
[MIT License](https://opensource.org/licenses/mit),
with all rights reserved to
[Liviu Ionescu](https://github.com/ilg-ul).
