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
#include "maple/storage.hpp"
#include "sh7091/serial.hpp"

#include "systembus.hpp"

struct storage_state {
  uint8_t * send_buf;
  uint8_t * recv_buf;
  uint32_t host_port_select;
  uint32_t destination_ap;
  struct {
    uint8_t partitions;
    uint16_t bytes_per_block;
    uint8_t write_accesses;
    uint8_t read_accesses;
  } device_status;
  struct {
    struct {
      uint16_t block_number;
    } system_area;
    struct {
      uint16_t block_number;
      uint16_t number_of_blocks;
    } fat_area;
    struct {
      uint16_t block_number;
      uint16_t number_of_blocks;
    } file_information;
  } media_info;
  storage::system_area system_area;
  storage::fat_area * fat_area;
  storage::file_information * file_information; // array

  uint32_t bytes_per_read_access()
  {
    // divide rounding up
    return (device_status.bytes_per_block + (device_status.read_accesses - 1)) / device_status.read_accesses;
  }

  uint32_t bytes_per_write_access()
  {
    // divide rounding up
    return (device_status.bytes_per_block + (device_status.write_accesses - 1)) / device_status.write_accesses;
  }

  uint32_t system_area_size()
  {
    return device_status.bytes_per_block * 1;
  }

  uint32_t fat_area_size()
  {
    return device_status.bytes_per_block * media_info.fat_area.number_of_blocks;
  }

  uint32_t file_information_size()
  {
    return device_status.bytes_per_block * media_info.file_information.number_of_blocks;
  }

  uint32_t file_information_entries()
  {
    return file_information_size() / (sizeof (struct storage::file_information));
  }
};

void parse_storage_function_definition(const uint32_t fd, storage_state& state)
{
  state.device_status.partitions = ((fd >> 24) & 0xff) + 1;
  state.device_status.bytes_per_block = (((fd >> 16) & 0xff) + 1) * 32;
  state.device_status.write_accesses = (fd >> 12) & 0xf;
  state.device_status.read_accesses = (fd >> 8) & 0xf;

  serial::string("      function_definition:\n");
  serial::string("        partitions: ");
  serial::integer(state.device_status.partitions);
  serial::string("        bytes_per_block: ");
  serial::integer(state.device_status.bytes_per_block);
  serial::string("        write_accesses: ");
  serial::integer(state.device_status.write_accesses);
  serial::string("        read_accesses: ");
  serial::integer(state.device_status.read_accesses);
}

template <typename T>
inline void copy(T * dst, const T * src, const int32_t n) noexcept
{
  int32_t n_t = n / (sizeof (T));
  while (n_t > 0) {
    *dst++ = *src++;
    n_t--;
  }
}

struct block_read_response {
  union responses {
    struct maple::file_error::data_fields file_error;
    struct maple::data_transfer<ft1::block_read_data_transfer::data_format>::data_fields data_transfer;
  };

  using data_fields = union responses;
};

bool do_block_read(storage_state& state, uint16_t block_number, uint8_t * dest)
{
  using command_type = maple::block_read;
  using response_type = block_read_response;

  auto writer = maple::host_command_writer(state.send_buf, state.recv_buf);

  maple::host_response<response_type::data_fields> * host_responses[state.device_status.read_accesses];
  for (int32_t phase = 0; phase < state.device_status.read_accesses; phase++) {
    bool end_flag = phase == (state.device_status.read_accesses - 1);
    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(state.host_port_select,
							   state.destination_ap,
							   end_flag,
							   0,                            // send_trailing
							   state.bytes_per_read_access() // recv_trailing
							   );

    auto& data_fields = host_command->bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::storage);
    data_fields.pt = 0;
    data_fields.phase = phase;
    data_fields.block_number = std::byteswap<uint16_t>(block_number);

    host_responses[phase] = host_response;

    //serial::string("read phase: ");
    //serial::integer<uint8_t>(phase);
    //serial::integer(writer.recv_offset);
  }

  maple::dma_start(state.send_buf, writer.send_offset,
		   state.recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  for (uint32_t phase = 0; phase < state.device_status.read_accesses; phase++) {
    auto& bus_data = host_responses[phase]->bus_data;
    if (bus_data.command_code != maple::data_transfer<uint8_t[0]>::command_code) {
      auto& file_error = bus_data.data_fields.file_error;
      serial::string("lm did not reply block_read: ");
      serial::integer<uint8_t>(bus_data.command_code);
      if (bus_data.command_code == maple::file_error::command_code) {
	serial::string("error: ");
	serial::hexlify(&file_error.function_error_code, 4);
	serial::character('\n');
      }
      return false;
    } else {
      auto& data_transfer = bus_data.data_fields.data_transfer;
      copy<uint8_t>(dest, data_transfer.data.block_data, state.bytes_per_read_access());
      dest += state.bytes_per_read_access();
    }
  }

  return true;
}

