/*
 * Default linker script.
 * 
 * Static constructors/destructors use the new .init_array/.fini_array
 * definitions; the old .init/.fini are no longer used.
 *
 * The heap starts immediately after the last statically allocated 
 * .bss/.noinit section (the _end symbol), and extends up to the stack.
 *
 * To make use of the multi-region initialisations, define
 * OS_INCLUDE_STARTUP_INIT_MULTIPLE_RAM_SECTIONS for the _startup.cpp file.
 */

 /*
 * The '__stack' definition is required by newlib crt0; do not remove it.
 * The stack is located at the very end of the RAM region.
 */
__stack = DEFINED(__stack) ? __stack : ORIGIN(RAM) + LENGTH(RAM);
__stack_size = DEFINED(__stack_size) ? __stack_size : 2K;

_initial_main_stack_pointer = __stack; /* µOS++ specific. */

_estack = __stack; 	/* STM specific definition */

/* 
 * The entry point is important for debuggers and simulators, the
 * hardware has its own notion of the startup address.
 */
ENTRY(Reset_Handler)

SECTIONS
{
  /*
   * For Cortex-M devices, the beginning of the startup code is stored in
   * the .interrupt_vectors section, which goes to FLASH. 
   */
  .interrupt_vectors : ALIGN(4)
  {
    FILL(0xFF)

    __vectors_start = ABSOLUTE(.) ;
    __vectors_start__ = ABSOLUTE(.) ; /* STM specific definition */

    KEEP(*(.interrupt_vectors))    /* Interrupt vectors */

    KEEP(*(.cfmconfig))            /* Freescale configuration words */   
  } >FLASH
  
  /* 
   * Dynamic Run Time Metadata.
   * Normally the debugger reaches this section by resolving a symbol, 
   * but in case it should scan memory for the magic, better place this
   * section as early as possible. 
   */
  .drtm : ALIGN(4)
  {
    KEEP(*(.drtm .drtm.*))    
  } >FLASH

  /* 
   * This section is here for convenience, to store the
   * startup code at the beginning of the flash area, hoping that
   * this will increase the readability of the listing.
   */
  .after_vectors : ALIGN(4)
  {
    *(.after_vectors .after_vectors.*)	/* Startup code and ISRs */
  } >FLASH

  /* 
   * Memory regions initialization arrays.
   *
   * There are two kinds of arrays for each RAM region, one for 
   * data and one for bss. Each is iterated at startup and the   
   * region initialization is performed.
   * 
   * The data array includes:
   * - from (LOADADDR())
   * - region_begin (ADDR())
   * - region_end (ADDR()+SIZEOF())
   *
   * The bss array includes:
   * - region_begin (ADDR())
   * - region_end (ADDR()+SIZEOF())
   *
   * WARNING: It is mandatory that the regions are word aligned, 
   * since the initialization code works only on words.
   */
  .mem_inits : ALIGN(4)
  {     
    PROVIDE_HIDDEN(__data_regions_array_begin__ = .); /* µOS++ specific. */
    
    LONG(LOADADDR(.data));
    LONG(ADDR(.data));
    LONG(ADDR(.data)+SIZEOF(.data));
    
    /* If more DATA regions are needed, add more such records. */   
            
    PROVIDE_HIDDEN(__data_regions_array_end__ = .); /* µOS++ specific. */
    
    PROVIDE_HIDDEN(__bss_regions_array_begin__ = .); /* µOS++ specific. */
    
    LONG(ADDR(.bss));
    LONG(ADDR(.bss)+SIZEOF(.bss));

    /* If more BSS regions are needed, add more such records. */   
            
    PROVIDE_HIDDEN(__bss_regions_array_end__ = .); /* µOS++ specific. */
  } >FLASH

  /*
   * The preinit code, i.e. an array of pointers to initialization 
   * functions to be performed before constructors.
   */
  .preinit_array : ALIGN(4)
  {
    /* 
     * PROVIDE not used intentionally, 
     * this symbol must not be used for other purposes. 
     */
    __preinit_array_begin__ = .;   /* µOS++ specific. */
    
    /*
     * Used to run the system inits before anything else.
     */
    KEEP(*(.preinit_array_sysinit .preinit_array_sysinit.*))
    
    /* 
     * Used for other platform inits.
     */
    KEEP(*(.preinit_array_platform .preinit_array_platform.*))
    
    /*
     * The application inits. If you need to enforce some order in 
     * execution, create new sections, as before.
     */
    KEEP(*(.preinit_array .preinit_array.*))

    __preinit_array_end__ = .;     /* µOS++ specific. */
  } >FLASH

  /*
   * The init code, i.e. an array of pointers to static constructors.
   */
  .init_array : ALIGN(4)
  {
    /* PROVIDE not used intentionally, this symbol must not be used. */
    __init_array_start = .;        /* Standard newlib definition. */
    __init_array_begin__ = .;      /* µOS++ specific. */
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    __init_array_end = .;          /* Standard newlib definition. */
    __init_array_end__ = .;        /* µOS++ specific. */
  } >FLASH

  /*
   * The fini code, i.e. an array of pointers to static destructors.
   */
  .fini_array : ALIGN(4)
  {
    /* PROVIDE not used intentionally, this symbol must not be used. */
    __fini_array_start = .;        /* Standard newlib definition. */
    __fini_array_begin__ = .;      /* µOS++ specific. */
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    __fini_array_end = .;          /* Standard newlib definition. */
    __fini_array_end__ = .;        /* µOS++ specific. */
  } >FLASH

  /*
   * The program code.
   */
  .text : ALIGN(4)
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >FLASH

  . = ALIGN(4);
  PROVIDE(__etext = .);
  PROVIDE(_etext = .);
  PROVIDE(etext = .);

  /*
   * C++ virtual tables.
   */
  .vtable : ALIGN(4)
  {
    KEEP(*(vtable))            
  } >FLASH

  /*
   * Exception frames.
   */
  .exceptions : ALIGN(4)
  {
    KEEP(*(.eh_frame*))
    *(.gcc_except_table)
  } >FLASH

  /*
   * Stub sections generated by the linker, to glue together 
   * ARM and Thumb code. .glue_7 is used for ARM code calling 
   * Thumb code, and .glue_7t is used for Thumb code calling 
   * ARM code. Apparently always generated by the linker, for some
   * architectures, so better leave them here.
   */
  .glue : ALIGN(4)
  {
    *(.glue_7)
    *(.glue_7t)
  } >FLASH


  /* ARM magic sections, after exceptions. */
  .ARM.extab : ALIGN(4)
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH
  
  . = ALIGN(4);
  __exidx_start = .;   	
  .ARM.exidx : ALIGN(4)
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } >FLASH
  __exidx_end = .;

  /*
   * Read-only data (constants) 
   */
  .rodata : ALIGN(4)
  {
    *(.rodata .rodata.*)         
    *(.constdata .constdata.*)         
    *(.gnu.linkonce.r.*)
  } >FLASH

  . = ALIGN(4);
  PROVIDE( _data = . );

  /*
   * The initialised data section.
   *
   * The program executes knowing that the data is in RAM
   * but the loader puts the initial values in FLASH.
   * The startup will copy the initial values from FLASH to RAM.
   */
  .data : ALIGN(4)
  {
    FILL(0xFF)
    
    __data_start__ = . ;           /* Standard newlib definition. */
    __data_begin__ = . ;           /* µOS++ specific */
    *(.data_begin .data_begin.*)   /* µOS++ __data_begin_guard */

    *(.data .data.*)
    *(.gnu.linkonce.d.*)
            
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)
    
    *(.data_end .data_end.*)       /* µOS++ __data_end_guard; must be last */        
    . = ALIGN(4);
    __data_end__ = . ;             /* Standard newlib definition. */
  } >RAM AT>FLASH

  /* 
   * This address is used by the µOS++ startup code to 
   * initialise the .data section.
   */
  __data_load_addr__ = LOADADDR(.data);

  . = ALIGN(4);
  PROVIDE( __edata = . );
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  /*
   * The uninitialised data sections. NOLOAD is used to avoid
   * the "section `.bss' type changed to PROGBITS" warning
   */

  /* The primary uninitialised data section. */
  .bss (NOLOAD) : ALIGN(4)
  {
    __bss_start = .;               /* Standard newlib definition. */
    __bss_start__ = .;             /* Standard newlib definition. */
    __bss_begin__ = .;          /* µOS++ specific */
    *(.bss_begin .bss_begin.*)    /* µOS++ __bss_begin_guard */ 

    *(.sbss .sbss.*)
    *(.gnu.linkonce.sb.*)
    
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    
    *(.bss_end .bss_end.*)         /* µOS++ __bss_end_guard; must be last */
    . = ALIGN(4);
    __bss_end__ = .;               /* Standard newlib definition. */
    __bss_end = .;                 /* Standard newlib definition. */
  } >RAM

  /* 
   * Similar to .bss, but not initialised to zero. µOS++ extension.
   */
  .noinit (NOLOAD) : ALIGN(4)
  {
    __noinit_begin__ = .;          /* µOS++ extension. */
    
    *(.noinit .noinit.*) 
    
    . = ALIGN(4) ;
    __noinit_end__ = .;            /* µOS++ extension. */
  } >RAM

  /* _sbrk() expects at least word alignment. */
  . = ALIGN(8);
  PROVIDE( __end = . );
  PROVIDE( _end = . );
  PROVIDE( end = . ); 

  PROVIDE( __heap_begin__ = . );     /* µOS++ extension. */

  /*
   * It should generate an error if the heap overides the stack.
   */
  .stack __stack - __stack_size :
  {
    PROVIDE( _heap_end = . );      /* Standard newlib definition. */
    PROVIDE( __heap_end__ = . );   /* µOS++ extension. */
    . += __stack_size;
  } >RAM

  /* ---------------------------------------------------------------------- */
  /* After that there are only debugging sections. */

  /* 
   * Stabs debugging sections.
   */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }

  /*
   * DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to the beginning
   * of the section so we begin them at 0.  
   */
   
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }    
}
