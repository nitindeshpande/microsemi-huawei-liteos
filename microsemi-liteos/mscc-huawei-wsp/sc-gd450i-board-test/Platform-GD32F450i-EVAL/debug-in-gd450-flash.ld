/*******************************************************************************
 * (c) Copyright 2017 Microsemi SoC Products Group.  All rights reserved.
 * 
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
GROUP(-lc -lgcc -lm)
OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)
SEARCH_DIR(.)
__DYNAMIC  =  0;

/*******************************************************************************
 * Start of board customization.
 *******************************************************************************/
MEMORY
{
    /*
    *   In general, example LD scripts use lowest common memory footprint
    *   so will work with all devices.
    */

    /* GD450 internal SRAM */
    ram (rw) : ORIGIN = 0x20000000, LENGTH = 190k
    
    /* GD450 internal Flash */
    flash (rwx) : ORIGIN = 0x08000000, LENGTH = 3M
}

RAM_START_ADDRESS   = 0x20000000;       /* Must be the same value MEMORY region ram ORIGIN above. */
RAM_SIZE            = 190k;             /* Must be the same value MEMORY region ram LENGTH above. */
MAIN_STACK_SIZE     = 40k;              /* Cortex main stack size. */
MIN_SIZE_HEAP       = 40k;              /* needs to be calculated for your application */

/* Please note that unassigned RAM will be allocated to the .heap section. */

/*******************************************************************************
 * End of board customization.
 *******************************************************************************/
 
PROVIDE (__main_stack_start = RAM_START_ADDRESS + RAM_SIZE);
PROVIDE (_estack = __main_stack_start);
PROVIDE (__mirrored_nvm = 0);   /* Indicate to startup code that NVM is not mirrored to VMA address .text copy is required. */

 /*
  * Remap instruction for start-up code and debugger.
  *  set __smartfusion2_memory_remap to one of the following:
  *  0: remap eNVM to address 0x00000000                           Production mode or debugging from eNVM   
  *  1: remap eSRAM to address 0x00000000                          See note 1 below.
  *  2: remap external DDR memory to address 0x00000000            Debugging from or production relocate to DDR memory
  *  note 1: This option should only be used in production mode if required. When debugging using eSRAM, code is not
  *  relocated and __smartfusion2_memory_remap should be set to option 0. In revision 7419 and below of
  *  this file, __smartfusion2_memory_remap was set to option 1. This remap was not required and could lead to an issue
  *  when displaying some invalid memory locations in the debugger using some Libero designs.
  * 
  */

PROVIDE (__smartfusion2_memory_remap = 0);

SECTIONS
{
  . = 0x08000000;
  .vector_table : ALIGN(0x10)
  {
    __vector_table_load = LOADADDR(.vector_table);
    __vector_table_start = .;
    __vector_table_vma_base_address = .;
    KEEP(*(.isr_vector))
    . = ALIGN(0x10);
    _evector_table = .;
  } >flash
  
  .boot_code : ALIGN(0x10) /* When all code in RAM, no requirement for this section- but adds clarity when looking at .lst file */
  {
    *(.boot_code)
    . = ALIGN(0x10);
  } >flash
  
  .text :
  ALIGN(0x10)
  {
    CREATE_OBJECT_SYMBOLS
    __text_load = LOADADDR(.text);
    __text_start = .;
    *(.text .text.* .gnu.linkonce.t.*)
    *(.plt)
    *(.gnu.warning)
    *(.glue_7t) *(.glue_7) *(.vfp11_veneer)

    . = ALIGN(0x4);
    /* These are for running static constructors and destructors under ELF.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

    *(.rodata .rodata.* .gnu.linkonce.r.*)

    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.gcc_except_table) 
    *(.eh_frame_hdr)
    *(.eh_frame)

    KEEP (*(.vector_table))
    KEEP (*(.init))
    KEEP (*(.fini))

    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(0x10);
  } >flash
  
  /* .ARM.exidx is sorted, so has to go in its own output section.  */
   __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } >flash
  
  __exidx_end = .;
  _etext = .;
  PROVIDE(__text_end = .);

  . = 0x20000000;
  .data :
  ALIGN(0x10)
  {
    __data_load = LOADADDR (.data);
    _sidata = LOADADDR (.data);
    __data_start = .;
    _sdata = .;
    KEEP(*(.jcr))
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.* .gnu.linkonce.d.*)
    . = ALIGN(0x10);
    _edata = .;
  } >ram 

  .bss : ALIGN(0x10)
  {
    __bss_start__ = . ;
    _sbss = .;
    *(.shbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(0x10);
    __bss_end__ = .;
    _end = .;
    __end = _end;
    _ebss = .;
    PROVIDE(end = .);
  } >ram 

  .heap : ALIGN(0x10)
  {
    __heap_start__ = .;
    . += MIN_SIZE_HEAP;         /* will generate error if this minimum size not available */
    . += ((ABSOLUTE(RAM_START_ADDRESS) + RAM_SIZE - MAIN_STACK_SIZE) - .); /* assumes stack starts after heap */
    _eheap = .;
  } >ram
  
  .stack : ALIGN(0x10)
  {
    __stack_start__ = .;
    . += MAIN_STACK_SIZE;
    _estack = .;
  } >ram
  
  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
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
  .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.isr_vector) }
}