struct get_last_error_response {
  union responses {
    struct maple::device_reply device_reply;
    struct maple::file_error file_error;
  };

  using data_fields = union responses;
};

bool do_block_write(storage_state& state, uint16_t block_number, uint8_t * src)
{
  auto writer = maple::host_command_writer(state.send_buf, state.recv_buf);

  uint32_t phase;
  for (phase = 0; phase < state.device_status.write_accesses; phase++) {
    using command_type = maple::block_write<uint8_t[0]>;
    using response_type = maple::device_reply;

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(state.host_port_select,
							   state.destination_ap,
							   false,                          // end_flag
							   state.bytes_per_write_access(), // send_trailing
							   0                               // recv_trailing
							   );

    auto& data_fields = host_command->bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::storage);
    data_fields.pt = 0;
    data_fields.phase = phase;
    data_fields.block_number = std::byteswap<uint16_t>(block_number);

    copy<uint8_t>(data_fields.written_data, src, state.bytes_per_write_access());
    src += state.bytes_per_write_access();
    //serial::string("write phase: ");
    //serial::integer<uint8_t>(phase);
    //serial::integer(writer.send_offset);
  }

  using command_type = maple::get_last_error;
  using response_type = get_last_error_response;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(state.host_port_select,
							 state.destination_ap,
							 true); // end_flag

  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = std::byteswap(function_type::storage);
  data_fields.pt = 0;
  data_fields.phase = phase;
  data_fields.block_number = std::byteswap<uint16_t>(block_number);
  //serial::string("write phase: ");
  //serial::integer<uint8_t>(phase);
  //serial::integer(writer.send_offset);

  maple::dma_start(state.send_buf, writer.send_offset,
		   state.recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  serial::string("block write status: ");
  serial::integer(host_response->bus_data.command_code);

  return true;
}

uint16_t allocate_fat_chain(storage_state& state, uint16_t blocks)
{
  int32_t first_block = -1;
  int32_t last_block;
  //
  for (int32_t i = (state.fat_area_size() / (sizeof (uint16_t))) - 1; i >= 0; i--) {
    if (state.fat_area->fat_number[i] == storage::fat_area::data::unused) {
      if (first_block == -1) {
	first_block = i;
      } else {
	state.fat_area->fat_number[last_block] = i;
      }
      last_block = i;
      blocks -= 1;
    }

    if (blocks == 0) {
      state.fat_area->fat_number[last_block] = storage::fat_area::data::data_end;
      break;
    }
  }
  for (uint32_t i = 0; i < (state.fat_area_size() / (sizeof (uint16_t))); i++) {
    //serial::integer<uint16_t>(i, ' ');
    //serial::integer(state.fat_area->fat_number[i]);
  }

  return first_block;
}

bool allocate_file_information_data(storage_state& state,
				    uint16_t start_fat,
				    uint8_t const * const file_name,
				    uint16_t block_size)
{
  for (uint32_t i = 0; i < state.file_information_entries(); i++) {
    if (state.file_information[i].status == storage::file_information::status::no_data_file) {

      state.file_information[i].status = storage::file_information::status::data_file;
      state.file_information[i].copy = 0;
      state.file_information[i].start_fat = start_fat;
      copy<uint8_t>(state.file_information[i].file_name, file_name, 12);
      state.file_information[i].block_size = block_size;
      state.file_information[i].header = 0;
      state.file_information[i]._reserved = 0;

      return true;
    }
  }
  return false;
}

