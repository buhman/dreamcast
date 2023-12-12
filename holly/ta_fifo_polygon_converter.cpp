#include <cstdint>

#include "core_bits.hpp"
#include "ta_bits.hpp"
#include "../holly.hpp"
#include "../systembus.hpp"
#include "../systembus_bits.hpp"
#include "../sh7091.hpp"
#include "../sh7091_bits.hpp"

#include "texture_memory_alloc.hpp"

#include "ta_fifo_polygon_converter.hpp"

void ta_polygon_converter_init()
{
  holly.SOFTRESET = softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  holly.TA_GLOB_TILE_CLIP = ta_glob_tile_clip::tile_y_num((480 / 32) - 1)
                          | ta_glob_tile_clip::tile_x_num((640 / 32) - 1);

  holly.TA_ALLOC_CTRL = ta_alloc_ctrl::opb_mode::increasing_addresses
		      | ta_alloc_ctrl::pt_opb::no_list
		      | ta_alloc_ctrl::tm_opb::no_list
		      | ta_alloc_ctrl::t_opb::no_list
		      | ta_alloc_ctrl::om_opb::no_list
		      | ta_alloc_ctrl::o_opb::_16x4byte;

  holly.TA_ISP_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));
  holly.TA_ISP_LIMIT = (offsetof (struct texture_memory_alloc, object_list)); // the end of isp_tsp_parameters
  holly.TA_OL_BASE = (offsetof (struct texture_memory_alloc, object_list));
  holly.TA_OL_LIMIT = (offsetof (struct texture_memory_alloc, _res0)); // the end of the object_list
  holly.TA_NEXT_OPB_INIT = (offsetof (struct texture_memory_alloc, object_list));
  //holly.TA_NEXT_OPB_INIT = (offsetof (struct texture_memory_alloc, object_list))
  //                       + (640 / 32) * (320 / 32) * 16 * 4;

  holly.TA_LIST_INIT = ta_list_init::list_init;

  volatile uint32_t _dummy_read = holly.TA_LIST_INIT;
  (void)_dummy_read;
}

extern void serial_string(const char * s);

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

  /* start a new CH2-DMA transfer from "system memory" to "TA FIFO polygon converter" */
  sh7091.DMAC.CHCR2 = 0; /* disable DMA channel */
  sh7091.DMAC.SAR2 = reinterpret_cast<uint32_t>(&buf[0]); /* start address, must be aligned to a CHCHR__TS-sized (32-byte) boundary */
  sh7091.DMAC.DMATCR2 = DMATCR2__TRANSFER_COUNT(size / 32); /* transfer count, in CHCHR__TS-sized (32-byte) units */
  sh7091.DMAC.CHCR2 = CHCR2__DM__DESTINATION_ADDRESS_FIXED
                    | CHCR2__SM__SOURCE_ADDRESS_INCREMENTED
		    | CHCR2__RS(0b0010) /* external request, single address mode;
					   external address space → external device */
                    | CHCR2__TM__BURST_MODE /* transmit mode */
                    | CHCR2__TS__32_BYTE    /* transfer size */
                    | CHCR2__DE;            /* DMAC (channel) enable */

  sh7091.DMAC.DMAOR = DMAOR__DDT                 /* on-demand data transfer mode */
                    | DMAOR__PR__CH2_CH0_CH1_CH3 /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | DMAOR__DME;                /* DMAC master enable */
  system.C2DSTAT = C2DSTAT__ADDRESS(0x10000000); /* CH2-DMA destination address */
  system.C2DLEN  = CD2LEN__LENGTH(size) ; /* CH2-DMA length (must be a multiple of 32) */
  system.C2DST   = 1;          /* CH2-DMA start (an 'external' request from SH7091's perspective) */

  // wait for CH2-DMA completion
  while ((system.ISTNRM & ISTNRM__END_OF_DMA_CH2_DMA) == 0);
  // reset CH2-DMA interrupt status
  system.ISTNRM = ISTNRM__END_OF_DMA_CH2_DMA;
}

void ta_wait_opaque_list()
{
  while ((system.ISTNRM & ISTNRM__END_OF_TRANSFERRING_OPAQUE_LIST) == 0);

  system.ISTNRM = ISTNRM__END_OF_TRANSFERRING_OPAQUE_LIST;
}
