#pragma once

#include <cstdint>

namespace color_format {

uint32_t argb8888(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
  return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

uint16_t argb4444(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
  int a4 = (a >> 4) & 15;
  int r4 = (r >> 4) & 15;
  int g4 = (g >> 4) & 15;
  int b4 = (b >> 4) & 15;
  return (a4 << 12) | (r4 << 8) | (g4 << 4) | (b4 << 0);
}


uint16_t argb1555(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
  int a1 = (a >> 7) & 1;
  int r5 = (r >> 3) & 31;
  int g5 = (g >> 3) & 31;
  int b5 = (b >> 3) & 31;
  return (a1 << 15) | (r5 << 10) | (g5 << 5) | (b5 << 0);
}

uint16_t rgb565(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
  int r5 = (r >> 3) & 31;
  int g6 = (g >> 2) & 63;
  int b5 = (b >> 3) & 31;
  return (r5 << 11) | (g6 << 5) | (b5 << 0);
}

}
