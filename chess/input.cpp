#include <bit>

#include "maple/maple.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "maple/maple_bus_ft9.hpp"
#include "maple/maple_host_command_writer.hpp"

#include "sh7091/serial.hpp"

#include "input.hpp"

namespace input {

struct input_state state;

void do_get_condition_controller(maple::host_command_writer& writer,
				 const uint32_t port)
{
  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::device;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 false); // end_flag

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::controller);

  state.port[port].host_response_data_transfer_ft0 = host_response;
}

void do_get_condition_pointing(maple::host_command_writer& writer,
			       const uint32_t port)
{
  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft9::data_transfer::data_format>;

  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::device;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
							 destination_ap,
							 false); // end_flag

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::pointing);

  state.port[port].host_response_data_transfer_ft9 = host_response;
}

maple::host_response<maple::device_status::data_fields> * do_device_request(maple::host_command_writer& writer)
{
  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  return host_response;
}

void state_update(uint32_t * send_buf, uint32_t * recv_buf)
{
  maple::host_command_writer writer(send_buf, recv_buf);

  for (uint8_t port = 0; port < 4; port++) {
    state.port[port].function_type = state.port[port].next_function_type;

    if (state.port[port].function_type & function_type::controller) {
      do_get_condition_controller(writer, port);
    }

    if (state.port[port].function_type & function_type::pointing) {
      do_get_condition_pointing(writer, port);
    }
  }
  auto host_response_device_status = do_device_request(writer);

  maple::dma_start(writer.send_buf, writer.send_offset,
                   writer.recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response_device_status[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != maple::device_status::command_code) {
      state.port[port].function_type = 0;
      state.port[port].next_function_type = 0;
      continue;
    } else {
      state.port[port].next_function_type = std::byteswap(data_fields.device_id.ft);
    }
  }
}

void state_init()
{
  for (uint8_t port = 0; port < 4; port++) {
    state.port[port].function_type = 0;
  }
}

}
