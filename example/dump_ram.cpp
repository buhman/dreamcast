#include <cstdint>

#include "memorymap.hpp"

#include "sh7091/serial.hpp"

void dump_ram(const volatile uint32_t * mem, const uint32_t len)
{
  uint32_t sum = 0;
  for (uint32_t i = 0; i < len; i++) {
    uint8_t n = mem[i];
    sum += n;
    serial::hexlify(n);
    if ((i & 0xf) == 0xf)
      serial::character('\n');
  }
  serial::character('\n');
  serial::integer<uint32_t>(sum);
}

void main()
{
  // dump the first 64k of system memory
  dump_ram(system_memory, 0x10000);


  while (1);
}
