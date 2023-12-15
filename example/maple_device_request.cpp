#include "align.hpp"

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "serial.hpp"

constexpr uint32_t command_data_size = (sizeof (device_request::data_fields));
constexpr uint32_t response_data_size = (sizeof (device_status::data_fields));

constexpr uint32_t host_command_size = (sizeof (struct maple::host_command<device_request::data_fields>));
constexpr uint32_t command_response_size = align_32byte((sizeof (struct maple::command_response<device_status::data_fields>)));

uint32_t _command_buf[host_command_size * 4 + 32] = {0};
uint32_t _receive_buf[command_response_size * 4 + 32] = {0};

void main()
{
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

  maple::init_host_command_all_ports(command_buf, receive_buf,
                                     device_request::command_code, command_data_size, response_data_size);

  maple::dma_start(command_buf);

  uint8_t * buf = reinterpret_cast<uint8_t *>(receive_buf);
  for (uint8_t port = 0; port < 4; port++) {
    serial::string("port ");
    serial::integer<uint8_t>(port);
    for (uint32_t i = 0; i < command_response_size; i++) {
      serial::integer<uint8_t>(buf[port * command_response_size + i]);
    }
    serial::character('\n');
  }

  while (1);
}
