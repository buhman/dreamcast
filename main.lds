OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0x8c010000, LENGTH = 16M
  p2ram : ORIGIN = 0xac010000, LENGTH = 16M
}
SECTIONS
{
  . = 0x8c010000;

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

  __p1ram_end = .;

  . = 0xac010000 + __p1ram_end - __p1ram_start;

  .text.p2ram ALIGN(4) : SUBALIGN(4)
  {
    *(.p2ram)
    *(.p2ram.*)
  } > p2ram AT>p1ram

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

SH7091_IC_A = 0xf0000000;
SH7091_IC_D = 0xf1000000;
SH7091_OC_A = 0xf4000000;
SH7091_OC_D = 0xf5000000;
SH7091 = 0xff000000;
HOLLY = 0xa05f8000;
__p1ram_start = ORIGIN(p1ram);
__p1ram_end = ORIGIN(p1ram) + LENGTH(p1ram);
