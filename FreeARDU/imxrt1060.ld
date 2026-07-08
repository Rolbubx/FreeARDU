/*
 * Linker script for i.MX RT1060 (MIMXRT1062)
 * 1MB RAM total:
 * - ITCM: 512KB (Instruction TCM) - 0x00000000
 * - DTCM: 512KB (Data TCM) - 0x20000000
 * - OCRAM: 512KB (On-chip RAM) - 0x20200000
 * Note: The 1MB is flexible, but typical default is 512KB ITCM/DTCM + 512KB OCRAM.
 * For bare-metal, we can put everything in DTCM/OCRAM if we don't use FlexSPI Flash.
 * If we use Flash, it's at 0x60000000.
 */

MEMORY
{
  ITCM  (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
  DTCM  (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
  OCRAM (rwx) : ORIGIN = 0x20200000, LENGTH = 512K
  FLASH (rx)  : ORIGIN = 0x60000000, LENGTH = 8M
}

ENTRY(reset_handler)

_estack = ORIGIN(DTCM) + LENGTH(DTCM);
_min_heap_size = 0x200;
_min_stack_size = 0x400;

SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } > FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  .data : 
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > DTCM AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } > DTCM

  .heap :
  {
    . = ALIGN(8);
    _sheap = .;
    . = . + _min_heap_size;
    . = ALIGN(8);
    _eheap = .;
  } > DTCM

  /* Check if there is enough space for stack */
  ._stack :
  {
    . = ALIGN(8);
    . = . + _min_stack_size;
    . = ALIGN(8);
  } > DTCM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
