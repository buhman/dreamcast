SECTIONS
{
  . = ORIGIN(p2ram);

  .text ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.text.start))
    *(.text.startup.*)
  } > p2ram AT>p1ram

  . = ORIGIN(p1ram) + (. - ORIGIN(p2ram));

  .text ALIGN(4) : SUBALIGN(4)
  {
    *(.text.*)
    *(.text)
  } > p1ram

  .data ALIGN(4) : SUBALIGN(32)
  {
    *(.data)
    *(.data.*)
  } > p1ram

  .rodata ALIGN(4) :
  {
    *(.rodata)
    *(.rodata.*)
  } > p1ram

  .ctors ALIGN(4) : SUBALIGN(4)
  {
    KEEP(*(.ctors))
    KEEP(*(.ctors.*))
  } > p1ram

  .text.vbr ALIGN(4) : SUBALIGN(4)
  {
    __vbr100 = .;
    KEEP(*(.vbr.100))
    . = __vbr100 + 0x300;
    KEEP(*(.vbr.400))
    . = __vbr100 + 0x500;
    KEEP(*(.vbr.600))
  } > p1ram

  .bss ALIGN(4) (NOLOAD) :
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > p1ram

  INCLUDE "debug.lds"
}

INCLUDE "symbols.lds"
INCLUDE "addresses.lds"
