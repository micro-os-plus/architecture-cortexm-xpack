/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2017 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CMSIS_PLUS_DIAG_DRTI_H_
#define CMSIS_PLUS_DIAG_DRTI_H_

// ----------------------------------------------------------------------------

// Debug Run Time Information, a set of metadata used by debuggers
// to get a better knowledge on the RTOS internals.

// http://semver.org

#define OS_RTOS_DRTI_VERSION_MAJOR  0
#define OS_RTOS_DRTI_VERSION_MINOR  1
#define OS_RTOS_DRTI_VERSION_PATCH  0

// ----------------------------------------------------------------------------

#define OS_RTOS_DRTI_OFFSETOF_MAGIC 0x00
#define OS_RTOS_DRTI_OFFSETOF_VERSION 0x04
#define OS_RTOS_DRTI_OFFSETOF_SCHEDULER_IS_STARTED_ADDR 0x08
#define OS_RTOS_DRTI_OFFSETOF_SCHEDULER_TOP_THREADS_LIST_ADDR 0x0C
#define OS_RTOS_DRTI_OFFSETOF_SCHEDULER_CURRENT_THREAD_ADDR 0x10
#define OS_RTOS_DRTI_OFFSETOF_THREAD_NAME_OFFSET 0x14
#define OS_RTOS_DRTI_OFFSETOF_THREAD_PARENT_OFFSET 0x16
#define OS_RTOS_DRTI_OFFSETOF_THREAD_LIST_NODE_OFFSET 0x18
#define OS_RTOS_DRTI_OFFSETOF_THREAD_CHILDREN_NODE_OFFSET 0x1A
#define OS_RTOS_DRTI_OFFSETOF_THREAD_STATE_OFFSET 0x1C
#define OS_RTOS_DRTI_OFFSETOF_THREAD_STACK_OFFSET 0x1E
#define OS_RTOS_DRTI_OFFSETOF_THREAD_PRIO_ASSIGNED 0x20
#define OS_RTOS_DRTI_OFFSETOF_THREAD_PRIO_INHERITED 0x22

// ----------------------------------------------------------------------------

#endif /* CMSIS_PLUS_DIAG_DRTI_H_ */
