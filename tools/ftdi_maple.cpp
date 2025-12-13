#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <bit>

#include "maple/maple_bus_bits.hpp"
#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_port.hpp"
#include "maple/maple_bus_ft1.hpp"

#include "ftdi_transfer.hpp"
#include "ftdi_maple.hpp"

constexpr uint32_t base_address = 0x0c004020;
constexpr uint32_t maple_buffer_size = 16384;

struct storage_fd {
  uint32_t fd;

  uint32_t partitions() {
    return ((fd >> 24) & 0xff) + 1;
  }
  uint32_t bytes_per_block() {
    return (((fd >> 16) & 0xff) + 1) * 32;
  }
  uint32_t write_accesses() {
    return (fd >> 12) & 0xf;
  }
  uint32_t read_accesses() {
    return (fd >> 8) & 0xf;
  }

  uint32_t bytes_per_read_access()
  {
    // divide rounding up
    return (bytes_per_block() + (read_accesses() - 1)) / read_accesses();
  }

  uint32_t bytes_per_write_access()
  {
    // divide rounding up
    return (bytes_per_block() + (write_accesses() - 1)) / write_accesses();
  }
};

constexpr int count_left_set_bits(uint32_t n, int stop_bit)
{
  int bit_ix = 31;
  int count = 0;
  while (bit_ix != stop_bit) {
    if (n & (1 << 31)) {
      count += 1;
    }
    n <<= 1;
    bit_ix -= 1;
  }

  return count;
}
static_assert(count_left_set_bits(0xe, 1) == 2);
static_assert(count_left_set_bits(0x2, 1) == 0);

consteval int count_trailing_zeros(uint32_t n)
{
  int count = 0;
  for (int i = 0; i < 32; i++) {
    if ((n & 1) != 0)
      break;
    count += 1;
    n >>= 1;
  }
  return count;
}
static_assert(count_trailing_zeros(0x80) == 7);
static_assert(count_trailing_zeros(0x2) == 1);

void print_storage_function_definition(const uint32_t fd)
{
  int partitions = ((fd >> 24) & 0xff) + 1;
  int bytes_per_block = (((fd >> 16) & 0xff) + 1) * 32;
  int write_accesses = (fd >> 12) & 0xf;
  int read_accesses = (fd >> 8) & 0xf;

  fprintf(stderr, "  storage function definition:\n");
  fprintf(stderr, "    partitions: %d\n", partitions);
  fprintf(stderr, "    bytes_per_block: %d\n", bytes_per_block);
  fprintf(stderr, "    write_accesses: %d\n", write_accesses);
  fprintf(stderr, "    read_accesses: %d\n", read_accesses);
}

