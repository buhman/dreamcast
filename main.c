#include <stdint.h>

#include "cache.h"
#include "load.h"

#include "sh7091.h"
#include "sh7091_bits.h"

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

void serial()
{
  SH7091.SCIF.SCSCR2 = 0;
  SH7091.SCIF.SCSMR2 = 0;
  SH7091.SCIF.SCBRR2 = 12;

#define SCFCR2__TFRST (1 << 2)
#define SCFCR2__RFRST (1 << 1)
  SH7091.SCIF.SCFCR2 = SCFCR2__TFRST | SCFCR2__RFRST;
  // tx/rx trigger on 1 byte
  SH7091.SCIF.SCFCR2 = 0;

  SH7091.SCIF.SCSPTR2 = 0;
  SH7091.SCIF.SCLSR2 = 0;

#define SCSCR2__TE (1 << 5)
#define SCSCR2__RE (1 << 4)
  SH7091.SCIF.SCSCR2 = SCSCR2__TE | SCSCR2__RE;
}

void main()
{
  cache_init();

  // clear BSS
  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  serial();

  load_init();

  while (1) {
#define SCFSR2__ER   (1 << 7) /* read error */
#define SCFSR2__TEND (1 << 6) /* transmit end */
#define SCFSR2__TFDE (1 << 5) /* transmit fifo data empty */
#define SCFSR2__BRK  (1 << 4) /* break detect */
#define SCFSR2__FER  (1 << 3) /* framing error */
#define SCFSR2__PER  (1 << 2) /* parity error */
#define SCFSR2__RDF  (1 << 1) /* receive FIFO data full */
#define SCFSR2__DR   (1 << 0) /* receive data ready */

    while ((SH7091.SCIF.SCFSR2 & SCFSR2__RDF) == 0) {
      // wait
    }
    while ((SH7091.SCIF.SCFDR2 & 0b11111) > 0) {
      uint8_t c = SH7091.SCIF.SCFRDR2;
      load_recv(c);
    }
    SH7091.SCIF.SCFSR2 = SH7091.SCIF.SCFSR2 & (~SCFSR2__RDF);
  }
}
