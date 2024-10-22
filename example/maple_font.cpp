#include <cstdint>
#include <bit>

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_display.hpp"
#include "align.hpp"
#include "sh7091/serial.hpp"

#include "holly/video_output.hpp"

extern uint32_t _binary_font_portfolio_6x8 __asm("_binary_font_portfolio_6x8_portfolio_6x8_data_start");

void make_vmu_framebuffer(maple::display::font_renderer& renderer)
{
  const char * s =
    " very   "
    " funneh "
    "        "
    " :))    ";
  for (int i = 0; i < 8 * 4; i++) {
    int x = i % 8;
    int y = i / 8;
    renderer.glyph(s[i], x, y);
  }
}

template <typename T>
inline void copy(T * dst, const T * src, const int32_t n) noexcept
{
  int32_t n_t = n / (sizeof (T));
  while (n_t > 0) {
    *dst++ = *src++;
    n_t--;
  }
}

static uint8_t * framebuffer;

void send_vmu_framebuffer(uint8_t port, uint8_t lm)
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  using command_type = maple::block_write<uint8_t[0]>;
  using response_type = maple::device_reply;

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 true,      // end_flag
							 maple::display::vmu::framebuffer_size, // send_trailing
							 0          // recv_trailing
							 );
  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = std::byteswap(function_type::bw_lcd);
  data_fields.pt = 0;
  data_fields.phase = 0;
  data_fields.block_number = std::byteswap<uint16_t>(0x0000);

  copy<uint8_t>(data_fields.written_data,
		reinterpret_cast<uint8_t *>(framebuffer),
		maple::display::vmu::framebuffer_size);

  maple::dma_start(send_buf, writer.send_offset,
		   recv_buf, writer.recv_offset);

  serial::integer<uint8_t>(host_response->bus_data.command_code);
  serial::integer<uint8_t>(host_response->bus_data.destination_ap);
  serial::integer<uint8_t>(host_response->bus_data.source_ap);
  serial::integer<uint8_t>(host_response->bus_data.data_size);
}

void do_lm_request(uint8_t port, uint8_t lm)
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 true); // end_flag

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  auto& bus_data = host_response->bus_data;
  auto& data_fields = bus_data.data_fields;
  if (bus_data.command_code != maple::device_status::command_code) {
    serial::string("lm did not reply: ");
    serial::integer<uint8_t>(port, ' ');
    serial::integer<uint8_t>(lm);
  } else {
    serial::string("    lm: ");
    serial::integer<uint8_t>(lm);
    serial::string("      ft:    ");
    serial::integer<uint32_t>(std::byteswap(data_fields.device_id.ft));
    serial::string("      fd[0]: ");
    serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[0]));
    serial::string("      fd[1]: ");
    serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[1]));
    serial::string("      fd[2]: ");
    serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[2]));
    serial::string("      source_ap.lm_bus: ");
    serial::integer<uint8_t>(bus_data.source_ap & ap::lm_bus::bit_mask);

    if (std::byteswap(data_fields.device_id.ft) & function_type::bw_lcd) {
      serial::string("send vmu_framebuffer\n");
      send_vmu_framebuffer(port, lm);
    }
  }
}

void do_lm_requests(uint8_t port, uint8_t lm)
{
  if (lm & ap::lm_bus::_0)
    do_lm_request(port, lm & ap::lm_bus::_0);
  if (lm & ap::lm_bus::_1)
    do_lm_request(port, lm & ap::lm_bus::_1);
  if (lm & ap::lm_bus::_2)
    do_lm_request(port, lm & ap::lm_bus::_2);
  if (lm & ap::lm_bus::_3)
    do_lm_request(port, lm & ap::lm_bus::_3);
  if (lm & ap::lm_bus::_4)
    do_lm_request(port, lm & ap::lm_bus::_4);
}

void do_device_request()
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != response_type::command_code) {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  disconnected\n");
    } else {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  ft:    ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.ft));
      serial::string("  fd[0]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[0]));
      serial::string("  fd[1]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[1]));
      serial::string("  fd[2]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[2]));
      serial::string("  source_ap.lm_bus: ");
      serial::integer<uint8_t>(bus_data.source_ap & ap::lm_bus::bit_mask);

      do_lm_requests(port,
		     bus_data.source_ap & ap::lm_bus::bit_mask);
    }
  }
}

void main()
{
  serial::init(4);

  const uint8_t * font = reinterpret_cast<const uint8_t *>(&_binary_font_portfolio_6x8);
  auto renderer = maple::display::font_renderer(font);
  make_vmu_framebuffer(renderer);
  framebuffer = renderer.fb;

  do_device_request();

  while (1);
}
