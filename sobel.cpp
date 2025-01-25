#include <stdint.h>

int clamp255(float v)
{
  int n = (int)v;
  if (n < 0)
    return 0;
  if (n > 255)
    return 255;
  return n;
}

uint32_t getpx(uint32_t * buf, int x, int y)
{
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;
  if (x >= 640)
    x = 640 - 1;
  if (y >= 480)
    y = 480 - 1;
  return buf[y * 640 + x];
}

float multiply(uint32_t * buf, int x, int y, float weight)
{
  uint32_t color = getpx(buf, x, y);
  int b = color & 0xff;
  color >>= 8;
  int g = color & 0xff;
  color >>= 8;
  int r = color & 0xff;
  color >>= 8;
  int a = color;

  float luminance = (float)(r + g + b + a) * 0.25;
  return luminance * (float)weight;
}

float kernel(uint32_t * buf, const float * weights, int x, int y)
{
  float c = 0;
  c += multiply(buf, x - 1, y - 1, weights[0]);
  c += multiply(buf, x    , y - 1, weights[1]);
  c += multiply(buf, x + 1, y - 1, weights[2]);

  c += multiply(buf, x - 1, y    , weights[3]);
  c += multiply(buf, x    , y    , weights[4]);
  c += multiply(buf, x + 1, y    , weights[5]);

  c += multiply(buf, x - 1, y + 1, weights[6]);
  c += multiply(buf, x    , y + 1, weights[7]);
  c += multiply(buf, x + 1, y + 1, weights[8]);

  return c;
}

const float gx[] = {
    1, 0, -1,
    2, 0, -2,
    1, 0, -1,
};

const float gy[] = {
    1, 2, 1,
    0, 0, 0,
    -1, -2, -1,
};

void convolve(uint32_t * in, uint32_t * out)
{
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      float vx = kernel(in, gx, x, y);
      float vy = kernel(in, gy, x, y);
      float c = vx * vx + vy * vy;
      int d = c > 100.f ? 0 : 1;
      uint32_t color = in[y * 640 + x];

      int b = color & 0xff;
      color >>= 8;
      int g = color & 0xff;
      color >>= 8;
      int r = color & 0xff;
      color >>= 8;
      int a = color;

      uint32_t color_out = 0;

      //color_out |= (a * d);
      //color_out <<= 8;
      color_out |= (r * d);
      color_out <<= 8;
      color_out |= (g * d);
      color_out <<= 8;
      color_out |= (b * d);

      out[y * 640 + x] = color_out;
    }
  }
}
