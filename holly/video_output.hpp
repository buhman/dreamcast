#pragma once

#include <cstdint>

#include "core_bits.hpp"

namespace video_output {

struct mode {
  const uint32_t fb_r_ctrl;
  const uint32_t spg_load;
  const uint32_t spg_hblank;
  const uint32_t spg_vblank;
  const uint32_t spg_width;
  const uint32_t spg_control;
  const uint32_t vo_startx;
  const uint32_t vo_starty;
  const uint32_t vo_control;
  const uint32_t spg_hblank_int;
  const uint32_t spg_vblank_int;
};

extern const struct mode vga;
extern const struct mode ntsc_ni;
extern const struct mode ntsc_i;
extern const struct mode pal_ni;
extern const struct mode pal_i;

struct framebuffer_resolution {
  uint32_t width;
  uint32_t height;
};

void set_framebuffer_resolution(const uint32_t x_size, const uint32_t y_size);
void set_mode(const struct mode& mode);
uint32_t get_cable_type();
framebuffer_resolution set_mode_by_cable_type(uint32_t cable_type);
void set_mode_vga();
void reset_sdram();

}
