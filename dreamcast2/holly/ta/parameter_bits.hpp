#pragma once

#include <cstdint>

namespace holly::ta::parameter {
  namespace parameter_control_word {
    namespace para_type {
      constexpr uint32_t end_of_list = 0 << 29;
      constexpr uint32_t user_tile_clip = 1 << 29;
      constexpr uint32_t object_list_set = 2 << 29;
      constexpr uint32_t polygon_or_modifier_volume = 4 << 29;
      constexpr uint32_t sprite = 5 << 29;
      constexpr uint32_t vertex_parameter = 7 << 29;

      constexpr uint32_t bit_mask = 0x7 << 29;
    }

    constexpr uint32_t end_of_strip = 1 << 28;

    namespace list_type {
      constexpr uint32_t opaque = 0 << 24;
      constexpr uint32_t opaque_modifier_volume = 1 << 24;
      constexpr uint32_t translucent = 2 << 24;
      constexpr uint32_t translucent_modifier_volume = 3 << 24;
      constexpr uint32_t punch_through = 4 << 24;

      constexpr uint32_t bit_mask = 0x7 << 24;
    }

    constexpr uint32_t group_en = 1 << 23;

    namespace strip_len {
      constexpr uint32_t _1_strip = 0 << 18;
      constexpr uint32_t _2_strip = 1 << 18;
      constexpr uint32_t _4_strip = 2 << 18;
      constexpr uint32_t _6_strip = 3 << 18;

      constexpr uint32_t bit_mask = 0x3 << 18;
    }

    namespace user_clip {
      constexpr uint32_t disabled = 0 << 16;
      constexpr uint32_t inside_enable = 2 << 16;
      constexpr uint32_t outside_enable = 3 << 16;

      constexpr uint32_t bit_mask = 0x3 << 16;
    }

    namespace polygon_volume {
      constexpr uint32_t no_volume = 0b00 << 6;
      constexpr uint32_t intensity_volume = 0b10 << 6;
      constexpr uint32_t two_volumes = 0b11 << 6;

      constexpr uint32_t bit_mask = 0x3 << 6;
    }

    namespace modifier_volume {
      constexpr uint32_t last_in_volume = 0b01 << 6;

      constexpr uint32_t bit_mask = 0x3 << 6;
    }

    namespace col_type {
      constexpr uint32_t packed_color = 0 << 4;
      constexpr uint32_t floating_color = 1 << 4;
      constexpr uint32_t intensity_mode_1 = 2 << 4;
      constexpr uint32_t intensity_mode_2 = 3 << 4;

      constexpr uint32_t bit_mask = 0x3 << 4;
    }

    constexpr uint32_t texture = 1 << 3;
    constexpr uint32_t offset = 1 << 2;
    constexpr uint32_t gouraud = 1 << 1;
    constexpr uint32_t _16bit_uv = 1 << 0;
  }

}
