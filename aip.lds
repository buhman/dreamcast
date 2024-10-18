OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")

MEMORY
{
  systemid (arx) : ORIGIN = 0x8c008000, LENGTH = 0x100
  toc      (arx) : ORIGIN = 0x8c008100, LENGTH = 0x200
  sg_sec   (arx) : ORIGIN = 0x8c008300, LENGTH = 0x3400
  sg_are   (arx) : ORIGIN = 0x8c00b700, LENGTH = 0x100
  sg_ini   (arx) : ORIGIN = 0x8c00b800, LENGTH = 0x2800
  aip      (arx) : ORIGIN = 0x8c00e000, LENGTH = 0x2000
}

SECTIONS
{
  .text.aip :
  {
    KEEP(*(.text.*aip))
    . = ALIGN(8192);
  } > aip

  /DISCARD/ :
  {
    *(.text)
    *(.data)
    *(.bss)
    *(.comment)
  }
}
