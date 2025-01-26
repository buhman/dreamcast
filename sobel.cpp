#include <stdint.h>

#include "sobel.hpp"

static inline float getpx(float * buf, int x, int y)
{
  return buf[y * 640 + x];
}

static inline float kernel2(float * buf, int x, int y)
{
  constexpr float gx[] = {
    1, 0, -1,    /* fr0 , _ , xf12 */
    2, 0, -2,    /* fr1 , _ , xf13 */
    1, 0, -1,    /* fr2, _ , xf14 */
  };

  constexpr float gy[] = {
    1, 2, 1,     /* fr0, fr1, fr2 */
    0, 0, 0,
    -1, -2, -1,  /* fr4, fr5, fr6 */
  };

  float a = getpx(buf, x - 1, y - 1);
  float b = getpx(buf, x    , y - 1);
  float c = getpx(buf, x + 1, y - 1);

  float d = getpx(buf, x - 1, y    );
  float e = getpx(buf, x    , y    );
  float f = getpx(buf, x + 1, y    );

  float g = getpx(buf, x - 1, y + 1);
  float h = getpx(buf, x    , y + 1);
  float i = getpx(buf, x + 1, y + 1);

  float sx = 0;
  float sy = 0;

  sx += a * gx[0];
  //sx += b * gx[1];
  sx += c * gx[2];

  sx += d * gx[3];
  //sx += e * gx[4];
  sx += f * gx[5];

  sx += g * gx[6];
  //sx += h * gx[7];
  sx += i * gx[8];

  sy += a * gy[0];
  sy += b * gy[1];
  sy += c * gy[2];

  //sy += d * gy[3];
  //sy += e * gy[4];
  //sy += f * gy[5];

  sy += g * gy[6];
  sy += h * gy[7];
  sy += i * gy[8];

  return sx * sx + sy * sy;
}

void convolve(float * in, uint32_t * out)
{
  for (int y = 1; y < 480 - 1; y++) {
    for (int x = 1; x < 640 - 1; x++) {
      float c = kernel2(in, x, y);
      int d = c > 100.f ? 0 : 0xffffffff;

      out[y * 640 + x] = (uint8_t)d;
    }
  }
}
