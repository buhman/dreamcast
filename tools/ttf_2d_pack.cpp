#include <cassert>
#include <cstdint>
#include <compare>
#include <iostream>
#include <array>

#include "insertion_sort.hpp"
#include "rect.hpp"
#include "../twiddle.hpp"
#include "ttf_2d_pack.hpp"

struct size {
  uint16_t width;
  uint16_t height;
};

constexpr struct size max_texture = {1024, 1024};

inline bool area_valid(const uint8_t texture[max_texture.height][max_texture.width],
		       const uint32_t x_offset,
		       const uint32_t y_offset,
		       const struct rect& rect,
		       const struct size& window)
{
  for (uint32_t yi = 0; yi < rect.height; yi++) {
    for (uint32_t xi = 0; xi < rect.width; xi++) {
      uint32_t x = x_offset + xi;
      uint32_t y = y_offset + yi;

      if (texture[y][x] != 0)
	return false;

      if (x >= window.width || y >= window.height)
	return false;
    }
  }

  return true;
}

constexpr inline std::array<uint32_t, 2>
from_ix(uint32_t curve_ix)
{
  std::array<uint32_t, 2> x_y = {0, 0};
  uint32_t curve_bit = 0;

  while (curve_ix != 0) {
    x_y[(curve_bit + 1) % 2] |= (curve_ix & 1) << (curve_bit / 2);
    curve_ix >>= 1;
    curve_bit += 1;
  }

  return x_y;
}

static_assert(from_ix(0) == std::array<uint32_t, 2>{{0b000, 0b000}});
static_assert(from_ix(2) == std::array<uint32_t, 2>{{0b001, 0b000}});
static_assert(from_ix(8) == std::array<uint32_t, 2>{{0b010, 0b000}});
static_assert(from_ix(10) == std::array<uint32_t, 2>{{0b011, 0b000}});
static_assert(from_ix(32) == std::array<uint32_t, 2>{{0b100, 0b000}});
static_assert(from_ix(34) == std::array<uint32_t, 2>{{0b101, 0b000}});
static_assert(from_ix(40) == std::array<uint32_t, 2>{{0b110, 0b000}});
static_assert(from_ix(42) == std::array<uint32_t, 2>{{0b111, 0b000}});

static_assert(from_ix(1) == std::array<uint32_t, 2>{{0b000, 0b001}});
static_assert(from_ix(4) == std::array<uint32_t, 2>{{0b000, 0b010}});
static_assert(from_ix(5) == std::array<uint32_t, 2>{{0b000, 0b011}});
static_assert(from_ix(16) == std::array<uint32_t, 2>{{0b000, 0b100}});
static_assert(from_ix(17) == std::array<uint32_t, 2>{{0b000, 0b101}});
static_assert(from_ix(20) == std::array<uint32_t, 2>{{0b000, 0b110}});
static_assert(from_ix(21) == std::array<uint32_t, 2>{{0b000, 0b111}});


uint32_t pack_into(uint8_t texture[max_texture.height][max_texture.width],
		   struct size& window,
		   struct rect& rect)
{
  uint32_t z_curve_ix = 0;

  if (rect.width == 0 || rect.height == 0) {
    rect.x = 0;
    rect.y = 0;
    return false;
  }

  while (true) {
    auto [x_offset, y_offset] = from_ix(z_curve_ix);

    if (x_offset >= window.width and y_offset >= window.height) {
      //std::cerr << z_curve_ix << ' ' << window.width << ' ' << window.height << '\n';
      assert(window.width < max_texture.width || window.height < max_texture.height);
      if (window.width == window.height) { window.height *= 2; }
      else                               { window.width *= 2; }

      // when the window changes; start again from the beginning and
      // re-check earlier locations that might have been skipped due
      // to window size
      z_curve_ix = 0;
    }

    if (area_valid(texture, x_offset, y_offset, rect, window)) {
      for (uint32_t yi = 0; yi < rect.height; yi++) {
	for (uint32_t xi = 0; xi < rect.width; xi++) {
	  uint32_t x = x_offset + xi;
	  uint32_t y = y_offset + yi;

	  texture[y][x] = 1;
	}
      }

      rect.x = x_offset;
      rect.y = y_offset;

      return twiddle::from_xy(rect.x + rect.width - 1,
			      rect.y + rect.height - 1,
                              1024,
                              1024);
    } else {
      z_curve_ix += 1;
      continue;
    }
  }

  assert(false);
}

struct window_curve_ix
pack_all(struct rect * rects, const uint32_t num_rects)
{
  uint8_t texture[max_texture.height][max_texture.width] = { 0 };
  size window = {1, 1};

  // sort all rectangles by size
  insertion_sort(rects, num_rects);

  uint32_t max_z_curve_ix = 0;

  for (uint32_t i = 0; i < num_rects; i++) {
    uint32_t z_curve_ix = pack_into(texture, window, rects[i]);
    //std::cerr << "z_curve_ix " << z_curve_ix << '\n';
    if (z_curve_ix > max_z_curve_ix)
      max_z_curve_ix = z_curve_ix;
  }

  //std::cerr << "window size: " << window.width << ' ' << window.height << '\n';
  std::cerr << "max_z_curve_ix: " << max_z_curve_ix << '\n';
  return {window.width, window.height, max_z_curve_ix};
}
