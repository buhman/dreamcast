#include <cstdint>
#include <bit>

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_display.hpp"

#include "serial_load.hpp"

extern uint32_t _binary_font_portfolio_6x8 __asm("_binary_font_portfolio_6x8_portfolio_6x8_data_start");

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

static uint32_t send_buf[1024 / 4] __attribute__((aligned(32)));
static uint32_t recv_buf[1024 / 4] __attribute__((aligned(32)));

enum struct step {
  IDLE = 0,
  DEVICE_STATUS,
  EXTENSION_DEVICE_STATUS,
  EXTENSION_DEVICE_REPLY,
};

struct maple_display_poll_state {
  bool want_start;
  enum step step;
  struct {
    uint8_t ap__lm;
  } port[4];
};

void send_vmu_framebuffer(maple::host_command_writer& writer, uint8_t port, uint8_t lm)
{
  using command_type = maple::block_write<uint8_t[0]>;
  using response_type = maple::device_reply;

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 false,      // end_flag
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
}

void recv_extension_device_status(struct maple_display_poll_state &state)
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using response_type = maple::host_response<maple::device_status::data_fields>;
  auto host_response = reinterpret_cast<response_type *>(recv_buf);

  uint32_t last_send_offset = 0;

  int response_index = 0;
  for (int port = 0; port < 4; port++) {
    uint32_t bit = ap::lm_bus::_0;
    uint8_t lm = state.port[port].ap__lm;

    for (int i = 0; i < 5; i++) {
      if (lm == 0)
	break;

      if ((lm & bit) == 0)
	continue;
      lm &= ~bit;

      auto& bus_data = host_response[response_index++].bus_data;
      auto& data_fields = bus_data.data_fields;
      if ((bus_data.command_code == maple::device_status::command_code) &&
	  (std::byteswap(data_fields.device_id.ft) & function_type::bw_lcd)) {

	last_send_offset = writer.send_offset;
	send_vmu_framebuffer(writer, port, bit);
      } else {
	// this extension device is not a bw_lcd; remove it
	state.port[port].ap__lm &= ~bit;
      }
      bit <<= 1;
    }
  }

  {
    using command_type = maple::host_command<maple::block_write<uint8_t[0]>::data_fields>;
    auto host_command = reinterpret_cast<command_type *>(&send_buf[last_send_offset / 4]);
    host_command->host_instruction |= host_instruction::end_flag;

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);
  }
}

void send_extension_device_request(maple::host_command_writer& writer, uint8_t port, uint8_t lm)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  writer.append_command<command_type, response_type>(host_port_select,
						     destination_ap,
						     false); // end_flag
}

typedef void (* func_t)(maple::host_command_writer& writer, uint8_t port, uint8_t lm);
void do_lm_requests(maple::host_command_writer& writer, uint8_t port, uint8_t lm, func_t func)
{
  uint32_t bit = ap::lm_bus::_0;
  for (int i = 0; i < 5; i++) {
    if (lm & bit) {
      lm &= ~bit;
      func(writer, port, bit);
    }
    bit <<= 1;
  }
}

void recv_device_status(struct maple_display_poll_state &state)
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using response_type = maple::host_response<maple::device_status::data_fields>;
  auto host_response = reinterpret_cast<response_type *>(recv_buf);

  for (int port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != maple::device_status::command_code) {
      state.port[port].ap__lm = 0;
    } else {
      auto& data_fields = bus_data.data_fields;

      uint8_t lm = bus_data.source_ap & ap::lm_bus::bit_mask;
      state.port[port].ap__lm = lm;
      do_lm_requests(writer, port, lm, &send_extension_device_request);
    }
  }

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
}

void send_device_request()
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
}

void handle_maple(struct maple_display_poll_state& state)
{
  switch (state.step) {
  case step::IDLE:
    if (state.want_start) {
      // always send to all ports
      send_device_request();
      state.step = step::DEVICE_STATUS;
      state.want_start = 0;
    }
    break;
  case step::DEVICE_STATUS:
    if (maple::dma_poll_complete()) {
      recv_device_status(state);
      state.step = step::EXTENSION_DEVICE_STATUS;
    }
    break;
  case step::EXTENSION_DEVICE_STATUS:
    if (maple::dma_poll_complete()) {
      recv_extension_device_status(state);
      state.step = step::EXTENSION_DEVICE_REPLY;
    }
    break;
  case step::EXTENSION_DEVICE_REPLY:
    if (maple::dma_poll_complete()) {
      state.step = step::IDLE;
    }
    break;
  }
}

void render_glyphs(maple::display::font_renderer& renderer, char * s)
{
  for (int i = 0; i < 8 * 4; i++) {
    int x = i % 8;
    int y = i / 8;
    renderer.glyph(s[i], x, y);
  }
}

void render_serial_state(maple::display::font_renderer& renderer, char * s, const char * msg)
{
  bool end = false;
  for (int i = 0; i < 8; i++) {
    if (end || msg[i] == 0) {
      s[0 + i] = ' ';
      end = true;
    } else {
      s[0 + i] = msg[i];
    }
  }
  char num_buf[8];
  string::hex(num_buf, 8, sh7091.SCIF.SCFSR2);
  for (int i = 0; i < 8; i++) s[8 + i] = num_buf[i];
  string::hex(num_buf, 8, sh7091.SCIF.SCFDR2);
  for (int i = 0; i < 8; i++) s[16 + i] = num_buf[i];
  render_glyphs(renderer, s);
}

void main() __attribute__((section(".text.main")));

void main()
{
  serial::init(0);

  struct maple_display_poll_state state = {0};
  const uint8_t * font = reinterpret_cast<const uint8_t *>(&_binary_font_portfolio_6x8);
  auto renderer = maple::display::font_renderer(font);
  framebuffer = renderer.fb;
  char s[33] =
    "1562500 "  // 0
    "        "  // 8
    "        "  // 16
    "        "; // 24
  render_glyphs(renderer, s);
  state.want_start = 1;

  serial_load::init();

  // reset serial status
  sh7091.SCIF.SCFSR2 = 0;
  // reset line status
  sh7091.SCIF.SCLSR2 = 0;

  serial::string("ready\n");

  while (1) {
    using namespace scif;

    const uint16_t scfsr2 = sh7091.SCIF.SCFSR2;
    if (scfsr2 & scfsr2::brk::bit_mask) {
      render_serial_state(renderer, s, "brk");
      // clear framing error and break
    } else if (scfsr2 & scfsr2::er::bit_mask) {
      render_serial_state(renderer, s, "er");
    } else if (scfsr2 & scfsr2::dr::bit_mask) {
      render_serial_state(renderer, s, "dr");
    } else if (sh7091.SCIF.SCLSR2 & sclsr2::orer::bit_mask) {
      render_serial_state(renderer, s, "orer");
    } else if (scfsr2 & scfsr2::rdf::bit_mask) {
      render_serial_state(renderer, s, "rdf");
      const uint8_t c = sh7091.SCIF.SCFRDR2;
      serial_load::recv(c);
    } else {
      render_serial_state(renderer, s, "idle");
    }
    state.want_start = 1;

    handle_maple(state);

    uint16_t error_bits = scfsr2::er::bit_mask | scfsr2::brk::bit_mask;
    if (sh7091.SCIF.SCFSR2 & error_bits) {
      sh7091.SCIF.SCFSR2 = ~error_bits;
    }
  }
}
