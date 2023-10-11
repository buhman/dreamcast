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
    KEEP(*(.text.sg_sec))
  } > sg_sec

  .text.sg_are :
  {
    KEEP(*(.text.sg_arejp))
    KEEP(*(.text.sg_areus))
    KEEP(*(.text.sg_areec))
    KEEP(*(.text.sg_are00))
    KEEP(*(.text.sg_are01))
    KEEP(*(.text.sg_are02))
    KEEP(*(.text.sg_are03))
    KEEP(*(.text.sg_are04))
  } > sg_are

  .text.sg_ini :
  {
    KEEP(*(.text.sg_ini))
  } > sg_ini

  .text.aip :
  {
    KEEP(*(.text.aip))
  } > aip

  /DISCARD/ :
  {
    *(*)
  }
}
