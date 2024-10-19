#include <cstdint>
#include <bit>

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_port.hpp"
#include "align.hpp"
#include "sh7091/serial.hpp"

#include "holly/video_output.hpp"

extern uint32_t _binary_font_portfolio_6x8 __asm("_binary_font_portfolio_6x8_portfolio_6x8_data_start");

namespace vmu_display {
constexpr int32_t width = 48;
constexpr int32_t height = 32;
constexpr int32_t pixels_per_byte = 8;
constexpr int32_t framebuffer_size = width * height / pixels_per_byte;
}

uint32_t vmu_framebuffer[vmu_display::framebuffer_size / 4];

static inline void render_glyph(uint8_t * dst, const uint8_t * src, uint8_t c, int x, int y)
{
  int y_ix = 186 - (y * 6 * 8);
  for (int i = 0; i < 8; i++) {
    switch (x) {
    case 0:
      dst[y_ix - i * 6 + 5] = src[(c - ' ') * 8 + i];
      break;
    case 1:
      dst[y_ix - i * 6 + 5] |= (src[(c - ' ') * 8 + i] & 0b11) << 6;
      dst[y_ix - i * 6 + 4] = src[(c - ' ') * 8 + i] >> 2; // 0b1111
      break;
    case 2:
      dst[y_ix - i * 6 + 4] |= (src[(c - ' ') * 8 + i] & 0b1111) << 4;
      dst[y_ix - i * 6 + 3] = src[(c - ' ') * 8 + i] >> 4; // 0b11
      break;
    case 3:
      dst[y_ix - i * 6 + 3] |= src[(c - ' ') * 8 + i] << 2;
      break;
    case 4:
      dst[y_ix - i * 6 + 2] = src[(c - ' ') * 8 + i];
      break;
    case 5:
      dst[y_ix - i * 6 + 2] |= (src[(c - ' ') * 8 + i] & 0b11) << 6;
      dst[y_ix - i * 6 + 1] = src[(c - ' ') * 8 + i] >> 2; // 0b1111
      break;
    case 6:
      dst[y_ix - i * 6 + 1] |= (src[(c - ' ') * 8 + i] & 0b1111) << 4;
      dst[y_ix - i * 6 + 0] = src[(c - ' ') * 8 + i] >> 4; // 0b11
      break;
    case 7:
      dst[y_ix - i * 6 + 0] |= src[(c - ' ') * 8 + i] << 2;
      break;
    }
  }
}

void make_vmu_framebuffer(uint32_t * buf)
{
  const uint8_t * src = reinterpret_cast<const uint8_t *>(&_binary_font_portfolio_6x8);
  uint8_t * dst = reinterpret_cast<uint8_t *>(buf);

  for (int i = 0; i < vmu_display::framebuffer_size; i++) {
    dst[i] = 0;
  }
  const char * s =
    " very   "
    " funneh "
    "        "
    " :))    ";
  for (int i = 0; i < 8 * 4; i++) {
    int x = i % 8;
    int y = i / 8;
    render_glyph(dst, src, s[i], x, y);
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
							 vmu_display::framebuffer_size, // send_trailing
							 0          // recv_trailing
							 );
  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = std::byteswap(function_type::bw_lcd);
  data_fields.pt = 0;
  data_fields.phase = 0;
  data_fields.block_number = std::byteswap<uint16_t>(0x0000);

  copy<uint8_t>(data_fields.written_data, reinterpret_cast<uint8_t *>(vmu_framebuffer), vmu_display::framebuffer_size);

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

  make_vmu_framebuffer(vmu_framebuffer);

  do_device_request();

  while (1);
}
