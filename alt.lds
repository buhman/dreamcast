OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0x8c020000, LENGTH = 0xff0000
  p2ram : ORIGIN = 0xac020000, LENGTH = 0xff0000
}
INCLUDE "common.lds"
