# Change & release log

Releases in reverse chronological order.

Please check
[GitHub](https://github.com/micro-os-plus/architecture-cortexm-xpack/issues/)
and close existing issues and pull requests.

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
