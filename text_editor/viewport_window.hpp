#pragma once

#include <cstdint>

#include "font/font.h"

struct viewport_window {
  int32_t first_line;
  struct {
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
  } box;
};

static inline void viewport_init_fullscreen(viewport_window& window, const font * font)
{
  window.first_line = 0;
  window.box.x0 = 10;
  window.box.y0 = 20;
  window.box.x1 = 640 - 10;
  window.box.y1 = 480 - (font->face_metrics.height >> 6) * 2;
}
