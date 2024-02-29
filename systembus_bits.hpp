#pragma once

#include <cstdint>

namespace c2dstat {
  constexpr uint32_t texture_memory_start_address(uint32_t num) { return (num & 0x13ffffe0) << 0; }
}

namespace c2dlen {
  constexpr uint32_t transfer_length(uint32_t num) { return (num & 0xffffe0) << 0; }
}

namespace c2dst {
  constexpr uint32_t start = 1 << 0;
}

namespace istnrm {
  constexpr uint32_t end_of_transferring_punch_through_list(uint32_t reg) { return (reg >> 21) & 0x1; }
  constexpr uint32_t end_of_dma_sort_dma(uint32_t reg) { return (reg >> 20) & 0x1; }
  constexpr uint32_t end_of_dma_ch2_dma(uint32_t reg) { return (reg >> 19) & 0x1; }
  constexpr uint32_t end_of_dma_dev_dma(uint32_t reg) { return (reg >> 18) & 0x1; }
  constexpr uint32_t end_of_dma_ext_dma2(uint32_t reg) { return (reg >> 17) & 0x1; }
  constexpr uint32_t end_of_dma_ext_dma1(uint32_t reg) { return (reg >> 16) & 0x1; }
  constexpr uint32_t end_of_dma_aica_dma(uint32_t reg) { return (reg >> 15) & 0x1; }
  constexpr uint32_t end_of_dma_gd_dma(uint32_t reg) { return (reg >> 14) & 0x1; }
  constexpr uint32_t maple_v_blank_over_interrupt(uint32_t reg) { return (reg >> 13) & 0x1; }
  constexpr uint32_t end_of_dma_maple_dma(uint32_t reg) { return (reg >> 12) & 0x1; }
  constexpr uint32_t end_of_dma_pvr_dma(uint32_t reg) { return (reg >> 11) & 0x1; }
  constexpr uint32_t end_of_transferring_translucent_modifier_volume_list(uint32_t reg) { return (reg >> 10) & 0x1; }
  constexpr uint32_t end_of_transferring_translucent_list(uint32_t reg) { return (reg >> 9) & 0x1; }
  constexpr uint32_t end_of_transferring_opaque_modifier_volume_list(uint32_t reg) { return (reg >> 8) & 0x1; }
  constexpr uint32_t end_of_transferring_opaque_list(uint32_t reg) { return (reg >> 7) & 0x1; }
  constexpr uint32_t end_of_transferring_yuv(uint32_t reg) { return (reg >> 6) & 0x1; }
  constexpr uint32_t h_blank_in_interrupt(uint32_t reg) { return (reg >> 5) & 0x1; }
  constexpr uint32_t v_blank_out_interrupt(uint32_t reg) { return (reg >> 4) & 0x1; }
  constexpr uint32_t v_blank_in_interrupt(uint32_t reg) { return (reg >> 3) & 0x1; }
  constexpr uint32_t end_of_render_tsp(uint32_t reg) { return (reg >> 2) & 0x1; }
  constexpr uint32_t end_of_render_isp(uint32_t reg) { return (reg >> 1) & 0x1; }
  constexpr uint32_t end_of_render_video(uint32_t reg) { return (reg >> 0) & 0x1; }
}

namespace isterr {
  constexpr uint32_t sh4__if_access_inhibited_area(uint32_t reg) { return (reg >> 31) & 0x1; }
  constexpr uint32_t ddt__if_sort_dma_command_error(uint32_t reg) { return (reg >> 28) & 0x1; }
  constexpr uint32_t g2__time_out_in_cpu_access(uint32_t reg) { return (reg >> 27) & 0x1; }
  constexpr uint32_t g2__dev_dma_time_out(uint32_t reg) { return (reg >> 26) & 0x1; }
  constexpr uint32_t g2__ext_dma2_time_out(uint32_t reg) { return (reg >> 25) & 0x1; }
  constexpr uint32_t g2__ext_dma1_time_out(uint32_t reg) { return (reg >> 24) & 0x1; }
  constexpr uint32_t g2__aica_dma_time_out(uint32_t reg) { return (reg >> 23) & 0x1; }
  constexpr uint32_t g2__dev_dma_over_run(uint32_t reg) { return (reg >> 22) & 0x1; }
  constexpr uint32_t g2__ext_dma2_over_run(uint32_t reg) { return (reg >> 21) & 0x1; }
  constexpr uint32_t g2__ext_dma1_over_run(uint32_t reg) { return (reg >> 20) & 0x1; }
  constexpr uint32_t g2__aica_dma_over_run(uint32_t reg) { return (reg >> 19) & 0x1; }
  constexpr uint32_t g2__dev_dma_illegal_address_set(uint32_t reg) { return (reg >> 18) & 0x1; }
  constexpr uint32_t g2__ext_dma2_illegal_address_set(uint32_t reg) { return (reg >> 17) & 0x1; }
  constexpr uint32_t g2__ext_dma1_illegal_address_set(uint32_t reg) { return (reg >> 16) & 0x1; }
  constexpr uint32_t g2__aica_dma_illegal_address_set(uint32_t reg) { return (reg >> 15) & 0x1; }
  constexpr uint32_t g1__rom_flash_access_at_gd_dma(uint32_t reg) { return (reg >> 14) & 0x1; }
  constexpr uint32_t g1__gd_dma_over_run(uint32_t reg) { return (reg >> 13) & 0x1; }
  constexpr uint32_t g1__illegal_address_set(uint32_t reg) { return (reg >> 12) & 0x1; }
  constexpr uint32_t maple__illegal_command(uint32_t reg) { return (reg >> 11) & 0x1; }
  constexpr uint32_t maple__write_fifo_over_flow(uint32_t reg) { return (reg >> 10) & 0x1; }
  constexpr uint32_t maple__dma_over_run(uint32_t reg) { return (reg >> 9) & 0x1; }
  constexpr uint32_t maple__illegal_address_set(uint32_t reg) { return (reg >> 8) & 0x1; }
  constexpr uint32_t pvrif__dma_over_run(uint32_t reg) { return (reg >> 7) & 0x1; }
  constexpr uint32_t pvrif__illegal_address_set(uint32_t reg) { return (reg >> 6) & 0x1; }
  constexpr uint32_t ta__fifo_overflow(uint32_t reg) { return (reg >> 5) & 0x1; }
  constexpr uint32_t ta__illegal_parameter(uint32_t reg) { return (reg >> 4) & 0x1; }
  constexpr uint32_t ta__object_list_pointer_overflow(uint32_t reg) { return (reg >> 3) & 0x1; }
  constexpr uint32_t ta__isp_tsp_parameter_overflow(uint32_t reg) { return (reg >> 2) & 0x1; }
  constexpr uint32_t render__hazard_processing_of_strip_buffer(uint32_t reg) { return (reg >> 1) & 0x1; }
  constexpr uint32_t render__isp_out_of_cache(uint32_t reg) { return (reg >> 0) & 0x1; }
}

namespace ffst {
  constexpr uint32_t holly_cpu_if_block_internal_write_buffer(uint32_t reg) { return (reg >> 5) & 0x1; }
  constexpr uint32_t holly_g2_if_block_internal_write_buffer(uint32_t reg) { return (reg >> 4) & 0x1; }
  constexpr uint32_t aica_internal_write_buffer(uint32_t reg) { return (reg >> 0) & 0x1; }
}

