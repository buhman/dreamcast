#include <bit>

#include "vga.hpp"
#include "align.hpp"

#include "maple/maple.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft8.hpp"
#include "serial.hpp"

uint32_t _command_buf[1024 / 4 + 32] = {0};
uint32_t _receive_buf[1024 / 4 + 32] = {0};

static uint32_t * command_buf;
static uint32_t * receive_buf;

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

  maple::init_host_command(command_buf, receive_buf,
			   destination_port,
			   destination_ap, get_media_info::command_code, (sizeof (struct get_media_info::data_fields)),
			   true);

  using command_type = struct maple::host_command<get_media_info::data_fields>;
  auto host_command = reinterpret_cast<command_type *>(command_buf);
  auto& fields = host_command->bus_data.data_fields;
  fields.function_type = std::byteswap(function_type::vibration);
  fields.pt = std::byteswap(1 << 24);

  maple::dma_start(command_buf);

  using response_type = struct maple::command_response<data_transfer::data_fields<struct ft8::data_transfer::data_format>>;
  auto response = reinterpret_cast<response_type *>(receive_buf);
  auto& bus_data = response->bus_data;
  if (bus_data.command_code != data_transfer::command_code) {
    serial::string("lm did not reply to vibration get_media_info: ");
    serial::integer<uint8_t>(lm);
    return;
  }
  serial::string("lm replied to vibration get_media_info: ");
  serial::integer<uint8_t>(lm);

  auto& data_fields = bus_data.data_fields;

  {
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
    using data_field_type = set_condition::data_fields<ft8::set_condition::data_format>;

    maple::init_host_command(command_buf, receive_buf,
			     destination_port,
			     destination_ap, set_condition::command_code, (sizeof (data_field_type)),
			     true);

    using command_type = struct maple::host_command<data_field_type>;
    auto host_command = reinterpret_cast<command_type *>(command_buf);
    auto& fields = host_command->bus_data.data_fields;
    fields.function_type = std::byteswap(function_type::vibration);
    fields.write_in_data.ctrl = 0x11;
    fields.write_in_data.pow = 0x70;
    fields.write_in_data.freq = 0x27;
    fields.write_in_data.inc = 0x00;

    maple::dma_start(command_buf);

    using response_type = struct maple::command_response<device_reply::data_fields>;
    auto response = reinterpret_cast<response_type *>(receive_buf);
    auto& bus_data = response->bus_data;

    if (bus_data.command_code != device_reply::command_code) {
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
  using response_type = struct maple::command_response<device_status::data_fields>;
  constexpr uint32_t response_size = align_32byte(sizeof (response_type));

  maple::init_host_command_all_ports(command_buf, receive_buf,
                                     device_request::command_code,
				     (sizeof (device_request::data_fields)), // command_data_size
				     (sizeof (device_status::data_fields))); // response_data_size
  maple::dma_start(command_buf);

  for (uint8_t port = 0; port < 4; port++) {
    auto response = reinterpret_cast<response_type *>(&receive_buf[response_size * port / 4]);

    auto& bus_data = response->bus_data;
    auto& data_fields = response->bus_data.data_fields;
    if (bus_data.command_code != device_status::command_code) {
      // the controller is disconnected
    } else {
      if ((data_fields.device_id.ft & std::byteswap(function_type::controller)) != 0) {
	serial::string("controller: ");
	serial::integer<uint8_t>(port);
	//serial::integer<uint8_t>(bus_data.source_ap & ap::lm_bus::bit_mask);
	do_lm_requests(port, bus_data.source_ap & ap::lm_bus::bit_mask);
      }
    }
  }
}

void main()
{
  command_buf = align_32byte(_command_buf);
  receive_buf = align_32byte(_receive_buf);

  vga();

  while (1) {
    for (int i = 0; i < 120; i++) {
      v_sync_out();
      v_sync_in();
    }
    do_device_request();
  };
}
