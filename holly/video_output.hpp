#pragma once

#include <cstdint>

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

}
