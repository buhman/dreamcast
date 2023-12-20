#include <cstdint>
#include <array>

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

template <typename T>
void texture(volatile T * dst, const T * src, const uint32_t width, const uint32_t height)
{
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint32_t twiddle_ix = from_xy(x, y);
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
      uint32_t twiddle_ix = from_xy(x, y);
      T value = src[y * width + x];
      uint32_t shift = (4 * (twiddle_ix & 1));
      dst[twiddle_ix / 2] &= ~(0b1111 << shift);
      dst[twiddle_ix / 2] |= value << shift;
    }
  }
}

}
