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
*/

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

/*
  the following code is not correct

template <uint32_t dst_bits_per_pixel, typename T, typename U>
void texture2(volatile T * dst, const U * src,
	      const uint32_t src_stride,
	      const uint32_t curve_end_ix)
{
  constexpr uint32_t t_bits = (sizeof (T)) * 8;
  static_assert(t_bits >= dst_bits_per_pixel);
  static_assert((t_bits / dst_bits_per_pixel) * dst_bits_per_pixel == t_bits);
  constexpr uint32_t pixels_per_t = t_bits / dst_bits_per_pixel;
  static_assert(pixels_per_t == 1 || pixels_per_t == 2 || pixels_per_t == 4 || pixels_per_t == 8 || pixels_per_t == 16 || pixels_per_t == 32);

  T dst_val = 0;
  for (uint32_t curve_ix = 0; curve_ix <= curve_end_ix; curve_ix++) {
    auto [x, y] = from_ix(curve_ix);
    const U src_val = src[y * src_stride + x];
    if constexpr (pixels_per_t == 1) {
      dst[curve_ix] = src_val;
    } else {
      const uint32_t curve_ix_mod = curve_ix & (pixels_per_t - 1);
      dst_val |= src_val << (dst_bits_per_pixel * curve_ix_mod);

      if (curve_ix_mod == (pixels_per_t - 1)) {
	dst[curve_ix / pixels_per_t] = dst_val;
	dst_val = 0;
      }
    }
  }
}

template <uint32_t dst_bits_per_pixel, uint32_t src_bits_per_pixel, typename T, typename U>
void texture3(volatile T * dst, const U * src,
	      const uint32_t src_stride,
	      const uint32_t curve_end_ix)
{
  constexpr uint32_t t_bits = (sizeof (T)) * 8;
  static_assert(t_bits >= dst_bits_per_pixel);
  static_assert((t_bits / dst_bits_per_pixel) * dst_bits_per_pixel == t_bits);
  constexpr uint32_t pixels_per_t = t_bits / dst_bits_per_pixel;
  static_assert(pixels_per_t == 1 || pixels_per_t == 2 || pixels_per_t == 4 || pixels_per_t == 8 || pixels_per_t == 16 || pixels_per_t == 32);

  constexpr uint32_t u_bits = (sizeof (U)) * 8;
  static_assert(u_bits >= src_bits_per_pixel);
  static_assert((u_bits / src_bits_per_pixel) * src_bits_per_pixel == u_bits);
  constexpr uint32_t pixels_per_u = u_bits / src_bits_per_pixel;
  static_assert(pixels_per_u == 1 || pixels_per_u == 2 || pixels_per_u == 4 || pixels_per_u == 8 || pixels_per_u == 16 || pixels_per_u == 32);

  constexpr uint32_t src_val_mask = ((1 << src_bits_per_pixel) - 1);
  constexpr uint32_t dst_val_mask = ((1 << dst_bits_per_pixel) - 1);
  constexpr uint32_t dst_src_shift = (dst_bits_per_pixel < src_bits_per_pixel) ? (src_bits_per_pixel - dst_bits_per_pixel) : 0;

  T dst_val = 0;
  for (uint32_t curve_ix = 0; curve_ix <= curve_end_ix; curve_ix++) {
    auto [x, y] = from_ix(curve_ix);
    const uint32_t src_ix = y * src_stride + (x / pixels_per_u);
    const uint32_t src_ix_mod = x & (pixels_per_u - 1);
    const U src_val = (src[src_ix] >> (src_bits_per_pixel * src_ix_mod)) & src_val_mask;
    if constexpr (pixels_per_t == 1) {
      dst[curve_ix] = src_val;
    } else {
      const uint32_t curve_ix_mod = curve_ix & (pixels_per_t - 1);
      dst_val |= ((src_val >> dst_src_shift) & dst_val_mask) << (dst_bits_per_pixel * curve_ix_mod);

      if (curve_ix_mod == (pixels_per_t - 1)) {
	dst[curve_ix / pixels_per_t] = dst_val;
	dst_val = 0;
      }
    }
  }
}
*/

}
