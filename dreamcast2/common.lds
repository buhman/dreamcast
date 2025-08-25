SECTIONS
{
  . = ORIGIN(p2ram);

  .text.start :
  {
    KEEP(*(.text.start))
    *(.text.startup.*)
  } > p2ram AT>p1ram

  . = ORIGIN(p1ram) + (. - ORIGIN(p2ram));

  .text :
  {
    *(.text.*)
    *(.text)
  } > p1ram

  .data :
  {
    *(.data)
    *(.data.*)
  } > p1ram

  .rodata :
  {
    *(.rodata)
    *(.rodata.*)
  } > p1ram

  .ctors ALIGN(4) :
  {
    KEEP(*(.ctors))
    KEEP(*(.ctors.*))
  } > p1ram

  .text.vbr :
  {
    __vbr100 = .;
    KEEP(*(.vbr.100))
    . = __vbr100 + 0x300;
    KEEP(*(.vbr.400))
    . = __vbr100 + 0x500;
    KEEP(*(.vbr.600))
  } > p1ram

  .bss ALIGN(32) (NOLOAD) :
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > p1ram

  INCLUDE "debug.lds"
}

INCLUDE "symbols.lds"
INCLUDE "addresses.lds"
