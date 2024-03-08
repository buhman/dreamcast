#include <cstdint>

#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

#include "core_bits.hpp"
#include "ta_bits.hpp"
#include "holly.hpp"
#include "texture_memory_alloc.hpp"

#include "ta_fifo_polygon_converter.hpp"

void ta_polygon_converter_init(uint32_t opb_total_size, // for one tile, for all render passes
			       uint32_t ta_alloc,
			       uint32_t tile_width,   // in tile units (e.g: (640 / 32))
			       uint32_t tile_height)  // in tile units (e.g: (480 / 32))
{
  // opb_size is the total size of all OPBs for all passes
  const uint32_t ta_next_opb_offset = opb_total_size * tile_width * tile_height;

  holly.SOFTRESET = softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  holly.TA_GLOB_TILE_CLIP = ta_glob_tile_clip::tile_y_num(tile_height - 1)
                          | ta_glob_tile_clip::tile_x_num(tile_width  - 1);

  holly.TA_ALLOC_CTRL = ta_alloc_ctrl::opb_mode::increasing_addresses
                      | ta_alloc;

  holly.TA_ISP_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));
  holly.TA_ISP_LIMIT = (offsetof (struct texture_memory_alloc, object_list)); // the end of isp_tsp_parameters
  holly.TA_OL_BASE = (offsetof (struct texture_memory_alloc, object_list));
  holly.TA_OL_LIMIT = (offsetof (struct texture_memory_alloc, _res0)); // the end of the object_list
  holly.TA_NEXT_OPB_INIT = (offsetof (struct texture_memory_alloc, object_list))
                         + ta_next_opb_offset;

  holly.TA_LIST_INIT = ta_list_init::list_init;

  uint32_t _dummy_read = holly.TA_LIST_INIT;
  (void)_dummy_read;
}

void ta_polygon_converter_cont(uint32_t ol_base_offset,
			       uint32_t ta_alloc)
{
  holly.TA_ALLOC_CTRL = ta_alloc_ctrl::opb_mode::increasing_addresses
                      | ta_alloc;
  holly.TA_OL_BASE = (offsetof (struct texture_memory_alloc, object_list))
                   + ol_base_offset;

  holly.TA_LIST_CONT = ta_list_cont::list_cont;

  uint32_t _dummy_read = holly.TA_LIST_CONT;
  (void)_dummy_read;
}

void ta_polygon_converter_transfer(volatile uint32_t * buf, uint32_t size)
{
  /* wait for previous transfer to complete (if any) */
  //while ((system.C2DST & C2DST__STATUS) != 0);  /* 1 == transfer is in progress */
  /* CHCR2__TE can be reset even if there is no in-progress transfer. Despite
     DCDBSysArc990907E's claim, it does not appear to be useful to check TE. */
  //while ((sh7091.DMAC.CHCR2 & CHCR2__TE) == 0); /* 1 == all transfers are completed */

  /* "Write back" the entire buffer to physical memory.

     This is required on real hardware if CCR__CB is enabled, and `buf` is in a
     cacheable area (e.g: system memory access via 0x8c00_0000).*/
  for (uint32_t i = 0; i < size / 32; i++) {
    asm volatile ("ocbwb @%0"
		  :                          // output
		  : "r" (&buf[(i * 32) / 4]) // input
		  );
  }

  // this dummy read appears to be required on real hardware.
  volatile uint32_t _dummy = sh7091.DMAC.CHCR2;
  (void)_dummy;

  using namespace dmac;

  /* start a new CH2-DMA transfer from "system memory" to "TA FIFO polygon converter" */
  sh7091.DMAC.CHCR2 = 0; /* disable DMA channel */
  sh7091.DMAC.SAR2 = reinterpret_cast<uint32_t>(&buf[0]);  /* start address, must be aligned to a CHCHR__TS-sized (32-byte) boundary */
  sh7091.DMAC.DMATCR2 = dmatcr::transfer_count(size / 32); /* transfer count, in CHCHR__TS-sized (32-byte) units */
  sh7091.DMAC.CHCR2 = chcr::dm::destination_address_fixed
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0010) /* external request, single address mode;
					                   external address space → external device */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                    | chcr::ts::_32_byte         /* transfer size */
                    | chcr::de::channel_operation_enabled;

  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */
  system.C2DSTAT = c2dstat::texture_memory_start_address(0x10000000); /* CH2-DMA destination address */
  system.C2DLEN  = c2dlen::transfer_length(size);         /* CH2-DMA length (must be a multiple of 32) */
  system.C2DST   = 1;          /* CH2-DMA start (an 'external' request from SH7091's perspective) */

  // wait for ch2-dma completion
  while ((system.ISTNRM & istnrm::end_of_dma_ch2_dma) == 0);
  // reset ch2-dma interrupt status
  system.ISTNRM = istnrm::end_of_dma_ch2_dma;
}

void ta_wait_opaque_list()
{
  while ((system.ISTNRM & istnrm::end_of_transferring_opaque_list) == 0);

  system.ISTNRM = istnrm::end_of_transferring_opaque_list;
}

void ta_wait_opaque_modifier_volume_list()
{
  while ((system.ISTNRM & istnrm::end_of_transferring_opaque_modifier_volume_list) == 0);

  system.ISTNRM = istnrm::end_of_transferring_opaque_modifier_volume_list;
}

void ta_wait_translucent_list()
{
  while ((system.ISTNRM & istnrm::end_of_transferring_translucent_list) == 0);

  system.ISTNRM = istnrm::end_of_transferring_translucent_list;
}

void ta_wait_punch_through_list()
{
  while ((system.ISTNRM & istnrm::end_of_transferring_punch_through_list) == 0);

  system.ISTNRM = istnrm::end_of_transferring_punch_through_list;
}
