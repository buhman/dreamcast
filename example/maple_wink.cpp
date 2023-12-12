#include <cstdint>

#include "maple/maple.hpp"
#include "vga.hpp"
#include "align.hpp"
#include "serial.hpp"

extern uint32_t _binary_wink_data_start __asm("_binary_wink_data_start");

void make_wink(uint32_t * buf)
{
  const uint8_t * src = reinterpret_cast<const uint8_t *>(&_binary_wink_data_start);
  uint8_t * dst = reinterpret_cast<uint8_t *>(buf);

  uint32_t ix = 0;
  dst[ix] = 0;
  for (int i = 0; i < 48 * 32; i++) {
    dst[ix] |= ((src[i] & 1) << (7 - (i % 8)));

    if (i % 8 == 7) {
      ix++;
      dst[ix] = 0;
    }
  }
}

void main()
{
  constexpr int width = 48;
  constexpr int height = 32;
  constexpr int pixels_per_byte = 8;

  uint32_t wink_buf[width * height / pixels_per_byte];
  make_wink(wink_buf);

  uint32_t _command_buf[128 / 4];
  uint32_t _receive_buf[128 / 4];
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

  maple::init_block_write(command_buf, receive_buf, wink_buf);
  maple::dma_start(command_buf);

  for (int i = 0; i < 32; i++) {
    serial::integer<uint32_t>(receive_buf[i]);
  }

  vga();
  v_sync_in();
  vga_fill_framebuffer();
  while(1);
}
