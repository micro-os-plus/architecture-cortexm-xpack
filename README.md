[![license](https://img.shields.io/github/license/micro-os-plus/architecture-cortexm-xpack)](https://github.com/micro-os-plus/architecture-cortexm-xpack/blob/xpack/LICENSE)
[![CI on Push](https://github.com/micro-os-plus/architecture-cortexm-xpack/workflows/CI%20on%20Push/badge.svg)](https://github.com/micro-os-plus/architecture-cortexm-xpack/actions?query=workflow%3A%22CI+on+Push%22)

# A source library xPacks with the µOS++ Arm Cortex-M architecture definitions

The project is hosted on GitHub as
[micro-os-plus/architecture-cortexm-xpack](https://github.com/micro-os-plus/architecture-cortexm-xpack).

## Maintainer info

This page is addressed to developers who plan to include this source
library into their own projects.

For maintainer info, please see the
[README-MAINTAINER](README-MAINTAINER.md) file.

## Install

As a source library xPacks, the easiest way to add it to a project is via
**xpm**, but it can also be used as any Git project, for example as a submodule.

### Prerequisites

A recent [xpm](https://xpack.github.io/xpm/),
which is a portable [Node.js](https://nodejs.org/) command line application.

For details please follow the instructions in the
[xPack install](https://xpack.github.io/install/) page.

### xpm

Note: the package will be available from npmjs.com at a later date.

For now, it can be installed from GitHub:

```sh
cd my-project
xpm init # Unless a package.json is already present

xpm install github:micro-os-plus/architecture-cortexm-xpack
```

When ready, this package will be available as
[`@micro-os-plus/architecture-cortexm`](https://www.npmjs.com/package/@micro-os-plus/architecture-cortexm)
from the `npmjs.com` registry:

```sh
cd my-project
xpm init # Unless a package.json is already present

xpm install @micro-os-plus/architecture-cortexm@latest

ls -l xpacks/micro-os-plus-architecture-cortexm
```

### Git submodule

If, for any reason, **xpm** is not available, the next recommended
solution is to link it as a Git submodule below an `xpacks` folder.

```sh
cd my-project
git init # Unless already a Git project
mkdir -p xpacks

git submodule add https://github.com/micro-os-plus/architecture-cortexm-xpack.git \
  xpacks/micro-os-plus-architecture-cortexm
```

## Branches

Apart from the unused `master` branch, there are two active branches:

- `xpack`, with the latest stable version (default)
- `xpack-develop`, with the current development version

All development is done in the `xpack-develop` branch, and contributions via
Pull Requests should be directed to this branch.

When new releases are published, the `xpack-develop` branch is merged
into `xpack`.

## Developer info

This source xPack provides general Cortex-M definitions in addition to the
CMSIS Core.

### Status

The µOS++ Cortex-M definitions are functional.

### Build & integration info

To include this package in a project, consider the following details.

#### Include folders

- `include`

The header file to be included is:

```c++
#include <micro-os-plus/architecture.h>
```

#### Source folders

- none

#### Preprocessor definitions

- none

#### Compiler options

- `-std=c++20` or higher for C++ sources
- `-std=c11` for C sources

#### C++ Namespaces

- `micro_os_plus::architecture`

#### C++ Classes

- none

### Examples

TBD

### Known problems

- none

### Tests

## Change log - incompatible changes

According to [semver](https://semver.org) rules:

> Major version X (X.y.z | X > 0) MUST be incremented if any
backwards incompatible changes are introduced to the public API.

The incompatible changes, in reverse chronological order,
are:

- v6.x: the linker script was renamed `sections-flash.ld`
- v5.x: the TRACE macro was renamed MICRO_OS_PLUS_TRACE
- v4.x: move RTOS port sources to separate package

## License

The original content is released under the
[MIT License](https://opensource.org/licenses/MIT/),
with all rights reserved to
[Liviu Ionescu](https://github.com/ilg-ul/).
