#pragma once

#include <cstdint>

#include "../float_uint32.hpp"

namespace ta_ol_base {
  constexpr uint32_t base_address(uint32_t num) { return (num & 0xffffe0) << 0; }
}

namespace ta_isp_base {
  constexpr uint32_t base_address(uint32_t num) { return (num & 0xfffffc) << 0; }
}

namespace ta_ol_limit {
  constexpr uint32_t limit_address(uint32_t num) { return (num & 0xffffe0) << 0; }
}

namespace ta_isp_limit {
  constexpr uint32_t limit_address(uint32_t num) { return (num & 0xfffffc) << 0; }
}

namespace ta_next_opb {
  constexpr uint32_t address(uint32_t num) { return (num & 0xffffe0) << 0; }
}

namespace ta_itp_current {
  constexpr uint32_t address(uint32_t reg) { return (reg >> 0) & 0xffffff; }
}

namespace ta_glob_tile_clip {
  constexpr uint32_t tile_y_num(uint32_t num) { return (num & 0xf) << 16; }
  constexpr uint32_t tile_x_num(uint32_t num) { return (num & 0x1f) << 0; }
}

namespace ta_alloc_ctrl {
  namespace opb_mode {
    constexpr uint32_t increasing_addresses = 0 << 20;
    constexpr uint32_t decreasing_addresses = 1 << 20;

    constexpr uint32_t bit_mask = 0x1 << 20;
  }

  namespace pt_opb {
    constexpr uint32_t no_list = 0 << 16;
    constexpr uint32_t _8x4byte = 1 << 16;
    constexpr uint32_t _16x4byte = 2 << 16;
    constexpr uint32_t _32x4byte = 3 << 16;

    constexpr uint32_t bit_mask = 0x3 << 16;
  }

  namespace tm_opb {
    constexpr uint32_t no_list = 0 << 12;
    constexpr uint32_t _8x4byte = 1 << 12;
    constexpr uint32_t _16x4byte = 2 << 12;
    constexpr uint32_t _32x4byte = 3 << 12;

    constexpr uint32_t bit_mask = 0x3 << 12;
  }

  namespace t_opb {
    constexpr uint32_t no_list = 0 << 8;
    constexpr uint32_t _8x4byte = 1 << 8;
    constexpr uint32_t _16x4byte = 2 << 8;
    constexpr uint32_t _32x4byte = 3 << 8;

    constexpr uint32_t bit_mask = 0x3 << 8;
  }

  namespace om_opb {
    constexpr uint32_t no_list = 0 << 4;
    constexpr uint32_t _8x4byte = 1 << 4;
    constexpr uint32_t _16x4byte = 2 << 4;
    constexpr uint32_t _32x4byte = 3 << 4;

    constexpr uint32_t bit_mask = 0x3 << 4;
  }

  namespace o_opb {
    constexpr uint32_t no_list = 0 << 0;
    constexpr uint32_t _8x4byte = 1 << 0;
    constexpr uint32_t _16x4byte = 2 << 0;
    constexpr uint32_t _32x4byte = 3 << 0;

    constexpr uint32_t bit_mask = 0x3 << 0;
  }
}

namespace ta_list_init {
  constexpr uint32_t list_init = 1 << 31;
}

namespace ta_yuv_tex_base {
  constexpr uint32_t base_address(uint32_t num) { return (num & 0xfffff8) << 0; }
}

namespace ta_yuv_tex_ctrl {
  namespace yuv_form {
    constexpr uint32_t yuv420 = 0 << 24;
    constexpr uint32_t yuv422 = 1 << 24;

    constexpr uint32_t bit_mask = 0x1 << 24;
  }

  namespace yuv_tex {
    constexpr uint32_t one_texture = 0 << 16;
    constexpr uint32_t multiple_textures = 1 << 16;

    constexpr uint32_t bit_mask = 0x1 << 16;
  }

  constexpr uint32_t yuv_v_size(uint32_t num) { return (num & 0x3f) << 8; }
  constexpr uint32_t yuv_u_size(uint32_t num) { return (num & 0x3f) << 0; }
}

namespace ta_yuv_tex_cnt {
  constexpr uint32_t yuv_num(uint32_t reg) { return (reg >> 0) & 0x1fff; }
}

namespace ta_list_cont {
  constexpr uint32_t list_cont = 1 << 31;
}

namespace ta_next_opb_init {
  constexpr uint32_t address(uint32_t num) { return (num & 0xffffe0) << 0; }
}

namespace ta_ol_pointers {
  constexpr uint32_t entry(uint32_t reg) { return (reg >> 31) & 0x1; }
  constexpr uint32_t sprite(uint32_t reg) { return (reg >> 30) & 0x1; }
  constexpr uint32_t triangle(uint32_t reg) { return (reg >> 29) & 0x1; }
  constexpr uint32_t number_of_triangles_quads(uint32_t reg) { return (reg >> 25) & 0xf; }
  constexpr uint32_t shadow(uint32_t reg) { return (reg >> 24) & 0x1; }
  constexpr uint32_t pointer_address(uint32_t reg) { return (reg >> 2) & 0x3fffff; }
  constexpr uint32_t skip(uint32_t reg) { return (reg >> 0) & 0x3; }
}

