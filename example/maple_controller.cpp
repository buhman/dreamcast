#include <bit>

#include "vga.hpp"
#include "align.hpp"

#include "maple/maple.hpp"
#include "maple/maple_impl.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "sh7091/serial.hpp"

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

  const uint32_t size = maple::init_get_condition(command_buf, receive_buf,
                                                  destination_port,
                                                  destination_ap,
                                                  std::byteswap(function_type::controller));
  maple::dma_start(command_buf, size);

  using response_type = data_transfer<ft0::data_transfer::data_format>;
  using command_response_type = struct maple::command_response<response_type::data_fields>;
  auto response = reinterpret_cast<command_response_type *>(receive_buf);
  auto& bus_data = response->bus_data;
  if (bus_data.command_code != response_type::command_code) {
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
  using command_type = device_request;
  using response_type = device_status;

  const uint32_t size = maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf);
  maple::dma_start(command_buf, size * 10);

  using command_response_type = struct maple::command_response<response_type::data_fields>;
  auto response = reinterpret_cast<command_response_type *>(receive_buf);
  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = response[port].bus_data;
    auto& data_fields = response[port].bus_data.data_fields;
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
  command_buf = reinterpret_cast<uint32_t *>(reinterpret_cast<uint32_t>(command_buf) | 0xa000'0000);
  receive_buf = align_32byte(_receive_buf);

  // flycast needs this in HLE mode, or else it won't start the vcount
  // counter.
  vga();

  while (1) {
    v_sync_in();
    do_device_request();
  };
}
