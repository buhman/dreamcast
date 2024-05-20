// this file is designed to be platform-agnostic
#pragma once

#include <cstdint>

// metrics are 26.6 fixed point
struct glyph_metrics {
  int32_t horiBearingX;
  int32_t horiBearingY;
  int32_t horiAdvance;
} __attribute__ ((packed));

static_assert((sizeof (glyph_metrics)) == ((sizeof (int32_t)) * 3));

struct glyph_bitmap {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
} __attribute__ ((packed));

static_assert((sizeof (glyph_bitmap)) == ((sizeof (uint16_t)) * 4));

struct glyph {
  glyph_bitmap bitmap;
  glyph_metrics metrics;
} __attribute__ ((packed));

static_assert((sizeof (glyph)) == ((sizeof (glyph_bitmap)) + (sizeof (glyph_metrics))));

struct font {
  uint32_t first_char_code;
  uint32_t last_char_code;
  struct face_metrics {
    int32_t height; // 26.6 fixed point
    int32_t max_advance; // 26.6 fixed point
  } face_metrics;
  uint16_t glyph_count;
  uint16_t texture_stride;
  uint16_t texture_width;
  uint16_t texture_height;
  uint32_t texture_size;
  uint32_t max_z_curve_ix;
} __attribute__ ((packed));

static_assert((sizeof (font)) == ((sizeof (uint32_t)) * 8));