template <typename T>
static inline T be_bswap(const T n)
{
  if constexpr (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return n;
  else
    return std::byteswap<T>(n);
}

void print_device_id(struct maple::device_id& device_id)
{
  fprintf(stderr, "  ft: %08x\n", be_bswap<uint32_t>(device_id.ft));
  fprintf(stderr, "  fd[0]: %08x\n", be_bswap<uint32_t>(device_id.fd[0]));
  fprintf(stderr, "  fd[1]: %08x\n", be_bswap<uint32_t>(device_id.fd[1]));
  fprintf(stderr, "  fd[2]: %08x\n", be_bswap<uint32_t>(device_id.fd[2]));
}

void print_media_info(struct ft1::get_media_info_data_transfer::data_format& data)
{
  fprintf(stderr, "  media_info:\n");
  fprintf(stderr, "    total_size: %04x\n", data.total_size);
  fprintf(stderr, "    partition_number: %04x\n", data.partition_number);
  fprintf(stderr, "    system_area_block_number: %04x\n", data.system_area_block_number);
  fprintf(stderr, "    fat_area_block_number: %04x\n", data.fat_area_block_number);
  fprintf(stderr, "    number_of_fat_area_blocks: %04x\n", data.number_of_fat_area_blocks);
  fprintf(stderr, "    file_information_block_number: %04x\n", data.file_information_block_number);
  fprintf(stderr, "    number_of_file_information_blocks: %04x\n", data.number_of_file_information_blocks);
  fprintf(stderr, "    volume_icon: %04x\n", data.volume_icon);
  fprintf(stderr, "    save_area_block_number: %04x\n", data.save_area_block_number);
  fprintf(stderr, "    number_of_save_area_blocks: %04x\n", data.number_of_save_area_blocks);
  fprintf(stderr, "    reserved_for_execution_file: %08x\n", data.reserved_for_execution_file);
}


void send_device_request_all_ports(maple::host_command_writer<base_address>& writer)
{
  using command_type = maple::device_request;
  using response_type = maple::device_status;

  writer.template append_command_all_ports<command_type, response_type>();
}


void send_extension_device_request(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  writer.template append_command<command_type, response_type>(host_port_select,
                                                              destination_ap,
                                                              false); // end_flag
}


void send_get_media_info(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::get_media_info;
  using response_type = maple::data_transfer<ft1::get_media_info_data_transfer::data_format>;

  auto [host_command, host_response] =
    writer.template append_command<command_type, response_type>(host_port_select,
                                                                destination_ap,
                                                                false); // end_flag

  host_command->bus_data.data_fields.function_type = be_bswap<uint32_t>(function_type::storage);
  host_command->bus_data.data_fields.pt = 0;
}

struct block_read_response {
  union responses {
    struct maple::file_error::data_fields file_error;
    struct maple::data_transfer<ft1::block_read_data_transfer::data_format>::data_fields data_transfer;
  };

  using data_fields = union responses;
};


void send_block_read(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm, uint32_t recv_trailing, int partition, int phase, int block_number)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::block_read;
  using response_type = block_read_response;

  auto [host_command, host_response] =
    writer.template append_command<command_type, response_type>(host_port_select,
                                                                destination_ap,
                                                                false, // end_flag
                                                                0,  // send_trailing
                                                                recv_trailing  // recv_trailing
                                                                );

  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = be_bswap<uint32_t>(function_type::storage);
  data_fields.pt = partition;
  data_fields.phase = phase;
  data_fields.block_number = be_bswap<uint16_t>(block_number);
}


void do_lm_requests(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm,
                    void (* func)(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm))
{
  uint32_t bit = ap::lm_bus::_0;
  for (int i = 0; i < 5; i++) {
    if (lm & bit) {
      func(writer, port, bit);
    }
    bit <<= 1;
  }
}

int handle_block_read_dump(struct ftdi_context * ftdi,
                           maple::host_command_writer<base_address>& writer,
                           struct storage_fd& storage_fd,
                           FILE * dump)
{
  writer.set_end_flag();
  int res = do_maple_raw(ftdi,
                         writer.send_buf, writer.send_offset,
                         writer.recv_buf, writer.recv_offset);
  if (res != 0) {
    return -1;
  }

  using response_type = block_read_response;
  using host_response_type = maple::host_response<response_type::data_fields>;
  const uint32_t recv_trailing = storage_fd.bytes_per_read_access();
  const uint32_t host_response_size = (sizeof (host_response_type)) + recv_trailing;

  uint8_t * recv_buf = reinterpret_cast<uint8_t *>(writer.recv_buf);

  for (uint32_t offset = 0; offset < writer.recv_offset; offset += host_response_size) {
    auto host_response = reinterpret_cast<host_response_type *>(&recv_buf[offset]);
    auto& bus_data = host_response->bus_data;
    if (bus_data.command_code != maple::data_transfer<uint8_t[0]>::command_code) {
      fprintf(stderr, "lm did not reply to block read: %d\n", bus_data.command_code);
      auto& file_error = bus_data.data_fields.file_error;
      if (bus_data.command_code == maple::file_error::command_code) {
	fprintf(stderr, "function error code: %d\n", file_error.function_error_code);
      }
      return -1;
    }

    fwrite(bus_data.data_fields.data_transfer.data.block_data,
           1,
           storage_fd.bytes_per_read_access(),
           dump);
  }

  writer.reset();

  return 0;
}


int do_maple_block_read_dump(struct ftdi_context * ftdi,
                             int port,
                             int lm,
                             struct storage_fd& storage_fd,
                             struct ft1::get_media_info_data_transfer::data_format& media_info) // copy of media_info on the stack
{
  uint8_t send_buf[maple_buffer_size];
  uint8_t recv_buf[maple_buffer_size];

  char filename[256];
  snprintf(filename, (sizeof (filename)), "dump_p%dl%d.bin", port, ap_lm_bus_int(lm));

