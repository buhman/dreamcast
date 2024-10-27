#include <bit>

#include "align.hpp"
#include "holly/video_output.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft8.hpp"
#include "sh7091/serial.hpp"

uint32_t send_buf[1024] __attribute__((aligned(32)));
uint32_t recv_buf[1024] __attribute__((aligned(32)));

void do_lm_request(uint8_t port, uint8_t lm)
{
  uint32_t destination_port;
  uint32_t destination_ap;

  switch (port) {
  case 0:
    destination_port = host_instruction::port_select::a;
    destination_ap = ap::de::expansion_device | ap::port_select::a | lm;
    break;
  case 1:
    destination_port = host_instruction::port_select::b;
    destination_ap = ap::de::expansion_device | ap::port_select::b | lm;
    break;
  case 2:
    destination_port = host_instruction::port_select::c;
    destination_ap = ap::de::expansion_device | ap::port_select::c | lm;
    break;
  case 3:
    destination_port = host_instruction::port_select::d;
    destination_ap = ap::de::expansion_device | ap::port_select::d | lm;
    break;
  default:
    return;
  }

  /*
    get media info
   */

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_media_info;
  using response_type = maple::data_transfer<ft8::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(destination_port,
							 destination_ap,
							 true,      // end_flag
							 0, // send_trailing
							 0  // recv_trailing
							 );
  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = std::byteswap(function_type::vibration);
  data_fields.pt = std::byteswap(1 << 24);

  serial::string("dma start\n");
  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  auto& bus_data = host_response->bus_data;

  if (bus_data.command_code != response_type::command_code) {
    serial::string("lm did not reply to vibration get_media_info: ");
    serial::integer<uint8_t>(lm);
    return;
  } else {
    auto& data_fields = bus_data.data_fields;
    serial::string("lm replied to vibration get_media_info: ");
    serial::integer<uint8_t>(lm);

    using namespace ft8::data_transfer::vset;
    serial::string("vn: ");
    serial::integer<uint8_t>(vn(data_fields.data.vset));
    serial::string("vp: ");
    serial::integer<uint8_t>(vp(data_fields.data.vset));
    serial::string("vd: ");
    serial::integer<uint8_t>(vd(data_fields.data.vset));
    serial::string("pf: ");
    serial::integer<uint8_t>(pf(data_fields.data.vset));
    serial::string("cv: ");
    serial::integer<uint8_t>(cv(data_fields.data.vset));
    serial::string("pd: ");
    serial::integer<uint8_t>(pd(data_fields.data.vset));
    serial::string("owf: ");
    serial::integer<uint8_t>(owf(data_fields.data.vset));
    serial::string("va: ");
    serial::integer<uint8_t>(va(data_fields.data.vset));
    serial::string("\nfm0 (fmin): ");
    serial::integer<uint8_t>(data_fields.data.fm0);
    serial::string("fm1 (fmax): ");
    serial::integer<uint8_t>(data_fields.data.fm1);
  }

  /*
     set condition
   */
  {
    using command_type = maple::set_condition<ft8::set_condition::data_format>;
    using response_type = maple::device_reply;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(destination_port,
							   destination_ap,
							   true,      // end_flag
							   0, // send_trailing
							   0  // recv_trailing
							   );
    auto& data_fields = host_command->bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::vibration);
    data_fields.write_in_data.ctrl = 0x11;
    data_fields.write_in_data.pow = 0x70;
    data_fields.write_in_data.freq = 0x27;
    data_fields.write_in_data.inc = 0x00;

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);
    maple::dma_wait_complete();

    auto& bus_data = host_response->bus_data;

    if (bus_data.command_code != maple::device_reply::command_code) {
      serial::string("lm did not reply to vibration set_condition: ");
      serial::integer<uint8_t>(lm);
    } else {
      serial::string("lm replied to vibration set_condition: ");
      serial::integer<uint8_t>(lm);
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
  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  uint8_t port__ap[4];

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;

    port__ap[port] = 0;

    if (bus_data.command_code != maple::device_status::command_code) {
      // the controller is disconnected
    } else {
      if ((data_fields.device_id.ft & std::byteswap(function_type::controller)) != 0) {
	serial::string("controller: ");
	serial::integer<uint8_t>(port);
	//serial::integer<uint8_t>(bus_data.source_ap & ap::lm_bus::bit_mask);
	port__ap[port] = bus_data.source_ap & ap::lm_bus::bit_mask;
      }
    }
  }

  for (uint8_t port = 0; port < 4; port++) {
    if (port__ap[port] != 0)
      do_lm_requests(port, port__ap[port]);
  }
}

void main()
{
  serial::init(0);

  do_device_request();
}
