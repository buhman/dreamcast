OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0xac020000, LENGTH = 16M
}
SECTIONS
{
  . = 0xac020000;

  .text ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.text.start))
    *(.text.startup.*)
    *(.text.*)
    *(.text)
  } > p1ram

  .data ALIGN(4) : SUBALIGN(4)
  {
    *(.data)
    *(.data.*)
  } > p1ram

  .rodata ALIGN(4) : SUBALIGN(4)
  {
    *(.rodata)
    *(.rodata.*)
  } > p1ram

  .ctors ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.ctors))
    KEEP(*(.ctors.*))
  } > p1ram

  .bss ALIGN(4) (NOLOAD) : SUBALIGN(4)
  {
    *(.bss)
    *(.bss.*)
  } > p1ram

  .text.p2ram ALIGN(4) : SUBALIGN(4)
  {
    *(.p2ram)
    *(.p2ram.*)
  } > p1ram

  __p1ram_end = .;

  /DISCARD/ :
  {
    *(.debug*)
    *(.comment*)
    *(.rela*)
  }

  __bss_link_start = ADDR(.bss);
  __bss_link_end = ADDR(.bss) + SIZEOF(.bss);

  __ctors_link_start = ADDR(.ctors);
  __ctors_link_end = ADDR(.ctors) + SIZEOF(.ctors);
}

__p1ram_start = ORIGIN(p1ram);
__p1ram_end = ORIGIN(p1ram) + LENGTH(p1ram);

INCLUDE "addresses.lds"
