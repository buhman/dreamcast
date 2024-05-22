#include <bit>

#include "align.hpp"
#include "holly/video_output.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "maple/maple.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "sh7091/serial.hpp"

#include "systembus.hpp"

void do_get_condition()
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = get_condition;
  using response_type = data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::controller);

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;

    if (bus_data.command_code != response_type::command_code) {
      //serial::string("device did not reply to get_condition: ");
      //serial::integer<uint8_t>(port);
    } else if ((std::byteswap(data_fields.function_type) & function_type::controller) != 0) {
      bool a = ft0::data_transfer::digital_button::a(data_fields.data.digital_button);
      if (a == 0) {
	serial::string("port ");
	serial::integer<uint8_t>(port);
	serial::string("  `a` press ");
	serial::integer<uint8_t>(a);
      }
    }
  }
}

void do_lm_request(uint8_t port, uint8_t lm)
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = device_request;
  using response_type = device_status;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 true); // end_flag

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  auto& bus_data = host_response->bus_data;
  auto& data_fields = bus_data.data_fields;
  if (bus_data.command_code != device_status::command_code) {
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

  using command_type = device_request;
  using response_type = device_status;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != device_status::command_code) {
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
  // flycast needs this in HLE mode, or else it won't start the vcount
  // counter.
  video_output::set_mode_vga();

  do_device_request();

  while (1) {
    while (!spg_status::vsync(holly.SPG_STATUS));
    while (spg_status::vsync(holly.SPG_STATUS));

    do_get_condition();
  };
}
