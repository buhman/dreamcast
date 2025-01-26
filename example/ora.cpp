#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"

#include "memorymap.hpp"

static void dma(uint32_t source, uint32_t destination, uint32_t transfers)
{
  using namespace dmac;

  sh7091.DMAC.CHCR1 = 0;

  sh7091.DMAC.SAR1 = source;
  sh7091.DMAC.DAR1 = destination;
  sh7091.DMAC.DMATCR1 = transfers & 0x00ff'ffff;

  sh7091.DMAC.CHCR1 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0101) /* auto request, external address space → on-chip peripheral module */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                  //| chcr::tm::cycle_steal_mode /* transmit mode */
                    | chcr::ts::_32_bit           /* transfer size */
                  //| chcr::ie::interrupt_request_generated
                    | chcr::de::channel_operation_enabled;
}

static void dma_init()
{
  using namespace dmac;

  sh7091.DMAC.CHCR0 = 0;
  sh7091.DMAC.CHCR1 = 0;
  sh7091.DMAC.CHCR2 = 0;
  sh7091.DMAC.CHCR3 = 0;
  sh7091.DMAC.DMAOR = dmaor::ddt::normal_dma_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */

}

void main()
{
  sh7091.CCN.CCR |= ccn::ccr::ora::_8_kbytes_used_as_cache_8_kbytes_used_as_ram;

  dma_init();

  // from entry 128 to entry 255 and from entry 384 to entry 511 of the OC are to be used as RAM
  uint32_t * oc_a = &sh7091_oc_d[128 * 32 / 4]; // 1024 words
  uint32_t * oc_b = &sh7091_oc_d[384 * 32 / 4]; // 1024 words

  for (int i = 0; i < 256; i++) {
    oc_a[i] = 0;
    texture_memory32[i] = (1 << 31) | i;
  }

  serial::string("tm: ");
  serial::integer<uint32_t>((uint32_t)&texture_memory32[0]);
  serial::string("oc_a: ");
  serial::integer<uint32_t>((uint32_t)&oc_a[0]);

  serial::string("dmaor: ");
  serial::integer<uint32_t>(sh7091.DMAC.DMAOR);

  uint32_t transfers = 64 / 4;
  dma((uint32_t)&texture_memory32[0], (uint32_t)&oc_a[0], transfers);

  serial::string("sar: ");
  serial::integer<uint32_t>(sh7091.DMAC.SAR1);
  serial::string("dar: ");
  serial::integer<uint32_t>(sh7091.DMAC.DAR1);

  uint32_t last_dar = sh7091.DMAC.DAR1;
  uint32_t count = 0;
  while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0) {
    uint32_t dar = sh7091.DMAC.DAR1;
    if (dar == last_dar)
      count += 1;
    else
      count = 0;
    if (count > 10)
      goto return_main;
    serial::integer<uint32_t>(sh7091.DMAC.DMAOR);
  }

  serial::string("dmaor: ");
  serial::integer<uint32_t>(sh7091.DMAC.DMAOR);
  serial::string("buf:\n");
  for (int i = 0; i < 64; i++) {
    serial::integer<uint32_t>(oc_a[i]);
  }

 return_main:
  serial::string("return\n");
  serial::string("return\n");
  serial::string("return\n");
  serial::string("return\n");
}
