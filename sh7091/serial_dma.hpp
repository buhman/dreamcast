#pragma once

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

/*

  (4) Channels 1 and 3 can be used in the following manner:

  The allowed transfer data length (8/16/32 bits, 32 bytes) depends on the
  transfer area.  The 64-bit transfer data length specification is permitted only for
  system memory.

  Address mode: Only dual address mode is permitted.
  Transfer initiation request: SCIF interrupt and auto request are permitted.
  Bus mode: Only cycle steal mode is permitted.
  DMA end interrupt: Can be used.
*/

/*
  After the desired transfer conditions have been set in the DMA source address register (SAR),
  DMA destination address register (DAR), DMA transfer count register (DMATCR), DMA
  channel control register (CHCR), and DMA operation register (DMAOR), the DMAC transfers
  data according to the following procedure:

  1. The DMAC checks to see if transfer is enabled (DE = 1, DME = 1, TE = 0, NMIF = 0, AE = 0).

  2. When a transfer request is issued and transfer has been enabled, the DMAC transfers one
  transfer unit of data (determined by the setting of TS2–TS0). In auto-request mode, the transfer
  begins automatically when the DE bit and DME bit are set to 1. The DMATCR value is
  decremented by 1 for each transfer. The actual transfer flow depends on the address mode and
  bus mode.

  3. When the specified number of transfers have been completed (when the DMATCR value
  reaches 0), the transfer ends normally. If the IE bit in CHCR is set to 1 at this time, a DMTE
  interrupt request is sent to the CPU.

  4. If a DMAC address error or NMI interrupt occurs, the transfer is suspended. Transfer is also
  suspended when the DE bit in CHCR or the DME bit in DMAOR is cleared to 0. In the event
  of an address error, a DMAE interrupt request is forcibly sent to the CPU.
*/


namespace serial {

static void recv_dma(uint32_t destination_address, uint32_t length)
{
  using namespace dmac;
  /* Initial settings (SAR, DAR, DMATCR, CHCR, DMAOR) */

  // RS: SCFRDR2 (SCIF receive-data-full transfer request)
  // RS: 0b1011

  // SAR1 = SCFRDR2
  // DAR1 = (address)
  // DMATCR1 = (transfer count, 24 bit)
  sh7091.DMAC.CHCR1 = 0;

  sh7091.DMAC.SAR1 = reinterpret_cast<uint32_t>(&sh7091.SCIF.SCFRDR2);
  sh7091.DMAC.DAR1 = destination_address;
  sh7091.DMAC.DMATCR1 = length & 0x00ff'ffff;

  // SSA (only used for PCMCIA)
  // STC (only used for PCMCIA)
  // DSA (only used for PCMCIA)
  // DTC (only used for PCMCIA)
  // DS  (only used for DREQ requests)
  // RL  (only used for DRAK requests)
  // AM  (only used for DACK requests)
  // AL  (only used for DACK requests)
  // DM  incremented
  // SM  fixed
  // RS  SCIF
  // TM  cycle steal
  // TS  byte?
  // IE  interrupt enable
  // TE  transfer end (status bit)
  // DE  channel enable
  sh7091.DMAC.CHCR1 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_fixed
                    | chcr::rs::resource_select(0b1011) /* SCIF */
                    | chcr::tm::cycle_steal_mode /* transmit mode */
                    | chcr::ts::_8_bit           /* transfer size */
                  //| chcr::ie::interrupt_request_generated
                    | chcr::de::channel_operation_enabled;

  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */
}

}
