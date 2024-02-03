#include <cstdint>

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "vga.hpp"
#include "align.hpp"
#include "sh7091/serial.hpp"

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

constexpr uint32_t width = 48;
constexpr uint32_t height = 32;
constexpr uint32_t pixels_per_byte = 8;
constexpr uint32_t wink_size = width * height / pixels_per_byte;

uint32_t _command_buf[(1024 + 32) / 4];
uint32_t _receive_buf[(1024 + 32) / 4];

void main()
{
  uint32_t wink_buf[wink_size / 4];
  make_wink(wink_buf);

  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

  const uint32_t command_size = maple::init_block_write(command_buf, receive_buf,
                                                        host_instruction::port_select::a,
                                                        ap::de::expansion_device | ap::port_select::a | ap::lm_bus::_0,
                                                        wink_buf,
                                                        wink_size);
  using response_type = device_reply;
  using host_response_type = struct maple::command_response<response_type::data_fields>;
  auto host_response = reinterpret_cast<host_response_type *>(receive_buf);
  maple::dma_start(command_buf, command_size,
                   receive_buf, maple::sizeof_command(host_response));

  serial::integer<uint8_t>(host_response->bus_data.command_code);
  serial::integer<uint8_t>(host_response->bus_data.destination_ap);
  serial::integer<uint8_t>(host_response->bus_data.source_ap);
  serial::integer<uint8_t>(host_response->bus_data.data_size);

  vga();
  v_sync_in();
  vga_fill_framebuffer();
  while(1);
}
