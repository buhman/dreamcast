#include <cstdint>

#include "sh7091.hpp"
#include "sh7091_bits.hpp"

#include "cache.hpp"
#include "load.hpp"

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

void main()
{
  cache_init();

  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  load_init();

  while (1) {
    while ((sh7091.SCIF.SCFSR2 & SCFSR2__TDFE) == 0) {
      // wait
    }
    while ((sh7091.SCIF.SCFDR2 & 0b11111) > 0) {
      uint8_t c = sh7091.SCIF.SCFRDR2;
      load_recv(c);
    }
    sh7091.SCIF.SCFSR2 = sh7091.SCIF.SCFSR2 & (~SCFSR2__RDF);
  }
}
