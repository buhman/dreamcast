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

constexpr inline uint32_t from_xy(uint32_t x, uint32_t y)
{
  // maximum texture size       : 1024x1024
  // maximum 1-dimensional index: 0xfffff
  // bits                       : 19-0

  uint32_t twiddle_ix = 0;
  for (int i = 0; i <= (20 / 2); i++) {
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

/*
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
*/

constexpr inline std::tuple<uint32_t, uint32_t>
from_ix(uint32_t curve_ix)
{
  uint32_t y = (curve_ix >> 0) & 0x55555555;
  y = (y | (y >> 1)) & 0x33333333;
  y = (y | (y >> 2)) & 0x0f0f0f0f;
  y = (y | (y >> 4)) & 0x00ff00ff;
  y = (y | (y >> 8)) & 0x0000ffff;

  uint32_t x = (curve_ix >> 1) & 0x55555555;
  x = (x | (x >> 1)) & 0x33333333;
  x = (x | (x >> 2)) & 0x0f0f0f0f;
  x = (x | (x >> 4)) & 0x00ff00ff;
  x = (x | (x >> 8)) & 0x0000ffff;

  return {x, y};
}

using xy_type = std::tuple<uint32_t, uint32_t>;
static_assert(from_ix(0) == xy_type{0b000, 0b000});
static_assert(from_ix(2) == xy_type{0b001, 0b000});
static_assert(from_ix(8) == xy_type{0b010, 0b000});
static_assert(from_ix(10) == xy_type{0b011, 0b000});
static_assert(from_ix(32) == xy_type{0b100, 0b000});
static_assert(from_ix(34) == xy_type{0b101, 0b000});
static_assert(from_ix(40) == xy_type{0b110, 0b000});
static_assert(from_ix(42) == xy_type{0b111, 0b000});

static_assert(from_ix(1) == xy_type{0b000, 0b001});
static_assert(from_ix(4) == xy_type{0b000, 0b010});
static_assert(from_ix(5) == xy_type{0b000, 0b011});
static_assert(from_ix(16) == xy_type{0b000, 0b100});
static_assert(from_ix(17) == xy_type{0b000, 0b101});
static_assert(from_ix(20) == xy_type{0b000, 0b110});
static_assert(from_ix(21) == xy_type{0b000, 0b111});

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

template <int B, typename T, typename U>
void texture2(volatile T * dst, const U * src,
	      const uint32_t width, const uint32_t height,
	      const uint32_t stride)
{
  constexpr uint32_t t_bits = (sizeof (T)) * 8;
  constexpr uint32_t bits_per_pixel = B;
  static_assert(t_bits >= bits_per_pixel);
  static_assert((t_bits / bits_per_pixel) * bits_per_pixel == t_bits);
  constexpr uint32_t pixels_per_t = t_bits / bits_per_pixel;
  static_assert(pixels_per_t == 1 || pixels_per_t == 2 || pixels_per_t == 4 || pixels_per_t == 8 || pixels_per_t == 16 || pixels_per_t == 32);

  T dst_val = 0;
  const uint32_t end_ix = from_xy(width - 1, height - 1);
  for (uint32_t curve_ix = 0; curve_ix <= end_ix; curve_ix++) {
    auto [x, y] = from_ix(curve_ix);
    const U src_val = src[y * stride + x];
    if constexpr (pixels_per_t == 1) {
      dst[curve_ix] = src_val;
    } else {
      const uint32_t curve_ix_mod = curve_ix & (pixels_per_t - 1);
      dst_val |= src_val << (bits_per_pixel * curve_ix_mod);

      if (curve_ix_mod == (pixels_per_t - 1)) {
	dst[curve_ix / pixels_per_t] = dst_val;
	dst_val = 0;
      }
    }
  }
}

}
