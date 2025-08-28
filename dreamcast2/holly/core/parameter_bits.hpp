#pragma once

#include <cstdint>

namespace holly::core::parameter {
  namespace isp_tsp_instruction_word {
    namespace depth_compare_mode {
      constexpr uint32_t never = 0 << 29;
      constexpr uint32_t less = 1 << 29;
      constexpr uint32_t equal = 2 << 29;
      constexpr uint32_t less_or_equal = 3 << 29;
      constexpr uint32_t greater = 4 << 29;
      constexpr uint32_t greater_or_equal = 5 << 29;
      constexpr uint32_t not_equal = 6 << 29;
      constexpr uint32_t always = 7 << 29;

      constexpr uint32_t bit_mask = 0x7 << 29;
    }

    namespace volume_instruction {
      constexpr uint32_t normal_polygon = 0 << 29;
      constexpr uint32_t inside_last_polygon = 1 << 29;
      constexpr uint32_t outside_last_polygon = 2 << 29;

      constexpr uint32_t bit_mask = 0x7 << 29;
    }

    namespace culling_mode {
      constexpr uint32_t no_culling = 0 << 27;
      constexpr uint32_t cull_if_small = 1 << 27;
      constexpr uint32_t cull_if_negative = 2 << 27;
      constexpr uint32_t cull_if_positive = 3 << 27;

      constexpr uint32_t bit_mask = 0x3 << 27;
    }

    constexpr uint32_t z_write_disable = 1 << 26;
    constexpr uint32_t texture = 1 << 25;
    constexpr uint32_t offset = 1 << 24;
    constexpr uint32_t gouraud_shading = 1 << 23;
    constexpr uint32_t _16bit_uv = 1 << 22;
    constexpr uint32_t cache_bypass = 1 << 21;
    constexpr uint32_t dcalc_ctrl = 1 << 20;
  }

  namespace tsp_instruction_word {
    namespace src_alpha_instr {
      constexpr uint32_t zero = 0 << 29;
      constexpr uint32_t one = 1 << 29;
      constexpr uint32_t other_color = 2 << 29;
      constexpr uint32_t inverse_other_color = 3 << 29;
      constexpr uint32_t src_alpha = 4 << 29;
      constexpr uint32_t inverse_src_alpha = 5 << 29;
      constexpr uint32_t dst_alpha = 6 << 29;
      constexpr uint32_t inverse_dst_alpha = 7 << 29;

      constexpr uint32_t bit_mask = 0x7 << 29;
    }

    namespace dst_alpha_instr {
      constexpr uint32_t zero = 0 << 26;
      constexpr uint32_t one = 1 << 26;
      constexpr uint32_t other_color = 2 << 26;
      constexpr uint32_t inverse_other_color = 3 << 26;
      constexpr uint32_t src_alpha = 4 << 26;
      constexpr uint32_t inverse_src_alpha = 5 << 26;
      constexpr uint32_t dst_alpha = 6 << 26;
      constexpr uint32_t inverse_dst_alpha = 7 << 26;

      constexpr uint32_t bit_mask = 0x7 << 26;
    }

    namespace src_select {
      constexpr uint32_t primary_accumulation_buffer = 0 << 25;
      constexpr uint32_t secondary_accumulation_buffer = 1 << 25;

      constexpr uint32_t bit_mask = 0x1 << 25;
    }

    namespace dst_select {
      constexpr uint32_t primary_accumulation_buffer = 0 << 24;
      constexpr uint32_t secondary_accumulation_buffer = 1 << 24;

      constexpr uint32_t bit_mask = 0x1 << 24;
    }

    namespace fog_control {
      constexpr uint32_t look_up_table = 0b00 << 22;
      constexpr uint32_t per_vertex = 0b01 << 22;
      constexpr uint32_t no_fog = 0b10 << 22;
      constexpr uint32_t look_up_table_mode_2 = 0b11 << 22;

      constexpr uint32_t bit_mask = 0x3 << 22;
    }

