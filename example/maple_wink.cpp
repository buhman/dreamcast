#include <cstdint>

#include "maple/maple.hpp"
#include "maple/maple_bus_bits.hpp"
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

  uint32_t __attribute__((aligned(4))) wink_buf[(width * height / pixels_per_byte + 32) / 4];
  make_wink(wink_buf);
  if ((((uint32_t)wink_buf) & 3) != 0) serial::string("misaligned\n");

  uint32_t _command_buf[(1024 + 32) / 4];
  uint32_t _receive_buf[(1024 + 32) / 4];
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);
  if ((((uint32_t)command_buf) & 31) != 0) serial::string("misaligned\n");
  if ((((uint32_t)receive_buf) & 31) != 0) serial::string("misaligned\n");

  maple::init_block_write(command_buf, receive_buf,
                          host_instruction::port_select::a,
                          ap::de::expansion_device | ap::port_select::a | ap::lm_bus::_0,
                          wink_buf,
                          192);
  maple::dma_start(command_buf);

  for (int i = 0; i < 1; i++) {
    serial::integer<uint32_t>(receive_buf[i]);
  }

  vga();
  v_sync_in();
  vga_fill_framebuffer();
  while(1);
}