void do_lm_request(uint8_t port, uint8_t lm)
{
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  const uint32_t host_port_select = host_instruction_port_select(port);
  const uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  storage_state state;
  state.send_buf = send_buf;
  state.recv_buf = recv_buf;
  state.host_port_select = host_port_select;
  state.destination_ap = destination_ap;

  {
    using command_type = maple::device_request;
    using response_type = maple::device_status;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(host_port_select,
							   destination_ap,
							   true); // end_flag

    serial::string("lm command send_offset ");
    serial::integer<uint32_t>(writer.send_offset);
    serial::hexlify(send_buf, writer.send_offset);

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);
    maple::dma_wait_complete();

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    serial::hexlify(recv_buf, writer.recv_offset);
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

    parse_storage_function_definition(std::byteswap(data_fields.device_id.fd[2]), state);
  }

  {
    using command_type = maple::get_media_info;
    using response_type = maple::data_transfer<ft1::get_media_info_data_transfer::data_format>;

    auto writer = maple::host_command_writer(send_buf, recv_buf);

    auto [host_command, host_response]
      = writer.append_command<command_type, response_type>(host_port_select,
							   destination_ap,
							   true); // end_flag
    host_command->bus_data.data_fields.function_type = std::byteswap(function_type::storage);
    host_command->bus_data.data_fields.pt = 0;

    maple::dma_start(send_buf, writer.send_offset,
		     recv_buf, writer.recv_offset);
    maple::dma_wait_complete();

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    auto& data = data_fields.data;

    if (bus_data.command_code != response_type::command_code) {
      serial::string("lm did not reply: ");
      serial::integer<uint8_t>(port, ' ');
      serial::integer<uint8_t>(lm);
    } else {
      serial::string("      media_info:\n");
      serial::string("        total_size: ");
      serial::integer<uint32_t>(data.total_size);
      serial::string("        partition_number: ");
      serial::integer<uint32_t>(data.partition_number);
      serial::string("        system_area_block_number: ");
      serial::integer<uint32_t>(data.system_area_block_number);
      serial::string("        fat_area_block_number: ");
      serial::integer<uint32_t>(data.fat_area_block_number);
      serial::string("        number_of_fat_area_blocks: ");
      serial::integer<uint32_t>(data.number_of_fat_area_blocks);
      serial::string("        file_information_block_number: ");
      serial::integer<uint32_t>(data.file_information_block_number);
      serial::string("        number_of_file_information_blocks: ");
      serial::integer<uint32_t>(data.number_of_file_information_blocks);
      serial::string("        volume_icon: ");
      serial::integer<uint32_t>(data.volume_icon);
      serial::string("        save_area_block_number: ");
      serial::integer<uint32_t>(data.save_area_block_number);
      serial::string("        number_of_save_area_blocks: ");
      serial::integer<uint32_t>(data.number_of_save_area_blocks);
      serial::string("        reserved_for_execution_file: ");
      serial::integer<uint32_t>(data.reserved_for_execution_file);
    }

    state.media_info.system_area.block_number = data.system_area_block_number;
    state.media_info.fat_area.block_number = data.fat_area_block_number;
    state.media_info.fat_area.number_of_blocks = data.number_of_fat_area_blocks;
    state.media_info.file_information.block_number = data.file_information_block_number;
    state.media_info.file_information.number_of_blocks = data.number_of_file_information_blocks;
  }

  uint8_t fat_area_data[state.fat_area_size()];
  uint8_t file_information_data[state.file_information_size()];

  {
    do_block_read(state, state.media_info.system_area.block_number,
		  reinterpret_cast<uint8_t *>(&state.system_area));
    serial::string("      system_area:\n");
    serial::string("        format_information: ");
    serial::hexlify(state.system_area.format_information, 16);
    serial::string("        volume_label: ");
    serial::hexlify(state.system_area.volume_label, 32);
    serial::string("        date_and_time_created: ");
    serial::hexlify(state.system_area.date_and_time_created, 8);
    serial::string("        total_size: ");
    serial::integer(state.system_area.total_size);
  }

  {
    uint8_t * bufi = fat_area_data;
    uint16_t chain = state.media_info.fat_area.block_number;
    state.fat_area = reinterpret_cast<storage::fat_area *>(fat_area_data);
    do {
      do_block_read(state, chain, bufi);
      bufi += state.device_status.bytes_per_block;
      chain = state.fat_area->fat_number[chain];
    } while (chain != storage::fat_area::data::data_end);
    serial::string("      read fat_area bytes: ");
    serial::integer((uint32_t)(bufi - fat_area_data));

    uint32_t count = 0;
    for (uint32_t i = 0; i < (bufi - fat_area_data) / (sizeof (uint16_t)); i++) {
      if (state.fat_area->fat_number[i] == 0xfffc) {
	count += 1;
      }
      //serial::integer<uint16_t>(i, ' ');
      //serial::integer(state.fat_area->fat_number[i]);
    }
    serial::string("        free blocks: ");
    serial::integer(count);
  }

  {
    uint8_t * bufi = file_information_data;
    uint16_t chain = state.media_info.file_information.block_number;
    state.file_information = reinterpret_cast<storage::file_information *>(file_information_data);
    do {
      do_block_read(state, chain, bufi);
      bufi += state.device_status.bytes_per_block;
      chain = state.fat_area->fat_number[chain];
    } while (chain != storage::fat_area::data::data_end);
    serial::string("      read file_information bytes: ");
    serial::integer((uint32_t)(bufi - file_information_data));

    for (uint32_t i = 0; i < state.file_information_entries(); i++) {
      if (state.file_information[i].status == storage::file_information::status::data_file
	  || state.file_information[i].status == storage::file_information::status::execution_file) {
	serial::string("        file_name: ");
	serial::string(state.file_information[i].file_name, 12);
	serial::character('\n');
	serial::string("          status: ");
	serial::integer<uint16_t>(state.file_information[i].status);
	serial::string("          start_fat: ");
	serial::integer<uint16_t>(state.file_information[i].start_fat);
	serial::string("          block_size: ");
	serial::integer<uint16_t>(state.file_information[i].block_size);
      }
    }
  }

  {
    uint16_t block_size = 3;
    uint16_t start_fat = allocate_fat_chain(state, block_size);
    char const * file_name = "HELLOANA.SUP";
    allocate_file_information_data(state, start_fat,
				   reinterpret_cast<uint8_t const *>(file_name),
				   block_size);

    {
      uint16_t chain = state.media_info.fat_area.block_number;
      uint8_t * buf = reinterpret_cast<uint8_t *>(state.fat_area);
      do {
	do_block_write(state, chain, buf);
	buf += state.device_status.bytes_per_block;
	chain = state.fat_area->fat_number[chain];
      } while (chain != storage::fat_area::data::data_end);
    }

    {
      uint16_t chain = state.media_info.file_information.block_number;
      uint8_t * buf = reinterpret_cast<uint8_t *>(state.file_information);
      do {
	do_block_write(state, chain, buf);
	buf += state.device_status.bytes_per_block;
	chain = state.fat_area->fat_number[chain];
      } while (chain != storage::fat_area::data::data_end);
    }
  }

  {
    uint8_t * bufi = fat_area_data;
    uint16_t chain = state.media_info.fat_area.block_number;
    state.fat_area = reinterpret_cast<storage::fat_area *>(fat_area_data);
    do {
      do_block_read(state, chain, bufi);
      bufi += state.device_status.bytes_per_block;
      chain = state.fat_area->fat_number[chain];
    } while (chain != storage::fat_area::data::data_end);
    serial::string("      read fat_area bytes: ");
    serial::integer((uint32_t)(bufi - fat_area_data));

    uint32_t count = 0;
    for (uint32_t i = 0; i < (bufi - fat_area_data) / (sizeof (uint16_t)); i++) {
      if (state.fat_area->fat_number[i] == 0xfffc) {
	count += 1;
      }
      serial::integer<uint16_t>(i, ' ');
      serial::integer(state.fat_area->fat_number[i]);
    }
    serial::string("        free blocks: ");
    serial::integer(count);
    allocate_fat_chain(state, 98);
  }
}

void do_lm_requests(uint8_t port, uint8_t lm)
{
  if (lm & ap::lm_bus::_0)
    do_lm_request(port, ap::lm_bus::_0);
  if (lm & ap::lm_bus::_1)
    do_lm_request(port, ap::lm_bus::_1);
  if (lm & ap::lm_bus::_2)
    do_lm_request(port, ap::lm_bus::_2);
  if (lm & ap::lm_bus::_3)
    do_lm_request(port, ap::lm_bus::_3);
  if (lm & ap::lm_bus::_4)
    do_lm_request(port, ap::lm_bus::_4);
}

void do_device_request()
{
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::device_request;
  using response_type = maple::device_status;

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
  serial::init(4);

  do_device_request();

  while (1);
}
