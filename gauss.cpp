#include <stdint.h>

#include "math/vec3.hpp"

using vec3 = vec<3, float>;

constexpr int dim = 5;
constexpr int dim2 = dim / 2;
constexpr int width = 128;
constexpr int height = 96;

constexpr float mat[dim][dim] = {
  {0.014218, 0.027920, 0.034963, 0.027920, 0.014218},
  {0.027920, 0.054827, 0.068657, 0.054827, 0.027920},
  {0.034963, 0.068657, 0.085976, 0.068657, 0.034963},
  {0.027920, 0.054827, 0.068657, 0.054827, 0.027920},
  {0.014218, 0.027920, 0.034963, 0.027920, 0.014218},
};

inline constexpr int clamp(int v, int max)
{
  if (v > max)
    return max;
  if (v < 0)
    return 0;
  return v;
}

inline constexpr vec3 gauss_pixel(vec3 const * const src, const int cx, const int cy)
{
  vec3 acc = {0, 0, 0};

  for (int dy = 0; dy < dim; dy++) {
    for (int dx = 0; dx < dim; dx++) {
      float i = mat[dy][dx];

      int x = clamp((dx - dim2) + cx, width - 1);
      int y = clamp((dy - dim2) + cy, height - 1);
      int ix = y * width + x;

      acc = acc + (src[ix] * i);
    }
  }
  return acc;
}

inline constexpr void gauss(vec3 const * const src, uint16_t * const dst)
{
  for (int cy = 0; cy < height; cy++) {
    for (int cx = 0; cx < width; cx++) {
      vec3 v = gauss_pixel(src, cx, cy);
      int r = clamp(v.x, 31);
      int g = clamp(v.y, 63);
      int b = clamp(v.z, 31);
      uint16_t px = (r << 11) | (g << 5) | (b << 0);
      dst[cy * width + cx] = px;
    }
  }
}

void gauss_rgb565(uint16_t const * const src, uint16_t * const dst)
{
  static vec3 tmp[width * height];

  for (int i = 0; i < width * height; i++) {
    uint16_t px = src[i];
    float r = (px >> 11) & 0b11111;
    float g = (px >>  5) & 0b111111;
    float b = (px >>  0) & 0b11111;
    tmp[i] = {r, g, b};
  }

  gauss(tmp, dst);
}