  FILE * dump = fopen(filename, "wb");

  auto writer = maple::host_command_writer<base_address>(send_buf,
                                                         recv_buf);
  constexpr int partition = 0;

  using response_type = maple::data_transfer<ft1::get_media_info_data_transfer::data_format>;
  using host_response_type = maple::host_response<response_type>;
  const uint32_t recv_trailing = storage_fd.bytes_per_read_access();
  const uint32_t host_response_size = (sizeof (host_response_type)) + recv_trailing;

  for (uint16_t block_number = 0; block_number < (media_info.total_size + 1); block_number++) {
    for (uint32_t phase = 0; phase < storage_fd.read_accesses(); phase++) {
      fprintf(stderr, "maple_block_read: block %d,%d\n", block_number, phase);
      send_block_read(writer, port, lm, recv_trailing, partition, phase, block_number);
    }

    if (writer.recv_offset + host_response_size > maple_buffer_size) {
      int res = handle_block_read_dump(ftdi,
                                       writer,
                                       storage_fd,
                                       dump);
      if (res != 0) {
        return -1;
      }
    }
  }

  if (writer.send_offset != 0) {
    int res = handle_block_read_dump(ftdi,
                                     writer,
                                     storage_fd,
                                     dump);
    if (res != 0) {
      return -1;
    }
  }

  fclose(dump);

  return 0;
}


int handle_get_media_info_data_transfer_response(struct ftdi_context * ftdi,
                                                 maple::host_command_writer<base_address>& writer,
                                                 struct storage_fd storage_fd[4][5],
                                                 int (* func)(struct ftdi_context * ftdi, int port, int lm, struct storage_fd& storage_fd, struct ft1::get_media_info_data_transfer::data_format& media_info))
{
  writer.set_end_flag();
  int res = do_maple_raw(ftdi,
                         writer.send_buf, writer.send_offset,
                         writer.recv_buf, writer.recv_offset);
  if (res != 0) {
    return -1;
  }

  const uint32_t recv_offset = writer.reset();

  using response_type = maple::data_transfer<ft1::get_media_info_data_transfer::data_format>;
  using host_response_type = maple::host_response<response_type::data_fields>;

  for (uint32_t offset = 0; offset < recv_offset; offset += (sizeof (host_response_type))) {
    auto host_response = reinterpret_cast<host_response_type *>(&writer.recv_buf[offset]);

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;
    auto& data = data_fields.data;

    if (bus_data.command_code != response_type::command_code) {
      fprintf(stderr, "  disconnected %02x %02x %02x %02x\n",
              bus_data.command_code,
              bus_data.destination_ap,
              bus_data.source_ap,
              bus_data.data_size);
      continue;
    }

    uint32_t port = (bus_data.source_ap & ap::port_select::bit_mask) >> 6;
    uint32_t lm_bus = (bus_data.source_ap & ap::lm_bus::bit_mask) >> 0;
    fprintf(stderr, "[extension] port: %d ; lm: %05b\n", port, lm_bus);
    print_media_info(data);

    func(ftdi,
         port,
         lm_bus,
         storage_fd[port][ap_lm_bus_int(lm_bus)],
         data);
  }

  return 0;
}


int handle_device_status_response_extension(struct ftdi_context * ftdi,
                                            maple::host_command_writer<base_address>& writer,
                                            struct storage_fd storage_fd[4][5])
{
  writer.set_end_flag();
  int res = do_maple_raw(ftdi,
                         writer.send_buf, writer.send_offset,
                         writer.recv_buf, writer.recv_offset);
  if (res != 0) {
    return -1;
  }

  const uint32_t recv_offset = writer.reset();

