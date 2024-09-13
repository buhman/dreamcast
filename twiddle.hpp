#include <cstdint>
#include <tuple>

namespace twiddle {

/*
This reproduces the twiddle index table shown in
"3.6.2.1 Twiddled Format".

              x →
            000     001     010     011
       --------------------------------
       | xyxyxy  xyxyxy  xyxyxy  xyxyxy
       |===============================
 y 000 | 000000  000010  001000  001010
 ↓ 001 | 000001  000011  001001  001011
   010 | 000100  000110  001100  001110
   011 | 000101  000111  001101  001111

alternately, in verilog syntax:


  input  [2:0] x; // x coordinate
  input  [2:0] y; // y coordinate
  output [5:0] t; // twiddled index
  assign t = {x[2], y[2], x[1], y[1], x[0], y[0]};
*/

constexpr inline int log2(uint32_t n)
{
  switch (n) {
  default:
  case 8: return 3;
  case 16: return 4;
  case 32: return 5;
  case 64: return 6;
  case 128: return 7;
  case 256: return 8;
  case 512: return 9;
  case 1024: return 10;
  }
}

constexpr inline uint32_t from_xy(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  // maximum texture size       : 1024x1024
  // maximum 1-dimensional index: 0xfffff
  // bits                       : 19-0

  // y bits: 0, 2, 4, 6, 8, 10, 12, 14, 16, 18
  // x bits: 1, 3, 5, 7, 9, 11, 13, 15, 17, 19

  int width_max = log2(width);
  int height_max = log2(height);

  uint32_t twiddle_ix = 0;
  for (int i = 0; i < (20 / 2); i++) {
    if (i < width_max && i < height_max) {
      twiddle_ix |= ((y >> i) & 1) << (i * 2 + 0);
      twiddle_ix |= ((x >> i) & 1) << (i * 2 + 1);
    } else if (i < width_max) {
      twiddle_ix |= ((x >> i) & 1) << (i + height_max);
    } else if (i < height_max) {
      twiddle_ix |= ((y >> i) & 1) << (i + width_max);
    } else {
      break;
    }
  }

  return twiddle_ix;
}

static_assert(from_xy(0b000, 0b000, 8, 8) == 0);
static_assert(from_xy(0b001, 0b000, 8, 8) == 2);
static_assert(from_xy(0b010, 0b000, 8, 8) == 8);
static_assert(from_xy(0b011, 0b000, 8, 8) == 10);
static_assert(from_xy(0b100, 0b000, 8, 8) == 32);
static_assert(from_xy(0b101, 0b000, 8, 8) == 34);
static_assert(from_xy(0b110, 0b000, 8, 8) == 40);
static_assert(from_xy(0b111, 0b000, 8, 8) == 42);

static_assert(from_xy(0b000, 0b001, 8, 8) == 1);
static_assert(from_xy(0b000, 0b010, 8, 8) == 4);
static_assert(from_xy(0b000, 0b011, 8, 8) == 5);
static_assert(from_xy(0b000, 0b100, 8, 8) == 16);
static_assert(from_xy(0b000, 0b101, 8, 8) == 17);
static_assert(from_xy(0b000, 0b110, 8, 8) == 20);
static_assert(from_xy(0b000, 0b111, 8, 8) == 21);

//                                1  0  0  0
// x bits: 19, 17, 15, 13, 11, 9, 7, 5, 3, 1
// y bits: 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
static_assert(from_xy(0b1000, 0b001, 16, 8) == 65);
static_assert(from_xy(0b1010, 0b001, 16, 8) == 73);

static_assert(from_xy(0b000, 0b1001, 8, 16) == 65);
static_assert(from_xy(0b010, 0b1001, 8, 16) == 73);

template <typename T>
void texture(volatile T * dst, const T * src, const uint32_t width, const uint32_t height)
{
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint32_t twiddle_ix = from_xy(x, y, width, height);
      T value = src[y * width + x];
      dst[twiddle_ix] = value;
    }
  }
}

template <typename T>
void texture_4bpp(volatile T * dst, const T * src, const uint32_t width, const uint32_t height)
{
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint32_t twiddle_ix = from_xy(x, y, width, height);
      T value = src[y * width + x];
      uint32_t shift = (4 * (twiddle_ix & 1));
      dst[twiddle_ix / 2] &= ~(0b1111 << shift);
      dst[twiddle_ix / 2] |= value << shift;
    }
  }
}
}
