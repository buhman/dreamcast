#include <bit>

#include "align.hpp"
#include "holly/video_output.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "maple/maple.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft1.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "sh7091/serial.hpp"

#include "systembus.hpp"

void do_lm_request(uint8_t port, uint8_t lm)
{
  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  const uint32_t host_port_select = host_instruction_port_select(port);
  const uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  {
    using command_type = device_request;
    using response_type = device_status;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(host_port_select,
							   destination_ap,
							   true); // end_flag

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != response_type::command_code) {
      serial::string("lm did not reply: ");
      serial::integer<uint8_t>(port, ' ');
      serial::integer<uint8_t>(lm);
    } else {
      serial::string("    lm: ");
      serial::integer<uint8_t>(lm);
      serial::string("      ft:    ");
      serial::hexlify(&data_fields.device_id.ft, 4);
      serial::string("      fd[0]: ");
      serial::hexlify(&data_fields.device_id.fd[0], 4);
      serial::string("      fd[1]: ");
      serial::hexlify(&data_fields.device_id.fd[1], 4);
      serial::string("      fd[2]: ");
      serial::hexlify(&data_fields.device_id.fd[2], 4);
      serial::string("      source_ap.lm_bus: ");
      serial::integer(bus_data.source_ap & ap::lm_bus::bit_mask);
    }

    if ((std::byteswap(data_fields.device_id.ft) & function_type::storage) == 0) {
      return;
    }
  }

  {
    using command_type = get_media_info;
    using response_type = data_transfer<ft1::get_media_info_data_transfer::data_format>;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(host_port_select,
							   destination_ap,
							   true); // end_flag
    host_command->bus_data.data_fields.function_type = std::byteswap(function_type::storage);
    host_command->bus_data.data_fields.pt = 0;

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    auto& data = data_fields.data;

    if (bus_data.command_code != response_type::command_code) {
      serial::string("lm did not reply: ");
      serial::integer<uint8_t>(port, ' ');
      serial::integer<uint8_t>(lm);
    } else {
      serial::string("      total_size: ");
      serial::integer<uint32_t>(data.total_size);
      serial::string("      partition_number: ");
      serial::integer<uint32_t>(data.partition_number);
      serial::string("      system_area_block_number: ");
      serial::integer<uint32_t>(data.system_area_block_number);
      serial::string("      fat_area_block_number: ");
      serial::integer<uint32_t>(data.fat_area_block_number);
      serial::string("      number_of_fat_area_blocks: ");
      serial::integer<uint32_t>(data.number_of_fat_area_blocks);
      serial::string("      file_information_block_number: ");
      serial::integer<uint32_t>(data.file_information_block_number);
      serial::string("      number_of_file_information_blocks: ");
      serial::integer<uint32_t>(data.number_of_file_information_blocks);
      serial::string("      volume_icon: ");
      serial::integer<uint32_t>(data.volume_icon);
      serial::string("      save_area_block_number: ");
      serial::integer<uint32_t>(data.save_area_block_number);
      serial::string("      number_of_save_area_blocks: ");
      serial::integer<uint32_t>(data.number_of_save_area_blocks);
      serial::string("      reserved_for_execution_file: ");
      serial::integer<uint32_t>(data.reserved_for_execution_file);
    }
  }

  {
    using command_type = block_read;
    using response_type = data_transfer<ft1::block_read_data_transfer::data_format<512>>;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(host_port_select,
							   destination_ap,
							   true); // end_flag
    host_command->bus_data.data_fields.function_type = std::byteswap(function_type::storage);
    host_command->bus_data.data_fields.pt = 0;
    host_command->bus_data.data_fields.phase = 0;
    host_command->bus_data.data_fields.block_number = std::byteswap<uint16_t>(0xff);

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    auto& data = data_fields.data;
    serial::integer(std::byteswap(host_command->bus_data.data_fields.block_number));
    if (bus_data.command_code != response_type::command_code) {
      serial::string("lm did not reply block_read: ");
      serial::integer<uint8_t>(port, ' ');
      serial::integer<uint8_t>(lm, ' ');
      serial::integer<uint8_t>(bus_data.command_code);
      auto error = reinterpret_cast<file_error::data_fields *>(&data_fields);
      serial::hexlify(&error->function_error_code, 4);
    } else {
      for (int i = 0; i < 512; i++) {
	serial::hexlify(data.block_data[i]);
	serial::character(' ');
	if (i % 16 == 15) {
	  serial::character('\n');
	}
      }
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
    if (bus_data.command_code != response_type::command_code) {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  disconnected\n");
    } else {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  ft:    ");
      serial::hexlify(&data_fields.device_id.ft, 4);
      serial::string("  fd[0]: ");
      serial::hexlify(&data_fields.device_id.fd[0], 4);
      serial::string("  fd[1]: ");
      serial::hexlify(&data_fields.device_id.fd[1], 4);
      serial::string("  fd[2]: ");
      serial::hexlify(&data_fields.device_id.fd[2], 4);
      serial::string("  source_ap.lm_bus: ");
      serial::integer(bus_data.source_ap & ap::lm_bus::bit_mask);

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
}
