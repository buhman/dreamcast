#pragma once

constexpr inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}

constexpr inline uint16_t grey565(uint8_t i)
{
  return ((i >> 3) << 11) | ((i >> 2) << 5) | ((i >> 3) << 0);
}

constexpr inline uint16_t argb4444(uint8_t i)
{
  return (i << 12) | (15 << 8) | (15 << 4) | (15 << 0);
}

template <int C>
void palette_data();

template <>
void palette_data<256>()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  // ranging in intensity from rgb565(0, 0, 0) to rgb565(31, 63, 31)
  for (int i = 0; i < 256; i += 1) {
    holly.PALETTE_RAM[i] = grey565(i);
  }
}

template <>
void palette_data<16>()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb4444;

  // ranging in intensity from rgb565(0, 0, 0) to rgb565(31, 63, 31)
  for (uint32_t i = 0; i < 16; i++) {
    holly.PALETTE_RAM[i] = argb4444(i);
  }
}

template <>
void palette_data<2>()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;

  holly.PALETTE_RAM[0] = 0x0000;
  holly.PALETTE_RAM[1] = 0xffff;
}
