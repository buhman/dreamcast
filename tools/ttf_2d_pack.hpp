#pragma once

#include <cstdint>

struct window_curve_ix {
  struct {
    uint16_t width;
    uint16_t height;
  } window;
  uint32_t max_z_curve_ix;
};

struct window_curve_ix
pack_all(struct rect * rects, const uint32_t num_rects);
