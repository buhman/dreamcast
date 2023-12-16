#include <cstdint>

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

constexpr inline uint32_t from_xy(uint32_t x, uint32_t y)
{
  // maximum texture size       : 1024x1024
  // maximum 1-dimensional index: 0xfffff
  // bits                       : 19-0

  uint32_t twiddle_ix = 0;
  for (int i = 0; i <= (19 / 2); i++) {
    twiddle_ix |= ((y >> i) & 1) << (i * 2 + 0);
    twiddle_ix |= ((x >> i) & 1) << (i * 2 + 1);
  }

  return twiddle_ix;
}

static_assert(from_xy(0b000, 0b000) == 0);
static_assert(from_xy(0b001, 0b000) == 2);
static_assert(from_xy(0b010, 0b000) == 8);
static_assert(from_xy(0b011, 0b000) == 10);
static_assert(from_xy(0b100, 0b000) == 32);
static_assert(from_xy(0b101, 0b000) == 34);
static_assert(from_xy(0b110, 0b000) == 40);
static_assert(from_xy(0b111, 0b000) == 42);

static_assert(from_xy(0b000, 0b001) == 1);
static_assert(from_xy(0b000, 0b010) == 4);
static_assert(from_xy(0b000, 0b011) == 5);
static_assert(from_xy(0b000, 0b100) == 16);
static_assert(from_xy(0b000, 0b101) == 17);
static_assert(from_xy(0b000, 0b110) == 20);
static_assert(from_xy(0b000, 0b111) == 21);

template <typename T>
void texture(T * dst, const T * src, const uint32_t width, const uint32_t height)
{
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint32_t twiddle_ix = from_xy(x, y);
      T value = src[y * width + x];
      dst[twiddle_ix] = value;
    }
  }
}

}
