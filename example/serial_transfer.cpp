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

#include "crc32.h"

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

static uint8_t * framebuffer[2];
static char textbuffer[2][4 * 8];

static uint32_t send_buf[1024 / 4] __attribute__((aligned(32)));
static uint32_t recv_buf[1024 / 4] __attribute__((aligned(32)));

struct serial_error_counter {
  uint32_t brk;
  uint32_t er;
  uint32_t dr;
  uint32_t orer;
};

struct serial_error_counter error_counter;

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

  uint8_t * fb;
  if (lm == ap::lm_bus::_0)
    fb = framebuffer[0 ^ (port & 1)];
  else
    fb = framebuffer[1 ^ (port & 1)];

  copy<uint8_t>(data_fields.written_data, fb, maple::display::vmu::framebuffer_size);
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

void render_glyphs(maple::display::font_renderer& renderer, const char * s)
{
  for (int i = 0; i < 8 * 4; i++) {
    int x = i % 8;
    int y = i / 8;
    renderer.glyph(s[i], x, y);
  }
}

void render_str(const char * src, char * dst)
{
  int i = 0;
  char c;
  while ((c = src[i]) != 0) {
    dst[i] = c;
    i++;
  }
}

void render_line(const char * src, char * dst, int src_len = 8)
{
  bool end = false;
  for (int i = 0; i < 8; i++) {
    if (end || src[i] == 0 || i >= src_len) {
      dst[i] = ' ';
      end = true;
    } else {
      dst[i] = src[i];
    }
  }
}

void render_clear(char * dst, int len)
{
  for (int i = 0; i < len; i++)
    dst[i] = ' ';
}

void render_u32(uint32_t src, char * dst)
{
  char num_buf[8];
  string::hex(num_buf, 8, src);
  for (int i = 0; i < 8; i++) dst[i] = num_buf[i];
}

void render_u8(uint32_t src, char * dst)
{
  char num_buf[2];
  string::hex(num_buf, 2, src);
  for (int i = 0; i < 2; i++) dst[i] = num_buf[i];
}

void render_idle_state(char * dst)
{
  render_line("idle", &dst[0]);
  render_u32(serial_load::state.buf.u32[0], &dst[8]);
  render_u32(serial_load::state.buf.u32[1], &dst[16]);
  render_u8(serial_load::state.len, &dst[24]);
  render_clear(&dst[26], 6);
}

void render_write_state(char * dst)
{
  render_line("write", &dst[0]);
  render_u32(sh7091.DMAC.DMATCR1, &dst[8]);
  render_u32(sh7091.DMAC.DAR1, &dst[16]);
  render_u32(serial_load::state.write_crc.value, &dst[24]);
}

void render_fsm_state(char * dst)
{
  using namespace serial_load;
  switch (state.fsm_state) {
  case fsm_state::idle:
    render_idle_state(dst);
    break;
  case fsm_state::write:
    render_write_state(dst);
    break;
  case fsm_state::read:
    render_line("read", &dst[0]);
    render_clear(&dst[8], 24);
    break;
  case fsm_state::jump:
    render_line("jump", &dst[0]);
    render_clear(&dst[8], 24);
    break;
  case fsm_state::speed:
    render_line("speed", &dst[0]);
    render_clear(&dst[8], 24);
    break;
  default:
    render_line("invalid", &dst[0]);
    render_clear(&dst[8], 24);
    break;
  }
}

void render_error_counter_state(char * dst)
{
  render_str("br", &dst[0]);
  render_u8(error_counter.brk, &dst[2]);
  render_str("er", &dst[4]);
  render_u8(error_counter.er, &dst[6]);
  render_str("dr", &dst[8]);
  render_u8(error_counter.dr, &dst[10]);
  render_str("oe", &dst[12]);
  render_u8(error_counter.orer, &dst[14]);

  render_u32(sh7091.SCIF.SCFSR2, &dst[16]);
  render_u32(sh7091.SCIF.SCLSR2, &dst[24]);
}

void render(maple::display::font_renderer& renderer0,
	    maple::display::font_renderer& renderer1)
{
  render_fsm_state(textbuffer[0]);
  render_error_counter_state(textbuffer[1]);
  render_glyphs(renderer0, textbuffer[0]);
  render_glyphs(renderer1, textbuffer[1]);
}

void main() __attribute__((section(".text.main")));

void main()
{
  serial::init(0);

  struct maple_display_poll_state state = {0};
  const uint8_t * font = reinterpret_cast<const uint8_t *>(&_binary_font_portfolio_6x8);
  auto renderer0 = maple::display::font_renderer(font);
  framebuffer[0] = renderer0.fb;
  auto renderer1 = maple::display::font_renderer(font);
  framebuffer[1] = renderer1.fb;

  state.want_start = 1;
  serial_load::init();

  // reset serial status
  sh7091.SCIF.SCFSR2 = 0;
  // reset line status
  sh7091.SCIF.SCLSR2 = 0;

  uint32_t last_scfsr2 = -1;
  uint32_t last_sclsr2 = -1;

  serial::string("ready\n");

  error_counter.brk = 0;
  error_counter.er = 0;
  error_counter.dr = 0;
  error_counter.orer = 0;

  while (1) {
    using namespace scif;

    const uint16_t scfsr2 = sh7091.SCIF.SCFSR2;
    const uint16_t sclsr2 = sh7091.SCIF.SCLSR2;

    if (scfsr2 & scfsr2::brk::bit_mask) {
      // clear framing error and break
      error_counter.brk += 1;
      error_counter.er += (scfsr2 & scfsr2::er::bit_mask) != 0;
      sh7091.SCIF.SCFSR2 = scfsr2 & ~(scfsr2::brk::bit_mask | scfsr2::er::bit_mask);
      serial_load::init();
      serial::reset_txrx();
    } else if (scfsr2 & scfsr2::er::bit_mask) {
      // clear framing error
      error_counter.er += 1;
      sh7091.SCIF.SCFSR2 = scfsr2 & ~scfsr2::er::bit_mask;
    } else if (scfsr2 & scfsr2::dr::bit_mask) {
      error_counter.dr += 1;
      sh7091.SCIF.SCFSR2 = scfsr2 & ~scfsr2::dr::bit_mask;
    } else if (sclsr2 & sclsr2::orer::bit_mask) {
      error_counter.orer += 1;
      sh7091.SCIF.SCLSR2 = scfsr2 & ~sclsr2::orer::bit_mask;
    } else if (scfsr2 & scfsr2::rdf::bit_mask) {
      if (serial_load::state.fsm_state == serial_load::fsm_state::idle) {
	while (sh7091.SCIF.SCFSR2 & scfsr2::rdf::bit_mask) {
	  const uint8_t c = sh7091.SCIF.SCFRDR2;
	  serial_load::recv(c);
	  sh7091.SCIF.SCFSR2 = scfsr2 & ~scfsr2::rdf::bit_mask;
	}
      }
    }

    serial_load::tick();
    render(renderer0, renderer1);

    //if (sclsr2 != last_sclsr2 || scfsr2 != last_scfsr2) {
      state.want_start = 1;
    //}
    last_scfsr2 = scfsr2;
    last_sclsr2 = sclsr2;
    handle_maple(state);
  }
}
