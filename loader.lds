OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0x8c010000, LENGTH = 0xff0000
  p2ram : ORIGIN = 0xac010000, LENGTH = 0xff0000
  ldram : ORIGIN = 0x8cfff000, LENGTH = 0x1000
}
SECTIONS
{
  . = ORIGIN(p2ram);

  .text.startup ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.text.start))
    *(.text.startup.*)
    . = ALIGN(4);
  } > p2ram AT> p2ram

  .ctors ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.ctors))
    KEEP(*(.ctors.*))
    . = ALIGN(4);
  } > p2ram AT> p2ram

  . = ORIGIN(ldram);

  .text ALIGN(4) : SUBALIGN(4)
  {
    *(.text.*)
    *(.text)
    . = ALIGN(4);
  } > ldram AT> p2ram

  .data ALIGN(4) : SUBALIGN(4)
  {
    *(.data)
    *(.data.*)
    . = ALIGN(4);
  } > ldram AT> p2ram

  .rodata ALIGN(4) : SUBALIGN(4)
  {
    *(.rodata)
    *(.rodata.*)
    . = ALIGN(4);
  } > ldram AT> p2ram

  .bss ALIGN(4) (NOLOAD) : SUBALIGN(4)
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
  } > ldram

  .text.vbr ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.vbr.100))
    . = ALIGN(0x300);
    KEEP(*(.vbr.400))
    . = ALIGN(0x200);
    KEEP(*(.vbr.600))
  } > p1ram

  INCLUDE "debug.lds"
}
__stack_reservation = 0x0000;
INCLUDE "symbols.lds"
INCLUDE "addresses.lds"
