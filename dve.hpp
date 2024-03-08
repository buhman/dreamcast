#pragma once

#include <cstdint>

namespace vreg {
  namespace output_mode {
    constexpr uint32_t vga = 0b00 << 0;
    constexpr uint32_t rgb = 0b10 << 0;
    constexpr uint32_t cvbs_yc = 0b11 << 0;

    constexpr uint32_t bit_mask = 0x3 << 0;
  }
}

namespace pdtra {
  namespace cable_type {
    constexpr uint32_t vga = 0b00 << 8;
    constexpr uint32_t rbg = 0b10 << 8;
    constexpr uint32_t cvbs_yc = 0b11 << 8;

    constexpr uint32_t bit_mask = 0x3 << 8;
  }

  namespace video_mode {
    constexpr uint32_t ntsc = 0b000 << 2;
    constexpr uint32_t pal = 0b001 << 2;
    constexpr uint32_t pal_m = 0b011 << 2;
    constexpr uint32_t pal_n = 0b101 << 2;
    constexpr uint32_t forced_ntsc_interlacing = 0b110 << 2;
    constexpr uint32_t forced_pal_interlacing = 0b111 << 2;

    constexpr uint32_t bit_mask = 0x7 << 2;
  }
}

