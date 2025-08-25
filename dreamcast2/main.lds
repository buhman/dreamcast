OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0x8c010000, LENGTH = 0xff0000
  p2ram : ORIGIN = 0xac010000, LENGTH = 0xff0000
}

INCLUDE "common.lds"

__stack_end = ORIGIN(p1ram) + LENGTH(p1ram) - 0x4000;
