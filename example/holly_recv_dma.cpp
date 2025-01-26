#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "memorymap.hpp"

static void dma(uint32_t source, uint32_t destination, uint32_t length)
{
  using namespace dmac;

  sh7091.DMAC.CHCR1 = 0;

  sh7091.DMAC.SAR1 = source;
  sh7091.DMAC.DAR1 = destination;
  sh7091.DMAC.DMATCR1 = length & 0x00ff'ffff;

  sh7091.DMAC.CHCR1 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0100) /* external address space → external address space */
    | chcr::tm::cycle_burst_mode /* transmit mode */
    //| chcr::tm::cycle_steal_mode /* transmit mode */
                    | chcr::ts::_32_byte           /* transfer size */
                  //| chcr::ie::interrupt_request_generated
                    | chcr::de::channel_operation_enabled;
}

static uint32_t buf[256] __attribute__((aligned(32)));

void main()
{
  for (int i = 0; i < 256; i++) {
    buf[i] = 0;
    texture_memory32[i] = (1 << 31) | i;
  }

  for (uint32_t i = 0; i < (sizeof (buf)) / 32; i++) {
    uint32_t address = (uint32_t)&buf[0];
    asm volatile ("ocbp @%0"
                  :                                                              // output
                  : "r" (address + (i * 32)) // input
                  );
  }

  sh7091.DMAC.DMAOR = 0;

  serial::integer<uint32_t>(sh7091.DMAC.DMAOR);

  serial::integer<uint32_t>((uint32_t)&buf[0]);

  dma((uint32_t)&texture_memory32[0], (uint32_t)&buf[0], (sizeof (buf)));

  uint32_t last_dar = sh7091.DMAC.DAR1;
  uint32_t count = 0;
  while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0) {
    uint32_t dar = sh7091.DMAC.DAR1;
    if (dar == last_dar)
      count += 1;
    else
      count = 0;
    if (count > 100)
      return;
  };
  serial::integer<uint32_t>(sh7091.DMAC.DMAOR);

  for (uint32_t i = 0; i < (sizeof (buf)) / 32; i++) {
    uint32_t address = (uint32_t)&buf[i * 32];
    asm volatile ("ocbi @%0"
                  :                                                              // output
                  : "r" (address) // input
                  );
  }

  serial::string("buf:\n");
  for (int i = 0; i < 256; i++) {
    serial::integer<uint32_t>(buf[i]);
  }
  serial::string("return\n");
  serial::string("return\n");
  serial::string("return\n");
  serial::string("return\n");
}