  using response_type = maple::device_status;
  using host_response_type = maple::host_response<response_type::data_fields>;
  for (uint32_t offset = 0; offset < recv_offset; offset += (sizeof (host_response_type))) {
    auto host_response = reinterpret_cast<host_response_type *>(&writer.recv_buf[offset]);

    auto& bus_data = host_response->bus_data;
    auto& data_fields = bus_data.data_fields;

    if (bus_data.command_code != response_type::command_code) {
      fprintf(stderr, "  disconnected %02x %02x %02x %02x\n",
              bus_data.command_code,
              bus_data.destination_ap,
              bus_data.source_ap,
              bus_data.data_size);
      continue;
    }

    uint32_t port = (bus_data.source_ap & ap::port_select::bit_mask) >> 6;
    uint32_t lm_bus = (bus_data.source_ap & ap::lm_bus::bit_mask) >> 0;
    fprintf(stderr, "[extension] port: %d ; lm: %05b\n", port, lm_bus);
    print_device_id(data_fields.device_id);

    uint32_t ft = be_bswap<uint32_t>(data_fields.device_id.ft);
    if (ft & function_type::storage) {
      int fd_ix = count_left_set_bits(ft, count_trailing_zeros(function_type::storage));
      uint32_t fd = be_bswap<uint32_t>(data_fields.device_id.fd[fd_ix]);
      print_storage_function_definition(fd);

      storage_fd[port][ap_lm_bus_int(lm_bus)].fd = fd;
      send_get_media_info(writer, port, lm_bus);
    }
  }

  return 0;
}


int handle_device_status_response(struct ftdi_context * ftdi,
                                  maple::host_command_writer<base_address>& writer)
{
  fprintf(stderr, "handle device status response:\n");
  fprintf(stderr, "  send_offset: %d\n", writer.send_offset);
  fprintf(stderr, "  recv_offset: %d\n", writer.recv_offset);
  for (uint32_t i = 0; i < writer.send_offset / 4; i++) {
    fprintf(stderr, "  send_buf: %08x\n", ((uint32_t*)writer.send_buf)[i]);
  }
  int res = do_maple_raw(ftdi,
                         writer.send_buf, writer.send_offset,
                         writer.recv_buf, writer.recv_offset);
  if (res != 0) {
    return -1;
  }
  writer.reset();

  using response_type = maple::device_status;
  using host_response_type = maple::host_response<response_type::data_fields>;

  for (uint8_t port = 0; port < 4; port++) {
    auto host_response = reinterpret_cast<host_response_type *>(writer.recv_buf);
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    fprintf(stderr, "[device] port: %d\n", port);
    if (bus_data.command_code != response_type::command_code) {
      fprintf(stderr, "  disconnected %02x %02x %02x %02x\n",
              bus_data.command_code,
              bus_data.destination_ap,
              bus_data.source_ap,
              bus_data.data_size);
    } else {
      print_device_id(data_fields.device_id);
      uint8_t source_ap__lm_bus = bus_data.source_ap & ap::lm_bus::bit_mask;
      do_lm_requests(writer, port, source_ap__lm_bus, &send_extension_device_request);
    }
  }

  return 0;
}

int do_maple_storage_dump(struct ftdi_context * ftdi)
{
  uint8_t send_buf[maple_buffer_size];
  uint8_t recv_buf[maple_buffer_size];

  auto writer = maple::host_command_writer<base_address>(send_buf,
                                                         recv_buf);

  int res;

  send_device_request_all_ports(writer);

  res = handle_device_status_response(ftdi, writer);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  struct storage_fd storage_fd[4][5];

  res = handle_device_status_response_extension(ftdi, writer, storage_fd);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  res = handle_get_media_info_data_transfer_response(ftdi, writer, storage_fd, &do_maple_block_read_dump);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  return 0;
}

int send_block_write(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm, int partition, int phase, int block_number, const uint8_t * src, uint32_t send_trailing)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::block_write<uint8_t[0]>;
  using response_type = maple::device_reply;

  auto [host_command, host_response] =
    writer.template append_command<command_type, response_type>(host_port_select,
                                                                destination_ap,
                                                                false, // end_flag
                                                                send_trailing,  // send_trailing
                                                                0  // recv_trailing
                                                                );

  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = be_bswap<uint32_t>(function_type::storage);
  data_fields.pt = partition;
  data_fields.phase = phase;
  data_fields.block_number = be_bswap<uint32_t>(block_number);

  memcpy(data_fields.written_data, src, send_trailing);

  //printf("sizeof host_command %ld %d\n", (sizeof (*host_command)), send_trailing);

  return (sizeof (*host_command)) + send_trailing;
}

struct get_last_error_response {
  union responses {
    struct maple::device_reply device_reply;
    struct maple::file_error file_error;
  };

  using data_fields = union responses;
};

