OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
MEMORY
{
  p1ram : ORIGIN = 0x8c010000, LENGTH = 0xff0000
  p2ram : ORIGIN = 0xac010000, LENGTH = 0xff0000
}
__stack_reservation = 0x1000;
INCLUDE "common.lds"
