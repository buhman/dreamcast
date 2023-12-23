#include <cassert>
#include <cstdint>
#include <compare>
#include <iostream>

#include "insertion_sort.hpp"
#include "rect.hpp"
#include "../twiddle.hpp"
#include "2d_pack.hpp"

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
    auto [x_offset, y_offset] = twiddle::from_ix(z_curve_ix);

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
			      rect.y + rect.height - 1);
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
