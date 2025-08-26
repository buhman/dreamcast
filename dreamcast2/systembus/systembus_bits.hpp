#pragma once

#include <cstdint>

namespace systembus {
  namespace c2dstat {
    constexpr inline uint32_t texture_memory_start_address(uint32_t num) { return (num & 0x13ffffe0) << 0; }
  }

  namespace c2dlen {
    constexpr inline uint32_t transfer_length(uint32_t num) { return (num & 0xffffe0) << 0; }
  }

  namespace c2dst {
    constexpr uint32_t start = 1 << 0;
  }

  namespace istnrm {
    constexpr uint32_t end_of_transferring_punch_through_list = 1 << 21;
    constexpr uint32_t end_of_dma_sort_dma = 1 << 20;
    constexpr uint32_t end_of_dma_ch2_dma = 1 << 19;
    constexpr uint32_t end_of_dma_dev_dma = 1 << 18;
    constexpr uint32_t end_of_dma_ext_dma2 = 1 << 17;
    constexpr uint32_t end_of_dma_ext_dma1 = 1 << 16;
    constexpr uint32_t end_of_dma_aica_dma = 1 << 15;
    constexpr uint32_t end_of_dma_gd_dma = 1 << 14;
    constexpr uint32_t maple_v_blank_over = 1 << 13;
    constexpr uint32_t end_of_dma_maple_dma = 1 << 12;
    constexpr uint32_t end_of_dma_pvr_dma = 1 << 11;
    constexpr uint32_t end_of_transferring_translucent_modifier_volume_list = 1 << 10;
    constexpr uint32_t end_of_transferring_translucent_list = 1 << 9;
    constexpr uint32_t end_of_transferring_opaque_modifier_volume_list = 1 << 8;
    constexpr uint32_t end_of_transferring_opaque_list = 1 << 7;
    constexpr uint32_t end_of_transferring_yuv = 1 << 6;
    constexpr uint32_t h_blank_in = 1 << 5;
    constexpr uint32_t v_blank_out = 1 << 4;
    constexpr uint32_t v_blank_in = 1 << 3;
    constexpr uint32_t end_of_render_tsp = 1 << 2;
    constexpr uint32_t end_of_render_isp = 1 << 1;
    constexpr uint32_t end_of_render_video = 1 << 0;
  }

  namespace isterr {
    constexpr uint32_t sh4__if_access_inhibited_area = 1 << 31;
    constexpr uint32_t ddt__if_sort_dma_command_error = 1 << 28;
    constexpr uint32_t g2__time_out_in_cpu_access = 1 << 27;
    constexpr uint32_t g2__dev_dma_time_out = 1 << 26;
    constexpr uint32_t g2__ext_dma2_time_out = 1 << 25;
    constexpr uint32_t g2__ext_dma1_time_out = 1 << 24;
    constexpr uint32_t g2__aica_dma_time_out = 1 << 23;
    constexpr uint32_t g2__dev_dma_over_run = 1 << 22;
    constexpr uint32_t g2__ext_dma2_over_run = 1 << 21;
    constexpr uint32_t g2__ext_dma1_over_run = 1 << 20;
    constexpr uint32_t g2__aica_dma_over_run = 1 << 19;
    constexpr uint32_t g2__dev_dma_illegal_address_set = 1 << 18;
    constexpr uint32_t g2__ext_dma2_illegal_address_set = 1 << 17;
    constexpr uint32_t g2__ext_dma1_illegal_address_set = 1 << 16;
    constexpr uint32_t g2__aica_dma_illegal_address_set = 1 << 15;
    constexpr uint32_t g1__rom_flash_access_at_gd_dma = 1 << 14;
    constexpr uint32_t g1__gd_dma_over_run = 1 << 13;
    constexpr uint32_t g1__illegal_address_set = 1 << 12;
    constexpr uint32_t maple__illegal_command = 1 << 11;
    constexpr uint32_t maple__write_fifo_over_flow = 1 << 10;
    constexpr uint32_t maple__dma_over_run = 1 << 9;
    constexpr uint32_t maple__illegal_address_set = 1 << 8;
    constexpr uint32_t pvrif__dma_over_run = 1 << 7;
    constexpr uint32_t pvrif__illegal_address_set = 1 << 6;
    constexpr uint32_t ta__fifo_overflow = 1 << 5;
    constexpr uint32_t ta__illegal_parameter = 1 << 4;
    constexpr uint32_t ta__object_list_pointer_overflow = 1 << 3;
    constexpr uint32_t ta__isp_tsp_parameter_overflow = 1 << 2;
    constexpr uint32_t render__hazard_processing_of_strip_buffer = 1 << 1;
    constexpr uint32_t render__isp_out_of_cache = 1 << 0;
  }

  namespace ffst {
    constexpr inline uint32_t holly_cpu_if_block_internal_write_buffer(uint32_t reg) { return (reg >> 5) & 0x1; }
    constexpr inline uint32_t holly_g2_if_block_internal_write_buffer(uint32_t reg) { return (reg >> 4) & 0x1; }
    constexpr inline uint32_t aica_internal_write_buffer(uint32_t reg) { return (reg >> 0) & 0x1; }
  }

  namespace istext {
    constexpr uint32_t external_device = 1 << 3;
    constexpr uint32_t modem = 1 << 2;
    constexpr uint32_t aica = 1 << 1;
    constexpr uint32_t gdrom = 1 << 0;
  }

}
