#include <bit>

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_commands.hpp"
#include "sh7091/serial.hpp"

void main()
{
  serial::init(0);

  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto writer = maple::host_command_writer<>(send_buf, recv_buf);

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != response_type::command_code) {
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
    }
  }

  //while (1);
}
