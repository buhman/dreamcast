#include "sh7091/sh7091.hpp"
#include "sh7091/serial.hpp"
#include "sh7091/serial_dma.hpp"

int main()
{
  serial::init(4);

  uint8_t buf[4] __attribute__((aligned(32))) = {0};
  uint8_t * bufi =
    reinterpret_cast<uint8_t *>(0xa000'0000
			      | reinterpret_cast<uint32_t>(buf));

  for (int i = 0; i < 10000; i++) {
    asm volatile ("nop;");
  }

  serial::integer(sh7091.DMAC.CHCR1);
  serial::integer(sh7091.DMAC.DMAOR);

  serial::string("DAR\n");

  sh7091.DMAC.DAR1 = reinterpret_cast<uint32_t>(&bufi[0]);

  serial::integer(sh7091.DMAC.DAR1);
  serial::integer(reinterpret_cast<uint32_t>(&bufi[0]));


  serial::string("wait\n");

  while (true) {
    serial::recv_dma(reinterpret_cast<uint32_t>(bufi), 4);

    while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0) {
    };

    for (uint32_t i = 0; i < 4; i++) {
      serial::hexlify(bufi[i]);
      serial::character(' ');
    }
    serial::string("end\n");
    serial::integer(sh7091.DMAC.DAR1);

    while ((sh7091.SCIF.SCFSR2 & 0x60) != 0x60) {}
    for (int i = 0; i < 1000000; i++) {
      asm volatile ("nop;");
    }
  }
}
