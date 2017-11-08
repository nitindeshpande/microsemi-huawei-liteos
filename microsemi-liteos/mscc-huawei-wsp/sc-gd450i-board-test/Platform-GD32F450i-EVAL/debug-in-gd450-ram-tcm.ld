/*******************************************************************************
 * (c) Copyright 2015 Microsemi SoC Products Group.  All rights reserved.
 * 
 * file name : debug-in-gd450-ram-tcm.ld
 * GD32F450ik linker script for creating SoftConsole downloadable image executing 
 * code from onchip SRAM. The TCM is also supported. Data structures can be placed in
 * TCM by using sections attribute.
 *  e.g. UINT8 g_ucMemStart[OS_SYS_MEM_SIZE] __attribute__ ((section (".tcm")));
 *
 * GD450: SRAM0 (112KB) + SRAM1 (16KB) + SRAM2 (64KB) = total 190K and TCMSRAM = 64K
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
    /* GD450 SRAM0 (112KB) + SRAM1 (16KB) + SRAM2 (64KB) = total 192K */
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 190k
    
    /*GD450 TCMSRAM*/
    tcm (rw) : ORIGIN = 0x10000000, LENGTH = 64k
}

RAM_START_ADDRESS   = 0x20000000;       /* Must be the same value MEMORY region ram ORIGIN above. */
RAM_SIZE            = 190k;              /* Must be the same value MEMORY region ram LENGTH above. */
MAIN_STACK_SIZE     = 4k;               /* Cortex main stack size. */
MIN_SIZE_HEAP       = 4k;               /* needs to be calculated for your application */

TCM_START_ADDRESS   = 0x10000000;       
TCM_SIZE            = 64k;

/* Please note that unassigned TCM will be allocated to the .heap section. */

/*******************************************************************************
 * End of board customization.
 *******************************************************************************/
 
PROVIDE (__main_stack_start = RAM_START_ADDRESS + RAM_SIZE);
PROVIDE (_estack = __main_stack_start);

SECTIONS
{
  .vector_table : ALIGN(0x10)
  {
    __vector_table_load = LOADADDR(.vector_table);
    __vector_table_start = .;
    __vector_table_vma_base_address = .;
    KEEP(*(.isr_vector))
    . = ALIGN(0x10);
    _evector_table = .;
  } >ram
  
  .boot_code : ALIGN(0x10) /* When all code in RAM, no requirement for this section- but adds clarity when looking at .lst file */
  {
    *(.boot_code)
    . = ALIGN(0x10);
  } >ram
  
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

    . = ALIGN(0x10);
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
  } >ram
  /* .ARM.exidx is sorted, so has to go in its own output section.  */
   __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } >ram
  __exidx_end = .;
  _etext = .;
  PROVIDE(__text_end = .);

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
    PROVIDE(__data_end = .);
  } >ram

  .bss : ALIGN(0x10)
  {
    __bss_start__ = . ;
    PROVIDE(__bss_start = .);
    _sbss = .;
    *(.shbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(0x10);
    PROVIDE(__bss_end = .);
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
    _eheap = .;
  } >ram
  
  .stack : ALIGN(0x10)
  {
    __stack_start__ = .;
    . += MAIN_STACK_SIZE;
    _estack = .;
  } >ram

  . = 0x10000000;
  .tcm : ALIGN(0x10)
  {
    *(.tcm)
    . = ALIGN(0x10);
  } >tcm

 
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
