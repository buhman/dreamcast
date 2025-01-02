#include <cstdint>

#include "memorymap.hpp"
#include "systembus.hpp"
#include "sh7091/serial.hpp"
#include "gdrom/gdrom.hpp"
#include "gdrom/gdrom_bits.hpp"
#include "gdrom/command_packet_format.hpp"
#include "gdrom/toc.hpp"
#include "holly/video_output.hpp"

#include "iso9660/primary_volume_descriptor.hpp"
#include "iso9660/directory_record.hpp"

#include "crc32.h"

typedef void (*main_ptr_t)(void);

void pio_data(const uint8_t * data)
{
  while ((gdrom::status::bsy(gdrom_if.status) | gdrom::status::drq(gdrom_if.status)) != 0);

  gdrom_if.features = gdrom::features::dma::disable;
  gdrom_if.drive_select = gdrom::drive_select::drive_select
                        | gdrom::drive_select::lun(0);

  gdrom_if.command = gdrom::command::code::packet_command;
  while (gdrom::status::drq(gdrom_if.status) == 0);

  const uint16_t * buf = reinterpret_cast<const uint16_t *>(&data[0]);
  for (int i = 0; i < 6; i++) {
    gdrom_if.data = buf[i];
  }

  while (gdrom::status::bsy(gdrom_if.status) != 0);
}

void dma_data(const uint8_t * data)
{
  while ((gdrom::status::bsy(gdrom_if.status) | gdrom::status::drq(gdrom_if.status)) != 0);

  gdrom_if.features = gdrom::features::dma::enable;
  gdrom_if.drive_select = gdrom::drive_select::drive_select
                        | gdrom::drive_select::lun(0);

  gdrom_if.command = gdrom::command::code::packet_command;
  while (gdrom::status::drq(gdrom_if.status) == 0);

  const uint16_t * buf = reinterpret_cast<const uint16_t *>(&data[0]);
  for (int i = 0; i < 6; i++) {
    gdrom_if.data = buf[i];
  }

  while (gdrom::status::bsy(gdrom_if.status) != 0);
}

void read_data(uint16_t * buf, const uint32_t length)
{
  //serial::string("read_data drq interrupt_reason: ");
  //serial::integer<uint8_t>(gdrom::status::drq(gdrom_if.status), ' ');
  //serial::integer<uint8_t>(gdrom_if.interrupt_reason);
  for (uint32_t i = 0; i < (length / 2); i++) {
    buf[i] = gdrom_if.data;
  }
}

uint32_t toc__get_data_track_fad()
{
  auto packet = gdrom_command_packet_format::get_toc(0,     // single-density
                                                     0x0198 // maximum toc length
                                                     );
  serial::string("get_toc\n");
  pio_data(packet._data());

  serial::string("byte_count: ");
  serial::integer<uint16_t>(gdrom_if.byte_count());
  uint16_t buf[gdrom_if.byte_count() / 2];
  read_data(buf, gdrom_if.byte_count());

  serial::string("status: ");
  serial::integer<uint8_t>(gdrom_if.status);

  auto toc = reinterpret_cast<const struct gdrom_toc::toc *>(buf);
  for (int i = 0; i < 99; i++) {
    if (toc->track[i].fad() == 0xffffff)
      break;
    serial::string("track ");
    serial::integer<uint8_t>(i);
    serial::integer<uint32_t>(toc->track[i].fad());
  }

  // assume track 1 is the correct track
  return toc->track[1].fad();
}

uint32_t cd_read(uint16_t * buf,
                 const uint32_t starting_address,
                 const uint32_t transfer_length)
{
  const uint8_t data_select = 0b0010; // data
  const uint8_t expected_data_type = 0b100; // XA mode 2 form 1
  const uint8_t parameter_type = 0b0; // FAD specified
  const uint8_t data = (data_select << 4) | (expected_data_type << 1) | (parameter_type << 0);

  auto packet = gdrom_command_packet_format::cd_read(data,
                                                     starting_address,
                                                     transfer_length);
  serial::string("cd_read\n");
  serial::string("starting_address: ");
  serial::integer<uint32_t>(starting_address);
  pio_data(packet._data());

  uint32_t length = 0;
  while ((gdrom::status::drq(gdrom_if.status)) != 0) {
    const uint32_t byte_count = gdrom_if.byte_count();
    length += byte_count;
    read_data(buf, byte_count);
  }
  serial::string("status: ");
  serial::integer<uint8_t>(gdrom_if.status);

  serial::string("read length: ");
  serial::integer<uint32_t>(length);
  return length;
}

void cd_read_dma(const uint32_t starting_address,
                 const uint32_t transfer_length)
{
  const uint8_t data_select = 0b0010; // data
  const uint8_t expected_data_type = 0b100; // XA mode 2 form 1
  const uint8_t parameter_type = 0b0; // FAD specified
  const uint8_t data = (data_select << 4) | (expected_data_type << 1) | (parameter_type << 0);

  auto packet = gdrom_command_packet_format::cd_read(data,
                                                     starting_address,
                                                     transfer_length);
  serial::string("cd_read\n");
  serial::string("starting_address: ");
  serial::integer<uint32_t>(starting_address);
  dma_data(packet._data());
}

bool dr_is_self_or_parent(const iso9660::directory_record * dr)
{
  if (dr->length_of_file_identifier != 1)
    return false;
  uint8_t c = dr->file_identifier[0];
  return c == 0 || c == 1;
}

#define FILE_FLAGS__DIRECTORY 2

bool is_jvm_bin(const uint8_t * file_identifier, int length_of_file_identifier)
{
  static const uint8_t * jvm_bin = (const uint8_t *)"0JVM.BIN;1";
  int jvm_bin_length = 10;
  if (length_of_file_identifier != jvm_bin_length)
    return false;

  for (int i = 0; i < jvm_bin_length; i++) {
    if (file_identifier[i] != jvm_bin[i])
      return false;
  }
  return true;
}

