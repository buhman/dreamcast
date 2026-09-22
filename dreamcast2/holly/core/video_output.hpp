#pragma once

namespace holly::core::video_output {
  struct framebuffer {
    int px_width;
    int px_height;
    int bytes_per_pixel;

    framebuffer(int width, int height, int bytes_per_pixel)
      : px_width(width), px_height(height), bytes_per_pixel(bytes_per_pixel)
    {}

    inline static int div(int n, int d) {
      return (n + 32 - 1) / 32;
    }
    inline int tile_width() const {
      return div(px_width, 32);
    }
    inline int tile_height() const {
      return div(px_height, 32);
    }
  };

  void framebuffer_init(struct framebuffer const& framebuffer);
  void scaler_init();
  void spg_set_mode_640x480();
}
