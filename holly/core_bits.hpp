#pragma once

#include <cstdint>

#include "../float_uint32.hpp"

namespace id {
  constexpr uint32_t device_id(uint32_t reg) { return (reg >> 16) & 0xffff; }
  constexpr uint32_t vendor_id(uint32_t reg) { return (reg >> 0) & 0xffff; }
}

namespace revision {
  constexpr uint32_t chip_revision(uint32_t reg) { return (reg >> 0) & 0xffff; }
}

namespace softreset {
  constexpr uint32_t sdram_if_soft_reset = 1 << 2;
  constexpr uint32_t pipeline_soft_reset = 1 << 1;
  constexpr uint32_t ta_soft_reset = 1 << 0;
}

namespace startrender {
  constexpr uint32_t start_render = 1 << 0;
}

namespace test_select {
  constexpr uint32_t diagdb_data(uint32_t reg) { return (reg >> 5) & 0x1f; }
  constexpr uint32_t diagda_data(uint32_t reg) { return (reg >> 0) & 0x1f; }
}

namespace param_base {
  constexpr uint32_t base_address(uint32_t num) { return (num & 0xf00000) << 0; }
}

namespace region_base {
  constexpr uint32_t base_address(uint32_t num) { return (num & 0xfffffc) << 0; }
}

namespace span_sort_cfg {
  constexpr uint32_t cache_bypass = 1 << 16;
  constexpr uint32_t offset_sort_enable = 1 << 8;
  constexpr uint32_t span_sort_enable = 1 << 0;
}

