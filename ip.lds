OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")

MEMORY
{
  systemid : ORIGIN = 0x8c008000, LENGTH = 0x100
  toc      : ORIGIN = 0x8c008100, LENGTH = 0x200
  sg_sec   : ORIGIN = 0x8c008300, LENGTH = 0x3400
  sg_are   : ORIGIN = 0x8c00b700, LENGTH = 0x100
  sg_ini   : ORIGIN = 0x8c00b800, LENGTH = 0x4800
/*  sg_ini   : ORIGIN = 0x8c00b800, LENGTH = 0x2800 */
/*  aip      : ORIGIN = 0x8c00e000, LENGTH = 0x2000 */
  p1ram    : ORIGIN = 0x8c010000, LENGTH = 0xff0000
}

SECTIONS
{
  .text.systemid :
  {
    KEEP(*(.text.systemid))
  } > systemid

  .text.toc :
  {
    KEEP(*(.text.toc))
  } > toc

  .text.sg_sec :
  {
    KEEP(*(.text.*sg_sec))
  } > sg_sec

  .text.sg_are :
  {
    KEEP(*(.text.*sg_arejp))
    KEEP(*(.text.*sg_areus))
    KEEP(*(.text.*sg_areec))
    KEEP(*(.text.*sg_are00))
    KEEP(*(.text.*sg_are01))
    KEEP(*(.text.*sg_are02))
    KEEP(*(.text.*sg_are03))
    KEEP(*(.text.*sg_are04))
  } > sg_are

  .text.sg_ini :
  {
    KEEP(*(.text.*sg_ini))
    KEEP(*(.text.start))
    *(.text.startup.*)
    *(.text*)
    *(.rodata*)
    *(.bss*)
    *(.data*)
    . = ORIGIN(sg_ini) + LENGTH(sg_ini);
  } > sg_ini

/*
  .text.aip :
  {
    KEEP(*(.text.*aip))
    . = ORIGIN(aip) + LENGTH(aip);
  } > aip
  */

  /DISCARD/ :
  {
    *(.text)
    *(.data)
    *(.bss)
    *(.comment)
  }

  INCLUDE "debug.lds"
}

__stack_end = ORIGIN(p1ram) + LENGTH(p1ram);

__text_link_start = 0;
__text_link_end = 0;
__text_load_start = 0;

__data_link_start = 0;
__data_link_end = 0;
__data_load_start = 0;

__rodata_link_start = 0;
__rodata_link_end = 0;
__rodata_load_start = 0;

__ctors_link_start = 0;
__ctors_link_end = 0;

__bss_link_start = 0;
__bss_link_end = 0;

__vbr_link_start = 0;
__vbr_link_end = 0;
__vbr_load_end = 0;

INCLUDE "addresses.lds"

__send_buf = 0xac000020;
__recv_buf = 0xac004020;
