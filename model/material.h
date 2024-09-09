#pragma once

#include <stdint.h>

struct pixel_descriptor {
  uint8_t * start;
  int32_t size;
  int32_t vram_offset; // offset into vram texture address space

  int16_t width;
  int16_t height;
};

struct palette_descriptor {
  uint8_t * start;
  int32_t size;
  int32_t vram_offset; // offset into vram palette address space

  int32_t palette_size;
};

struct material_descriptor {
  struct pixel_descriptor pixel;
  struct palette_descriptor palette;
};
