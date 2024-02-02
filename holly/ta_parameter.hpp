#pragma once

#include <cstdint>
#include <cstddef>

#include "../float_uint32.hpp"

namespace para_control {
  namespace para_type {
    constexpr uint32_t end_of_list                = 0 << 29;
    constexpr uint32_t user_tile_clip             = 1 << 29;
    constexpr uint32_t object_list_set            = 2 << 29;
    constexpr uint32_t polygon_or_modifier_volume = 4 << 29;
    constexpr uint32_t sprite                     = 5 << 29;
    constexpr uint32_t vertex_parameter           = 7 << 29;
  }

  constexpr uint32_t end_of_strip = 1 << 28;

  namespace list_type {
    constexpr uint32_t opaque = 0 << 24;
    constexpr uint32_t opaque_modifier_volume = 1 << 24;
    constexpr uint32_t translucent = 2 << 24;
    constexpr uint32_t translucent_modifier_volume = 3 << 24;
    constexpr uint32_t punch_through = 4 << 24;
  }
}

namespace group_control {
  constexpr uint32_t group_en = 1 << 23;

  namespace strip_len {
    constexpr uint32_t _1_strip = 0 << 18;
    constexpr uint32_t _2_strip = 1 << 18;
    constexpr uint32_t _4_strip = 2 << 18;
    constexpr uint32_t _6_strip = 3 << 18;
  }

  namespace user_clip {
    constexpr uint32_t disabled = 0 << 16;
    constexpr uint32_t inside_enable = 2 << 16;
    constexpr uint32_t outside_enable = 3 << 16;
  }
}

namespace obj_control {
  constexpr uint32_t shadow = 1 << 7;
  namespace volume {
    namespace polygon {
      constexpr uint32_t with_two_volumes = 1 << 6;
    }
    namespace modifier_volume {
      constexpr uint32_t last_in_volume = 1 << 6;
    }
  }

  namespace col_type {
    constexpr uint32_t packed_color     = 0 << 4;
    constexpr uint32_t floating_color   = 1 << 4;
    constexpr uint32_t intensity_mode_1 = 2 << 4;
    constexpr uint32_t intensity_mode_2 = 3 << 4;
  }

  constexpr uint32_t texture = 1 << 3;
  constexpr uint32_t offset = 1 << 2;
  constexpr uint32_t gouraud = 1 << 1;
  constexpr uint32_t _16bit_uv = 1 << 0;
}

constexpr uint32_t polygon_vertex_parameter_control_word(const bool end_of_strip)
{
  return para_control::para_type::vertex_parameter
       | (end_of_strip ? para_control::end_of_strip : 0);
}

constexpr uint32_t modifier_volume_vertex_parameter_control_word()
{
  // DCDBSysArc990907E page 212 indirectly suggests the end_of_strip bit should
  // always be set.
  return para_control::para_type::vertex_parameter
       | para_control::end_of_strip;
}

struct ta_parameter_writer {
  uint32_t * buf;
  uint32_t offset; // in bytes

  ta_parameter_writer(uint32_t * buf)
    : buf(buf), offset(0)
  { }

  template <typename T>
  inline T& append()
  {
    T& t = *reinterpret_cast<T *>(&buf[offset / 4]);
    offset += (sizeof (T));
    return t;
  }
};

constexpr inline uint32_t uv_16bit(float u, float v)
{
  uint32_t * ui = (reinterpret_cast<uint32_t *>(&u));
  uint32_t * vi = (reinterpret_cast<uint32_t *>(&v));
  uint32_t u_half = ((*ui) >> 16) & 0xffff;
  uint32_t v_half = ((*vi) >> 16) & 0xffff;
  return (u_half << 16) | (v_half << 0);
}
