# Change & release log

Releases in reverse chronological order.

Please check
[GitHub](https://github.com/micro-os-plus/architecture-cortexm-xpack/issues/)
and close existing issues and pull requests.

## 2026-09-08

* v8.0.0
* 2232762 show-cpuid group 6M & 7M definitions
* c36d5e6 add svds
* 3c653f5 fix debugger-entry-point.S includes
* fb883a2 add micro_os_plus_semihosting_register_t

## 2026-09-07

* eb23f0f cosmetics xpm packages
* 30b12de debugger-entry-point fix syntax & cosmetics

## 2026-09-06

* fb14d10 settings.json update
* 0897030 move handlers from device_qemu here
* c3ed570 functions.h: plan for tz_set_stackseal
* 0f82d7e registers.h cosmetics
* 65fd835 add psp, msplim, psplim accessors
* c3d64f8 registers.h: fix guard
* 2154f94 exception-handlers.h: add support for arm v8m
* 077fd84 add show-cpuid
* 5a00e30 linker scripts final edits before moving to startup

## 2026-08-05

* d063178 readme cosmetics

## 2026-08-04

* 08b4f9c 7.0.0
* 0f94e4d prepare v7.0.0
* 7faca60 re-format
* 5b196e1 re-generate top commons
* 2fd915d package*.json update
* a5fed11 CHANGELOG update
* 4b810b2 re-generate top xcdl
* d186f80 re-generate top commons
* b917041 xcdl-package.jsonc update
* a254b24 update guards & includes

## 2026-07-21

* ba536d9 re-generate top commons
* 13ebf8c reorganise inlines, separate semihosting.cpp

## 2026-07-20

* cc5f620 xcdl-export with alias
* 9c53975 xcdl templates add alias
* 12aed50 update copyright notices
* 9a9a2f6 re-generate top commons

## 2026-06-06

* 4fd5a88 copyright update 2026

## 2025-11-20

* 1376560 rename npm-pack

## 2025-10-07

* 2847586 update copyright notices

## 2023-11-28

* 1e9215a README updates
* d25172b package.json: cosmetise scripts

## 2023-10-19

* 54368fd package.json: cosmetise description
* da2238d README updates

## 2023-07-14

* c299074 README updates
* 124dee1 semihosting-inlines.h: cosmetics
* f0ff4be package.json: cosmetise urls
* 5d632fc package.json: minXpm 0.16.2

## 2023-06-04

* 2b58282 update for @scope/name

## 2023-06-03

* f8f1da0 lower case ci.yml
* e6aa8f3 package.json min 0.16.0

## 2023-05-08

* 4b82d62 .npmignore update
* 351bbd8 6.3.0
* b8c4e99 prepare v6.3.0
* 30790e4 README updates
* 3711756 README update
* 594cf3e #6: add _init_fini()

## 2022-08-16

* e40dd9d package.json min 0.14.0 & defaults

## 2022-08-03

* 171a9e9 .vscode/settings.json: cmake.ignoreCMakeListsMissing
* aaba4fe .vscode/settings.json: makefile.configureOnOpen

## 2022-07-28

* v6.2.0
* 29d4946 #5: add signed_register_t for semihosting result
* ca06f6b #4: add sections-ram.ld

## 2022-07-25

* 285a2f6 add preliminary xpack.json

## 2022-06-09

* v6.1.0 released
* a5fd72a sections-flash.ld update stack & heap; cleanups
* bc55a40 include: add setter & shorten namespaces
* b542bc6 exception-handlers.h: add handler_ptr_t

## 2022-06-04

* v6.0.0

## 2022-06-03

* bcd8e9a semihosting-inlines.h simplify for thumb only
* a7c98c8 rename sections-flash.ld

## 2022-05-26

* 6fbb090 semihosting-inlines: add param_block_t and response_t
* 3e38bfc sections.ld: add `__end__`
* 64824ee -std=c++20

## 2022-02-08

* v5.0.0
* aeeef4a rename MICRO_OS_PLUS_TRACE

## 2022-02-05

* v4.0.1

## 2022-02-04

* v4.0.0
* cmake: add xpack_display_target_lists

## 2022-02-02

* add meson support
* add registers-* files
* mode files to device-cortexm package

## 2022-01-27

* move rtos-port-sources to separate package

## 2022-01-26

* v3.1.0
* rework CMake

## 2022-01-02

* v3.0.2

## 2021-03-10

* v3.0.1
* fix/silence warnings

## 2021-03-09

* v3.0.0
* separate rtos-port folder
* fix/silence warnings

## 2021-03-01

* v2.0.0

## 2021-02-28

* rename namespaces, prefixes, etc to micro_os_plus

## 2021-02-04

* v1.1.0
* content moved here form single repo