namespace vo_border_col {
  constexpr uint32_t chroma(uint32_t num) { return (num & 0x1) << 24; }
  constexpr uint32_t red(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t green(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t blue(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fb_r_ctrl {
  namespace vclk_div {
    constexpr uint32_t pclk_vclk_2 = 0 << 23;
    constexpr uint32_t pclk_vclk_1 = 1 << 23;
    
    constexpr uint32_t bit_mask = 0x1 << 23;
  }
  
  constexpr uint32_t fb_strip_buf_en = 1 << 22;
  constexpr uint32_t fb_stripsize(uint32_t num) { return (num & 0x3e) << 16; }
  constexpr uint32_t fb_chroma_threshold(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t fb_concat(uint32_t num) { return (num & 0x3) << 4; }
  
  namespace fb_depth {
    constexpr uint32_t _0555_rgb_16bit = 0 << 2;
    constexpr uint32_t _0565_rgb_16bit = 1 << 2;
    constexpr uint32_t _888_rgb_24bit_packed = 2 << 2;
    constexpr uint32_t _0888_rgb_32bit = 3 << 2;
    
    constexpr uint32_t bit_mask = 0x3 << 2;
  }
  
  constexpr uint32_t fb_line_double = 1 << 1;
  constexpr uint32_t fb_enable = 1 << 0;
}

namespace fb_w_ctrl {
  constexpr uint32_t fb_alpha_threshold(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t fb_kval(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t fb_dither = 1 << 3;
  
  namespace fb_packmode {
    constexpr uint32_t _0555_krgb_16bit = 0 << 0;
    constexpr uint32_t _565_rgb_16bit = 1 << 0;
    constexpr uint32_t _4444_argb_16bit = 2 << 0;
    constexpr uint32_t _1555_argb_16bit = 3 << 0;
    constexpr uint32_t _888_rgb_24bit_packed = 4 << 0;
    constexpr uint32_t _0888_krgb_32bit = 5 << 0;
    constexpr uint32_t _8888_argb_32bit = 6 << 0;
    
    constexpr uint32_t bit_mask = 0x7 << 0;
  }
}

namespace fb_w_linestride {
  constexpr uint32_t fb_line_stride(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fb_r_sof1 {
  constexpr uint32_t frame_buffer_read_address_frame_1(uint32_t num) { return (num & 0xfffffc) << 0; }
}

namespace fb_r_sof2 {
  constexpr uint32_t frame_buffer_read_address_frame_2(uint32_t num) { return (num & 0xfffffc) << 0; }
}

namespace fb_r_size {
  constexpr uint32_t fb_modulus(uint32_t num) { return (num & 0x3ff) << 20; }
  constexpr uint32_t fb_y_size(uint32_t num) { return (num & 0x3ff) << 10; }
  constexpr uint32_t fb_x_size(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace fb_w_sof1 {
  constexpr uint32_t frame_buffer_write_address_frame_1(uint32_t num) { return (num & 0x1fffffc) << 0; }
}

namespace fb_w_sof2 {
  constexpr uint32_t frame_buffer_write_address_frame_2(uint32_t num) { return (num & 0x1fffffc) << 0; }
}

namespace fb_x_clip {
  constexpr uint32_t fb_x_clip_max(uint32_t num) { return (num & 0x7ff) << 16; }
  constexpr uint32_t fb_x_clip_min(uint32_t num) { return (num & 0x7ff) << 0; }
}

namespace fb_y_clip {
  constexpr uint32_t fb_y_clip_max(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t fb_y_clip_min(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace fpu_shad_scale {
  namespace simple_shadow_enable {
    constexpr uint32_t parameter_selection_volume_mode = 0 << 8;
    constexpr uint32_t intensity_volume_mode = 1 << 8;
    
    constexpr uint32_t bit_mask = 0x1 << 8;
  }
  
  constexpr uint32_t scale_factor_for_shadows(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fpu_cull_val {
  inline uint32_t culling_comparison_value(float num) { return _i(__builtin_fabsf(num));; }
}

namespace fpu_param_cfg {
  namespace region_header_type {
    constexpr uint32_t type_1 = 0 << 21;
    constexpr uint32_t type_2 = 1 << 21;
    
    constexpr uint32_t bit_mask = 0x1 << 21;
  }
  
  constexpr uint32_t tsp_parameter_burst_threshold(uint32_t num) { return (num & 0x3f) << 14; }
  constexpr uint32_t isp_parameter_burst_threshold(uint32_t num) { return (num & 0x3f) << 8; }
  constexpr uint32_t pointer_burst_size(uint32_t num) { return (num & 0xf) << 4; }
  constexpr uint32_t pointer_first_burst_size(uint32_t num) { return (num & 0xf) << 0; }
}

namespace half_offset {
  namespace tsp_texel_sampling_position {
    constexpr uint32_t top_left = 1 << 2;
    constexpr uint32_t center = 1 << 2;
    
    constexpr uint32_t bit_mask = 0x1 << 2;
  }
  
  namespace tsp_pixel_sampling_position {
    constexpr uint32_t top_left = 1 << 1;
    constexpr uint32_t center = 1 << 1;
    
    constexpr uint32_t bit_mask = 0x1 << 1;
  }
  
  namespace fpu_pixel_sampling_position {
    constexpr uint32_t top_left = 1 << 0;
    constexpr uint32_t center = 1 << 0;
    
    constexpr uint32_t bit_mask = 0x1 << 0;
  }
}

namespace fpu_perp_val {
  inline uint32_t perpendicular_triangle_compare(float num) { return _i(__builtin_fabsf(num));; }
}

namespace isp_backgnd_d {
  inline uint32_t background_plane_depth(float num) { return _i(num) & 0xfffffff0; }
}

namespace isp_backgnd_t {
  constexpr uint32_t cache_bypass = 1 << 28;
  constexpr uint32_t shadow = 1 << 27;
  constexpr uint32_t skip(uint32_t num) { return (num & 0x7) << 24; }
  constexpr uint32_t tag_address(uint32_t num) { return (num & 0x1fffff) << 3; }
  constexpr uint32_t tag_offset(uint32_t num) { return (num & 0x7) << 0; }
}

namespace isp_feed_cfg {
  constexpr uint32_t cache_size_for_translucency(uint32_t num) { return (num & 0x3ff) << 14; }
  constexpr uint32_t punch_through_chunk_size(uint32_t num) { return (num & 0x3ff) << 4; }
  constexpr uint32_t discard_mode = 1 << 3;
  constexpr uint32_t pre_sort_mode = 1 << 0;
}

namespace sdram_refresh {
  constexpr uint32_t refresh_counter_value(uint32_t num) { return (num & 0xff) << 0; }
}

namespace sdram_arb_cfg {
  namespace override_value {
    constexpr uint32_t priority_only = 0x0 << 18;
    constexpr uint32_t rendered_data = 0x1 << 18;
    constexpr uint32_t texture_vq_index = 0x2 << 18;
    constexpr uint32_t texture_normal_data_and_vq_codebook = 0x3 << 18;
    constexpr uint32_t tile_accelerator_isp_tsp_data = 0x4 << 18;
    constexpr uint32_t tile_accelerator_pointers = 0x5 << 18;
    constexpr uint32_t sh4 = 0x6 << 18;
    constexpr uint32_t tsp_parameters = 0x7 << 18;
    constexpr uint32_t tsp_region_data = 0x8 << 18;
    constexpr uint32_t isp_pointer_data = 0x9 << 18;
    constexpr uint32_t isp_parameters = 0xa << 18;
    constexpr uint32_t crt_controller = 0xb << 18;
    
    constexpr uint32_t bit_mask = 0xf << 18;
  }
  
  namespace arbiter_priority_control {
    constexpr uint32_t priority_arbitration_only = 0x0 << 16;
    constexpr uint32_t override_value_field = 0x1 << 16;
    constexpr uint32_t round_robin_counter = 0x2 << 16;
    
    constexpr uint32_t bit_mask = 0x3 << 16;
  }
  
  constexpr uint32_t arbiter_crt_page_break_latency_count_value(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t arbiter_page_break_latency_count_value(uint32_t num) { return (num & 0xff) << 0; }
}

namespace sdram_cfg {
  constexpr uint32_t read_command_to_returned_data_delay(uint32_t num) { return (num & 0x7) << 26; }
  constexpr uint32_t cas_latency_value(uint32_t num) { return (num & 0x7) << 23; }
  constexpr uint32_t activate_to_activate_period(uint32_t num) { return (num & 0x3) << 21; }
  constexpr uint32_t read_to_write_period(uint32_t num) { return (num & 0x7) << 18; }
  constexpr uint32_t refresh_to_activate_period(uint32_t num) { return (num & 0xf) << 14; }
  constexpr uint32_t pre_charge_to_activate_period(uint32_t num) { return (num & 0x3) << 10; }
  constexpr uint32_t activate_to_pre_charge_period(uint32_t num) { return (num & 0xf) << 6; }
  constexpr uint32_t activate_to_read_write_command_period(uint32_t num) { return (num & 0x3) << 4; }
  constexpr uint32_t write_to_pre_charge_period(uint32_t num) { return (num & 0x3) << 2; }
  constexpr uint32_t read_to_pre_charge_period(uint32_t num) { return (num & 0x3) << 0; }
}

namespace fog_col_ram {
  constexpr uint32_t red(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t green(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t blue(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fog_col_vert {
  constexpr uint32_t red(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t green(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t blue(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fog_density {
  constexpr uint32_t fog_scale_mantissa(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t fog_scale_exponent(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fog_clamp_max {
  constexpr uint32_t alpha(uint32_t num) { return (num & 0xff) << 24; }
  constexpr uint32_t red(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t green(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t blue(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fog_clamp_min {
  constexpr uint32_t alpha(uint32_t num) { return (num & 0xff) << 24; }
  constexpr uint32_t red(uint32_t num) { return (num & 0xff) << 16; }
  constexpr uint32_t green(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t blue(uint32_t num) { return (num & 0xff) << 0; }
}

namespace spg_trigger_pos {
  constexpr uint32_t trigger_v_count(uint32_t reg) { return (reg >> 16) & 0x3ff; }
  constexpr uint32_t trigger_h_count(uint32_t reg) { return (reg >> 0) & 0x3ff; }
}

namespace spg_hblank_int {
  constexpr uint32_t hblank_in_interrupt(uint32_t reg) { return (reg >> 16) & 0x3ff; }
  
  namespace hblank_int_mode {
    constexpr uint32_t output_equal_line_comp_val = 0x0 << 12;
    constexpr uint32_t output_every_line_comp_val = 0x1 << 12;
    constexpr uint32_t output_every_line = 0x2 << 12;
    
    constexpr uint32_t bit_mask = 0x3 << 12;
  }
  
  constexpr uint32_t line_comp_val(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace spg_vblank_int {
  constexpr uint32_t vblank_out_interrupt_line_number(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t vblank_in_interrupt_line_number(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace spg_control {
  namespace csync_on_h {
    constexpr uint32_t hsync = 0 << 9;
    constexpr uint32_t csync = 1 << 9;
    
    constexpr uint32_t bit_mask = 0x1 << 9;
  }
  
  namespace sync_direction {
    constexpr uint32_t input = 0 << 8;
    constexpr uint32_t output = 1 << 8;
    
    constexpr uint32_t bit_mask = 0x1 << 8;
  }
  
  constexpr uint32_t pal = 1 << 7;
  constexpr uint32_t ntsc = 1 << 6;
  constexpr uint32_t force_field2 = 1 << 5;
  constexpr uint32_t interlace = 1 << 4;
  constexpr uint32_t spg_lock = 1 << 3;
  
  namespace mcsync_pol {
    constexpr uint32_t active_low = 0 << 2;
    constexpr uint32_t active_high = 1 << 2;
    
    constexpr uint32_t bit_mask = 0x1 << 2;
  }
  
  namespace mvsync_pol {
    constexpr uint32_t active_low = 0 << 1;
    constexpr uint32_t active_high = 1 << 1;
    
    constexpr uint32_t bit_mask = 0x1 << 1;
  }
  
  namespace mhsync_pol {
    constexpr uint32_t active_low = 0 << 0;
    constexpr uint32_t active_high = 1 << 0;
    
    constexpr uint32_t bit_mask = 0x1 << 0;
  }
}

namespace spg_hblank {
  constexpr uint32_t hbend(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t hbstart(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace spg_load {
  constexpr uint32_t vcount(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t hcount(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace spg_vblank {
  constexpr uint32_t vbend(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t vbstart(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace spg_width {
  constexpr uint32_t eqwidth(uint32_t num) { return (num & 0x3ff) << 22; }
  constexpr uint32_t bpwidth(uint32_t num) { return (num & 0x3ff) << 12; }
  constexpr uint32_t vswidth(uint32_t num) { return (num & 0xf) << 8; }
  constexpr uint32_t hswidth(uint32_t num) { return (num & 0x7f) << 0; }
}

namespace text_control {
  namespace code_book_endian {
    constexpr uint32_t little_endian = 0 << 17;
    constexpr uint32_t big_endian = 1 << 17;
    
    constexpr uint32_t bit_mask = 0x1 << 17;
  }
  
  namespace index_endian {
    constexpr uint32_t little_endian = 0 << 16;
    constexpr uint32_t big_endian = 1 << 16;
    
    constexpr uint32_t bit_mask = 0x1 << 16;
  }
  
  constexpr uint32_t bank_bit(uint32_t num) { return (num & 0x1f) << 8; }
  constexpr uint32_t stride(uint32_t num) { return (num & 0x1f) << 0; }
}

namespace vo_control {
  constexpr uint32_t pclk_delay_reset = 1 << 21;
  constexpr uint32_t pclk_delay(uint32_t num) { return (num & 0xf) << 16; }
  constexpr uint32_t pixel_double = 1 << 8;
  
  namespace field_mode {
    constexpr uint32_t use_field_flag_from_spg = 0x0 << 4;
    constexpr uint32_t use_inverse_of_field_flag_from_spg = 0x1 << 4;
    constexpr uint32_t field_1_fixed = 0x2 << 4;
    constexpr uint32_t field_2_fixed = 0x3 << 4;
    constexpr uint32_t field_1_when_the_active_edges_of_hsync_and_vsync_match = 0x4 << 4;
    constexpr uint32_t field_2_when_the_active_edges_of_hsync_and_vsync_match = 0x5 << 4;
    constexpr uint32_t field_1_when_hsync_becomes_active_in_the_middle_of_the_vsync_active_edge = 0x6 << 4;
    constexpr uint32_t field_2_when_hsync_becomes_active_in_the_middle_of_the_vsync_active_edge = 0x7 << 4;
    constexpr uint32_t inverted_at_the_active_edge_of_vsync = 0x8 << 4;
    
    constexpr uint32_t bit_mask = 0xf << 4;
  }
  
  constexpr uint32_t blank_video = 1 << 3;
  
  namespace blank_pol {
    constexpr uint32_t active_low = 0 << 2;
    constexpr uint32_t active_high = 1 << 2;
    
    constexpr uint32_t bit_mask = 0x1 << 2;
  }
  
  namespace vsync_pol {
    constexpr uint32_t active_low = 0 << 1;
    constexpr uint32_t active_high = 1 << 1;
    
    constexpr uint32_t bit_mask = 0x1 << 1;
  }
  
  namespace hsync_pol {
    constexpr uint32_t active_low = 0 << 0;
    constexpr uint32_t active_high = 1 << 0;
    
    constexpr uint32_t bit_mask = 0x1 << 0;
  }
}

namespace vo_startx {
  constexpr uint32_t horizontal_start_position(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace vo_starty {
  constexpr uint32_t vertical_start_position_on_field_2(uint32_t num) { return (num & 0x3ff) << 16; }
  constexpr uint32_t vertical_start_position_on_field_1(uint32_t num) { return (num & 0x3ff) << 0; }
}

namespace scaler_ctl {
  namespace field_select {
    constexpr uint32_t field_1 = 0 << 18;
    constexpr uint32_t field_2 = 1 << 18;
    
    constexpr uint32_t bit_mask = 0x1 << 18;
  }
  
  constexpr uint32_t interlace = 1 << 17;
  constexpr uint32_t horizontal_scaling_enable = 1 << 16;
  constexpr uint32_t vertical_scale_factor(uint32_t num) { return (num & 0xffff) << 0; }
}

namespace pal_ram_ctrl {
  namespace pixel_format {
    constexpr uint32_t argb1555 = 0 << 0;
    constexpr uint32_t rgb565 = 1 << 0;
    constexpr uint32_t argb4444 = 2 << 0;
    constexpr uint32_t argb8888 = 3 << 0;
    
    constexpr uint32_t bit_mask = 0x3 << 0;
  }
}

namespace spg_status {
  constexpr uint32_t vsync(uint32_t reg) { return (reg >> 13) & 0x1; }
  constexpr uint32_t hsync(uint32_t reg) { return (reg >> 12) & 0x1; }
  constexpr uint32_t blank(uint32_t reg) { return (reg >> 11) & 0x1; }
  constexpr uint32_t fieldnum(uint32_t reg) { return (reg >> 10) & 0x1; }
  constexpr uint32_t scanline(uint32_t reg) { return (reg >> 0) & 0x3ff; }
}

namespace fb_burstctrl {
  constexpr uint32_t wr_burst(uint32_t num) { return (num & 0xf) << 16; }
  constexpr uint32_t vid_lat(uint32_t num) { return (num & 0x7f) << 8; }
  constexpr uint32_t vid_burst(uint32_t num) { return (num & 0x3f) << 0; }
}

namespace fb_c_sof {
  constexpr uint32_t frame_buffer_current_read_address(uint32_t reg) { return (reg >> 0) & 0xffffff; }
}

namespace y_coeff {
  constexpr uint32_t coefficient_1(uint32_t num) { return (num & 0xff) << 8; }
  constexpr uint32_t coefficient_0_2(uint32_t num) { return (num & 0xff) << 0; }
}

namespace pt_alpha_ref {
  constexpr uint32_t alpha_reference_for_punch_through(uint32_t num) { return (num & 0xff) << 0; }
}

namespace fog_table {
  constexpr uint32_t fog_table_data(uint32_t num) { return (num & 0xffff) << 0; }
}

namespace palette_ram {
  constexpr uint32_t palette_data(uint32_t num) { return (num & 0xffffffff) << 0; }
}

