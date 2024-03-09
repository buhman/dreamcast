#include "align.hpp"

#include "maple/maple.hpp"
#include "maple/maple_impl.hpp"
#include "maple/maple_bus_commands.hpp"
#include "sh7091/serial.hpp"

uint32_t _command_buf[(1024 + 32) / 4];
uint32_t _receive_buf[(1024 + 32) / 4];

void main()
{
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

  using command_type = device_request;
  using response_type = device_status;

  uint32_t command_size = maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf);

  constexpr uint32_t host_response_size = (sizeof (maple::command_response<response_type::data_fields>));

  maple::dma_start(command_buf, command_size,
                   receive_buf, host_response_size);

  uint8_t * buf = reinterpret_cast<uint8_t *>(receive_buf);
  for (uint8_t port = 0; port < 4; port++) {
    serial::string("port ");
    serial::integer<uint8_t>(port);

    for (uint32_t i = 0; i < host_response_size; i++) {
      serial::integer<uint8_t>(buf[port * host_response_size + i]);
    }
    serial::character('\n');
  }

  while (1);
}
