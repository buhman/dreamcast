#include <cstdint>

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/cache.hpp"

#include "serial_load.hpp"

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

void main()
{
  cache::init();

  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  load_init();

  while (1) {
    using namespace scif;

    while ((sh7091.SCIF.SCFSR2 & scfsr2::tdfe::bit_mask) == 0) {
      // wait
    }
    while ((scfdr2::receive_data_bytes(sh7091.SCIF.SCFDR2)) > 0) {
      uint8_t c = sh7091.SCIF.SCFRDR2;
      load_recv(c);
    }
    sh7091.SCIF.SCFSR2 = sh7091.SCIF.SCFSR2 & (~scfsr2::rdf::bit_mask);
  }
}