static inline int min(int a, int b)
{
  return a < b ? a : b;
}

static bool jvm_load_complete;
static int __data_length;
constexpr int load_address = 0xac010000;

void load_jvm_bin(const iso9660::directory_record * dr)
{
  serial::string("load jvm bin:\n");
  //__attribute__((aligned(4))) static uint16_t file_buf16[16384 / 2];
  //uint32_t * file_buf32 = reinterpret_cast<uint32_t *>(file_buf16);

  int extent = dr->location_of_extent.get();
  int data_length = dr->data_length.get();

  int sectors = ((data_length + 2047) / 2048);
  data_length = sectors * 2048;

  g1_if.GDAPRO = 0x8843407F;
  g1_if.G1GDRC = 0x00001001;
  g1_if.GDSTAR = load_address & ~(0b111 << 29);
  g1_if.GDLEN = data_length;
  g1_if.GDDIR = 1;
  g1_if.GDEN = 1;
  g1_if.GDST = 1;

  cd_read_dma(extent + 150,
              sectors);

  serial::string("wait gdst");
  while ((g1_if.GDST & 1) != 0);

  /*
  int transfers = 0;
  while (data_length > 0) {
    int transfer_size = min(data_length, 2048);
    int sectors = transfer_size >> 11; // divide by 2048
    if (sectors == 0)
      sectors = 1;

    cd_read(&load_buf[offset],
            extent + 150, // 150?
            sectors // one sector
            );

    offset += transfer_size / (sizeof (load_buf[0]));

    extent += sectors;
    data_length -= sectors * 2048;
    transfers += 1;
  }
  */

  serial::string("\njvm load complete\n");
  jvm_load_complete = true;
  serial::integer<uint32_t>(extent);

  serial::string("size: ");
  serial::integer<uint32_t>(dr->data_length.get());
  __data_length = dr->data_length.get();
}

bool walk_directory_record(const iso9660::directory_record * dr)
{
  if (dr_is_self_or_parent(dr))
    return false;

  if ((dr->file_flags & FILE_FLAGS__DIRECTORY) == 0) {
    serial::string(" [regular file] ");
  } else {
    serial::string(" [directory] ");
  }

  serial::string("  file_identifier: ");
  serial::string(dr->file_identifier, dr->length_of_file_identifier);
  serial::character('\n');

  if ((dr->file_flags & FILE_FLAGS__DIRECTORY) == 0) {
    if (is_jvm_bin(dr->file_identifier, dr->length_of_file_identifier)) {
      load_jvm_bin(dr);
      return true;
    }
  }
  return false;
}

void walk_directory(uint16_t * buf, int extent, int num_extents)
{
  auto buf8 = reinterpret_cast<const uint8_t *>(buf);

  while (num_extents > 0) {
    cd_read(buf,
            extent + 150, // 150?
            1 // one sector
            );

    int offset = 0;

    while (true) {
      auto dr = reinterpret_cast<const iso9660::directory_record *>(&buf8[offset]);
      if (dr->length_of_directory_record == 0)
        break;

      bool jvm_loaded = walk_directory_record(dr);
      if (jvm_loaded)
        break;

      offset += dr->length_of_directory_record;
    }
    num_extents -= 1;
    extent += 1;
  }
}

void main()
{
  serial::init(0);

  // gdrom unlock undocumented register
  g1_if.GDUNLOCK = 0x1fffff;

  // Without this read from system_boot_rom, the read value of
  // gdrom_if.status is always 0xff
  for (uint32_t i = 0; i < 0x200000 / 4; i++) {
    (void)system_boot_rom[i];
  }

  const uint32_t fad = toc__get_data_track_fad();
  serial::character('\n');

  const uint32_t primary_volume_descriptor = fad + 16;
  uint16_t buf[2048 / 2];
  cd_read(buf,
	  primary_volume_descriptor,
	  1 // one sector
	  );
  serial::character('\n');

  auto pvd = reinterpret_cast<const iso9660::primary_volume_descriptor *>(&buf[0]);
  auto root_dr = reinterpret_cast<const iso9660::directory_record *>(&pvd->directory_record_for_root_directory[0]);

  for (int i = 0; i < 16; i++) {
    serial::integer<uint8_t, string::dec_type>(((uint8_t *)buf)[i], ' ');
  }

  serial::string("primary volume descriptor:\n");
  serial::string("  standard_identifier: ");
  serial::string(pvd->standard_identifier, 5);
  serial::character('\n');
  serial::string("  root directory record:\n");
  serial::string("    location of extent: ");
  serial::integer<uint32_t>(root_dr->location_of_extent.get());
  serial::string("    data length: ");
  serial::integer<uint32_t>(root_dr->data_length.get());

  serial::character('\n');

  const int extent = root_dr->location_of_extent.get();
  const int data_length = root_dr->data_length.get();
  const int num_extents = data_length >> 11; // division by 2048

  jvm_load_complete = false;
  walk_directory(buf, extent, num_extents);
  serial::integer<uint32_t>(jvm_load_complete);
  if (jvm_load_complete) {
    main_ptr_t jvm_main = reinterpret_cast<main_ptr_t>(load_address);

    /*
    serial::string("crc32: ");
    int chunks = __data_length / 2048;
    for (int i = 0; i < chunks; i++) {
      uint32_t crc = crc32(&((uint8_t *)load_address)[i * 2048], 2048);
      serial::integer<uint32_t>(crc);
    }
    */

    serial::string("jvm jump\n");
    jvm_main();
    serial::string("jvm return\n");
  }

  while (1);
}
