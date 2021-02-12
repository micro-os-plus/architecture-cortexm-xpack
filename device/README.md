# Device specific files

These files are rightly located in the architecture-cortexm
package because they are common to all Arm Cortex-M devices,
but due to the specifics of CMSIS, which require the vendor supplied
"<device>.h" file (like "stm32f4xx.h"), available only in the
device library, these files can be compiled only in the device library.

TODO: explain how to include these files in the device library.
