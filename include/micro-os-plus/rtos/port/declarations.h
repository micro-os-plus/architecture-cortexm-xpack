/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
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

/*
 * This file is part of the CMSIS++ proposal, intended as a CMSIS
 * replacement for C++ applications.
 *
 * It is included in `cmsis-plus/rtos/os.h` to customise
 * it with Cortex-M specific declarations.
 */

#ifndef MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_RTOS_PORT_DECLARATIONS_H_
#define MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_RTOS_PORT_DECLARATIONS_H_

// ----------------------------------------------------------------------------

#include <micro-os-plus/config.h>
#include <micro-os-plus/rtos/port/declarations-c.h>
#include <micro-os-plus/rtos/port/defines.h>
#include <micro-os-plus/startup/defines.h>

// ----------------------------------------------------------------------------

#include <signal.h>
// Platform definitions
#include <sys/time.h>

// ----------------------------------------------------------------------------

#ifdef __cplusplus

#include <cstddef>
#include <cstdint>

namespace os
{
namespace rtos
{
namespace port
{
// ----------------------------------------------------------------------------

namespace stack
{
// Stack word.
using element_t = os_port_thread_stack_element_t;

// Align stack to 8 bytes.
using allocation_element_t = os_port_thread_stack_allocation_element_t;

// Initial value for the minimum stack size in bytes.
constexpr std::size_t min_size_bytes = OS_INTEGER_RTOS_MIN_STACK_SIZE_BYTES;

// Initial value for the default stack size in bytes.
constexpr std::size_t default_size_bytes
    = OS_INTEGER_RTOS_DEFAULT_STACK_SIZE_BYTES;

// Used to fill in the stack.
constexpr element_t magic = OS_INTEGER_STARTUP_STACK_FILL_MAGIC; // DEADBEEF

} /* namespace stack */

namespace interrupts
{
// Type to store the entire processor interrupts mask.
using state_t = os_port_irq_state_t;

namespace state
{
constexpr state_t init = 0;
} /* namespace state */

} /* namespace interrupts */

namespace scheduler
{
using state_t = os_port_scheduler_state_t;

namespace state
{
constexpr state_t locked = true;
constexpr state_t unlocked = false;
constexpr state_t init = unlocked;
} /* namespace state */

extern state_t lock_state;

} /* namespace scheduler */

using thread_context_t = struct context_s
{
  // On Cortex-M cores the context itself is stored on the stack,
  // only the stack pointer needs to be preserved.
  stack::element_t* stack_ptr;
};

// ----------------------------------------------------------------------------

} /* namespace port */
} /* namespace rtos */
} /* namespace os */

// ----------------------------------------------------------------------------

#endif /* __cplusplus */

#endif /* MICRO_OS_PLUS_ARCHITECTURE_CORTEXM_RTOS_PORT_DECLARATIONS_H_ */

// ----------------------------------------------------------------------------