    constexpr uint32_t color_clamp = 1 << 21;
    constexpr uint32_t use_alpha = 1 << 20;
    constexpr uint32_t ignore_texture_alpha = 1 << 19;

    namespace flip_uv {
      constexpr uint32_t none = 0 << 17;
      constexpr uint32_t v = 1 << 17;
      constexpr uint32_t u = 2 << 17;
      constexpr uint32_t uv = 3 << 17;

      constexpr uint32_t bit_mask = 0x3 << 17;
    }

    namespace clamp_uv {
      constexpr uint32_t none = 0 << 15;
      constexpr uint32_t v = 1 << 15;
      constexpr uint32_t u = 2 << 15;
      constexpr uint32_t uv = 3 << 15;

      constexpr uint32_t bit_mask = 0x3 << 15;
    }

    namespace filter_mode {
      constexpr uint32_t point_sampled = 0b00 << 13;
      constexpr uint32_t bilinear_filter = 0b01 << 13;
      constexpr uint32_t trilinear_pass_a = 0b10 << 13;
      constexpr uint32_t trilinear_pass_b = 0b11 << 13;

      constexpr uint32_t bit_mask = 0x3 << 13;
    }

    constexpr uint32_t super_sample_texture = 1 << 12;
    constexpr inline uint32_t mip_map_d_adjust(uint32_t num) { return (num & 0xf) << 8; }

    namespace texture_shading_instruction {
      constexpr uint32_t decal = 0 << 6;
      constexpr uint32_t modulate = 1 << 6;
      constexpr uint32_t decal_alpha = 2 << 6;
      constexpr uint32_t modulate_alpha = 3 << 6;

      constexpr uint32_t bit_mask = 0x3 << 6;
    }

    namespace texture_u_size {
      constexpr uint32_t _8 = 0 << 3;
      constexpr uint32_t _16 = 1 << 3;
      constexpr uint32_t _32 = 2 << 3;
      constexpr uint32_t _64 = 3 << 3;
      constexpr uint32_t _128 = 4 << 3;
      constexpr uint32_t _256 = 5 << 3;
      constexpr uint32_t _512 = 6 << 3;
      constexpr uint32_t _1024 = 7 << 3;

      constexpr uint32_t bit_mask = 0x7 << 3;
    }

    namespace texture_v_size {
      constexpr uint32_t _8 = 0 << 0;
      constexpr uint32_t _16 = 1 << 0;
      constexpr uint32_t _32 = 2 << 0;
      constexpr uint32_t _64 = 3 << 0;
      constexpr uint32_t _128 = 4 << 0;
      constexpr uint32_t _256 = 5 << 0;
      constexpr uint32_t _512 = 6 << 0;
      constexpr uint32_t _1024 = 7 << 0;

      constexpr uint32_t bit_mask = 0x7 << 0;
    }
  }

  namespace texture_control_word {
    constexpr uint32_t mip_mapped = 1 << 31;
    constexpr uint32_t vq_compressed = 1 << 30;

    namespace pixel_format {
      constexpr uint32_t argb1555 = 0 << 27;
      constexpr uint32_t rgb565 = 1 << 27;
      constexpr uint32_t argb4444 = 2 << 27;
      constexpr uint32_t yuv422 = 3 << 27;
      constexpr uint32_t bump_map = 4 << 27;
      constexpr uint32_t palette_4bpp = 5 << 27;
      constexpr uint32_t palette_8bpp = 6 << 27;

      constexpr uint32_t bit_mask = 0x7 << 27;
    }

    namespace scan_order {
      constexpr uint32_t twiddled = 0 << 26;
      constexpr uint32_t non_twiddled = 1 << 26;

      constexpr uint32_t bit_mask = 0x1 << 26;
    }

    constexpr uint32_t stride_select = 1 << 25;
    constexpr inline uint32_t palette_selector4(uint32_t num) { return (num & 0x3f) << 21; }
    constexpr inline uint32_t palette_selector8(uint32_t num) { return (num & 0x3) << 25; }
    constexpr inline uint32_t texture_address(uint32_t num) { return (num & 0x1fffff) << 0; }
  }

}