int send_get_last_error(maple::host_command_writer<base_address>& writer, uint8_t port, uint8_t lm, int partition, int phase, int block_number)
{
  uint32_t host_port_select = host_instruction_port_select(port);
  uint32_t destination_ap = ap_port_select(port) | ap::de::expansion_device | lm;

  using command_type = maple::get_last_error;
  using response_type = get_last_error_response;

  auto [host_command, host_response]
    = writer.append_command<command_type, response_type>(host_port_select,
                                                         destination_ap,
                                                         true); // end_flag

  auto& data_fields = host_command->bus_data.data_fields;
  data_fields.function_type = be_bswap<uint32_t>(function_type::storage);
  data_fields.pt = partition;
  data_fields.phase = phase;
  data_fields.block_number = be_bswap<uint16_t>(block_number);
  printf("write phase: %d\n", phase);

  return (sizeof (*host_command));
}

int handle_block_write(struct ftdi_context * ftdi,
                       maple::host_command_writer<base_address>& writer)
{
  writer.set_end_flag();
  int res = do_maple_raw(ftdi,
                         writer.send_buf, writer.send_offset,
                         writer.recv_buf, writer.recv_offset);
  if (res != 0) {
    return -1;
  }

  using response_type = maple::device_reply;
  using host_response_type = maple::host_response<response_type::data_fields>;

  auto host_response = reinterpret_cast<host_response_type *>(writer.recv_buf);
  auto& bus_data = host_response->bus_data;
  printf("handle_block_write: command_code: %d\n", host_response->bus_data.command_code);

  if (bus_data.command_code != response_type::command_code) {
    printf("lm did not reply to block write: command code: %d\n", bus_data.command_code);
    return -1;
  }

  writer.reset();

  return 0;
}

int do_maple_block_load(struct ftdi_context * ftdi,
                        int port,
                        int lm,
                        struct storage_fd& storage_fd,
                        struct ft1::get_media_info_data_transfer::data_format& media_info)
{
  printf("do maple block load\n");
  printf("  port: %d\n", port);
  printf("  lm: %d\n", lm);

  uint8_t send_buf[maple_buffer_size];
  uint8_t recv_buf[maple_buffer_size];

  FILE * img = fopen("/home/bilbo/dreamcast/img.vmu.data", "rb");
  assert(img != NULL);
  uint8_t buf[128 * 1024];
  size_t ret = fread(buf, 1, (sizeof (buf)), img);
  assert(ret == (sizeof (buf)));
  fclose(img);

  auto writer = maple::host_command_writer<base_address>(send_buf,
                                                         recv_buf);

  constexpr int partition = 0;
  const uint32_t send_trailing = storage_fd.bytes_per_write_access();

  int offset = 0;

  for (uint16_t block_number = 0; block_number < (media_info.total_size + 1); block_number++) {
    uint32_t phase;
    fprintf(stderr, "maple_block_load: block_write: block_number %d\n", block_number);

    for (phase = 0; phase < storage_fd.write_accesses(); phase++) {
      send_block_write(writer, port, lm, partition, phase, block_number,
                       &buf[offset], send_trailing);
      int res = handle_block_write(ftdi, writer);
      if (res != 0) {
        return -1;
      }
      usleep(16000); // sleep 16 ms

      offset += send_trailing;
    }

    send_get_last_error(writer, port, lm, partition, phase, block_number);
    int res = handle_block_write(ftdi, writer);
    if (res != 0) {
      return -1;
    }
    usleep(16000); // sleep 16 ms
  }

  assert(writer.send_offset == 0);

  return 0;
}

int do_maple_storage_load(struct ftdi_context * ftdi)
{
  uint8_t send_buf[maple_buffer_size];
  uint8_t recv_buf[maple_buffer_size];

  auto writer = maple::host_command_writer<base_address>(send_buf,
                                                         recv_buf);

  int res;

  send_device_request_all_ports(writer);

  res = handle_device_status_response(ftdi, writer);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  struct storage_fd storage_fd[4][5];

  res = handle_device_status_response_extension(ftdi, writer, storage_fd);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  res = handle_get_media_info_data_transfer_response(ftdi, writer, storage_fd, &do_maple_block_load);
  if (res != 0)
    return -1;
  if (writer.send_offset == 0)
    return 0;

  return 0;
}
