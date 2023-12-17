#include <bit>

#include "vga.hpp"
#include "align.hpp"

#include "maple/maple.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "serial.hpp"

uint32_t _command_buf[1024 / 4 + 32] = {0};
uint32_t _receive_buf[1024 / 4 + 32] = {0};

static uint32_t * command_buf;
static uint32_t * receive_buf;

void do_get_condition(uint32_t port)
{
  uint32_t destination_port;
  uint32_t destination_ap;

  switch (port) {
  case 0:
    destination_port = host_instruction::port_select::a;
    destination_ap = ap::de::device | ap::port_select::a;
    break;
  case 1:
    destination_port = host_instruction::port_select::b;
    destination_ap = ap::de::device | ap::port_select::b;
    break;
  case 2:
    destination_port = host_instruction::port_select::c;
    destination_ap = ap::de::device | ap::port_select::c;
    break;
  case 3:
    destination_port = host_instruction::port_select::d;
    destination_ap = ap::de::device | ap::port_select::d;
    break;
  default:
    return;
  }

  maple::init_get_condition(command_buf, receive_buf,
			    destination_port,
			    destination_ap,
			    std::byteswap(function_type::controller));
  maple::dma_start(command_buf);

  using response_type = struct maple::command_response<data_transfer::data_fields<struct ft0::data_transfer::data_format>>;
  auto response = reinterpret_cast<response_type *>(receive_buf);
  auto& bus_data = response->bus_data;
  if (bus_data.command_code != data_transfer::command_code) {
    return;
  }
  auto& data_fields = bus_data.data_fields;
  if ((data_fields.function_type & std::byteswap(function_type::controller)) == 0) {
    return;
  }

  bool a = ft0::data_transfer::digital_button::a(data_fields.data.digital_button);
  if (a == 0) {
    serial::string("port ");
    serial::integer<uint8_t>(port);
    serial::string("  `a` press ");
    serial::integer<uint8_t>(a);
  }
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
	//serial::string("is controller: ");
	//serial::integer<uint8_t>(port);
	do_get_condition(port);
      }
    }
  }
}

void main()
{
  command_buf = align_32byte(_command_buf);
  receive_buf = align_32byte(_receive_buf);

  // flycast needs this in HLE mode, or else it won't start the vcount
  // counter.
  vga();

  while (1) {
    v_sync_out();
    v_sync_in();
    do_device_request();
  };
}
