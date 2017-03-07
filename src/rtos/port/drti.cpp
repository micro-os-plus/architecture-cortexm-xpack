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

#include <cmsis-plus/rtos/port/diag/drti.h>

#include <cmsis-plus/rtos/os.h>
#include <cstdint>

// ----------------------------------------------------------------------------

/*
 * Debug Run Time Information.
 *
 * This is custom metadata stored in program flash space, intended to help
 * debuggers access internal RTOS objects.
 *
 * The preliminary version 0.x uses POD structs; future versions
 * might consider more metadata and a more elaborate structure, like
 * a compile time binary JSON.
 *
 * To include DRTI support in an application:
 * - define `OS_INCLUDE_RTOS_DRTI`
 * - preferably add `KEEP(*(.drti))` to the `.isr_vector` section
 * of the linker script; this will ensure the DRTI data is
 * allocated right after the vectors;
 * - alternatively add `-u os_rtos_drti` to the linker command line.
 */

#if defined(OS_INCLUDE_RTOS_DRTI)

namespace os
{
  namespace rtos
  {
    namespace port
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
      typedef __attribute__ ((packed, aligned((4))))
      struct drti_s
      {
        // 0x00, 4 bytes
        uint8_t magic[4];

        // 0x04, 4 bytes
        struct
        {
          uint8_t v;
          uint8_t major;
          uint8_t minor;
          uint8_t patch;
        } version;

        // 0x08, 32-bits pointer
        void* scheduler_is_started_addr;

        // 0x0C, 32-bits pointer
        void* scheduler_top_threads_list_addr;

        // 0x10, 32-bits pointer
        volatile void* scheduler_current_thread_addr;

        // 0x14, 16-bits unsigned int
        uint16_t thread_name_offset;

        // 0x16, 16-bits unsigned int
        uint16_t thread_parent_offset;

        // 0x18, 16-bits unsigned int
        uint16_t thread_list_node_offset;

        // 0x1A, 16-bits unsigned int
        uint16_t thread_children_node_offset;

        // 0x1C, 16-bits unsigned int
        uint16_t thread_state_offset;

        // 0x1E, 16-bits unsigned int
        uint16_t thread_stack_offset;

        // 0x20, 16-bits unsigned int
        uint16_t thread_prio_assigned;

        // 0x22, 16-bits unsigned int
        uint16_t thread_prio_inherited;

      } drti_t;
#pragma GCC diagnostic pop
    }
  }
}

extern "C" const os::rtos::port::drti_t os_rtos_drti;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

const __attribute__ ((section(".drti")))
struct os::rtos::port::drti_s os_rtos_drti =
  {
      { 'D', 'R', 'T', 'I'},
      { 'v', OS_RTOS_DRTI_VERSION_MAJOR, OS_RTOS_DRTI_VERSION_MINOR,
        OS_RTOS_DRTI_VERSION_PATCH}, /**/

    &os::rtos::scheduler::is_started_, /**/
    &os::rtos::scheduler::top_threads_list_, /**/
    &os::rtos::scheduler::current_thread_, /**/

    offsetof(os::rtos::thread, name_),
    offsetof(os::rtos::thread, parent_), /**/
    offsetof(os::rtos::thread, child_links_), /**/
    offsetof(os::rtos::thread, children_), /**/
    offsetof(os::rtos::thread, state_), /**/
    offsetof(os::rtos::thread, context_.port_.stack_ptr),  /**/
    offsetof(os::rtos::thread, prio_assigned_),  /**/
    offsetof(os::rtos::thread, prio_inherited_)
  };

#pragma GCC diagnostic pop

#endif /* defined(OS_INCLUDE_RTOS_DRTI) */

// ----------------------------------------------------------------------------
