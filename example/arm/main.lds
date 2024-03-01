OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
MEMORY
{
  ram     : ORIGIN = 0x00000000, LENGTH = 0x1ff000
  buffers : ORIGIN = 0x001ff000, LENGTH = 0x1000
}
SECTIONS
{
  . = ORIGIN(ram);

  .text ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.text.start))
    *(.text.startup.*)
    *(.text.*)
    *(.text)
  } > ram

  .data ALIGN(4) : SUBALIGN(4)
  {
    *(.data)
    *(.data.*)
  } > ram

  .rodata ALIGN(4) : SUBALIGN(4)
  {
    *(.rodata)
    *(.rodata.*)
  } > ram

  .ctors ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.ctors))
    KEEP(*(.ctors.*))
  } > ram

  .bss ALIGN(4) (NOLOAD) : SUBALIGN(4)
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > ram

  . = ORIGIN(buffers);

  .buffers ALIGN(4) (NOLOAD) : SUBALIGN(4)
  {
    *(.buffers)
    *(.buffers.*)
  } > buffers

  INCLUDE "../../debug.lds"
  INCLUDE "arm.lds"
}

__ram_start = ORIGIN(ram);
__ram_end = ORIGIN(ram) + LENGTH(ram);

__bss_link_start = ADDR(.bss);
__bss_link_end = ADDR(.bss) + SIZEOF(.bss);

__ctors_link_start = ADDR(.ctors);
__ctors_link_end = ADDR(.ctors) + SIZEOF(.ctors);

INCLUDE "addresses.lds"
