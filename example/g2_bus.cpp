#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

constexpr uint32_t patterns[] = {
  0x11223344,
  0x55667788,
  0x99aabbcc,
  0xddeeff00,
};

int main()
{
  serial::init(0);

  uint32_t i = 0;
  uint8_t j = 0;

  uint32_t * buf = (uint32_t *)(0xa0000000 | 0x14000000);

  while (1) {
    buf[j] = patterns[i & 3];
    i++;

    uint32_t ffst = system.FFST;
    while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
            | ffst::holly_g2_if_block_internal_write_buffer(ffst)
            | ffst::aica_internal_write_buffer(ffst)) {
      ffst = system.FFST;
    };

    j++;

    serial::string("ISTERR:\n");
    serial::integer<uint32_t>(system.ISTERR);
    //serial::integer<uint32_t>(system.ISTNRM);
    system.ISTERR = system.ISTERR;

    for (int k = 0; k < 1000000; k++) {
      asm volatile ("nop");
    }
  }
}
