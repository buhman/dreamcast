#pragma once

#include <stdint.h>

namespace framebuffer {

  constexpr inline int div32(int n)
  {
    return (n + 32 - 1) / 32;
  }

  struct framebuffer {
    int px_width;
    int px_height;

    framebuffer()
      : px_width(0), px_height(0)
    {}

    framebuffer(int width, int height)
      : px_width(width), px_height(height)
    {}

    int tile_width() {
      return div32(px_width);
    }
    int tile_height() {
      return div32(px_height);
    }
  };

  const int bytes_per_pixel = 2;

  extern struct framebuffer framebuffer;

  void init(int px_width,
            int px_height,
            uint32_t framebuffer_start);
  void scaler_init();
  void spg_set_mode_720x480_vga();
  void spg_set_mode_640x480_vga();
  void spg_set_mode_320x240_ntsc_ni();
}
