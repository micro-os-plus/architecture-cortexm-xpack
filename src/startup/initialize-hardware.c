/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2015 Liviu Ionescu.
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

#if defined(__ARM_EABI__)

// ----------------------------------------------------------------------------

#include <micro-os-plus/startup/hooks.h>
#include <micro-os-plus/device.h>

// ----------------------------------------------------------------------------

/**
 * @details
 * This is the default implementation for the second hardware
 * initialisation routine.
 *
 * It is called from `_start()`, right after DATA & BSS init, before
 * the static constructors.
 *
 * The application can and should
 * redefine it for more complex cases that
 * require custom inits (before constructors), otherwise these inits can
 * be done in main().
 */
void __attribute__ ((weak)) os_startup_initialize_hardware (void)
{
  // Call the CSMSIS system clock routine to store the clock frequency
  // in the SystemCoreClock global RAM location.
  SystemCoreClockUpdate ();
}

// ----------------------------------------------------------------------------

#endif /* defined(__ARM_EABI__) */

// ----------------------------------------------------------------------------
