#include <cstdint>

#include "memorymap.hpp"
#include "systembus.hpp"
#include "sh7091/serial.hpp"
#include "gdrom/gdrom.hpp"
#include "gdrom/gdrom_bits.hpp"
#include "gdrom/command_packet_format.hpp"
#include "gdrom/toc.hpp"

#include "iso9660/primary_volume_descriptor.hpp"
#include "iso9660/directory_record.hpp"

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

void read_data(uint16_t * buf, const uint32_t length)
{
  serial::string("read_data drq interrupt_reason: ");
  serial::integer<uint8_t>(gdrom::status::drq(gdrom_if.status), ' ');
  serial::integer<uint8_t>(gdrom_if.interrupt_reason);
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

void main()
{
  // gdrom unlock undocumented register
  g1_if.GDUNLOCK = 0x1fffff;

  // Without this read from system_boot_rom, the read value of
  // gdrom_if.status is always 0xff
  for(uint32_t i = 0; i < 0x200000 / 4; i++) {
    (void)system_boot_rom[i];
  }

  const uint32_t fad = toc__get_data_track_fad();
  serial::character('\n');

  const uint32_t primary_volume_descriptor = fad + 16;
  uint16_t buf[2048 / 2];
  const uint32_t length0 = cd_read(buf,
                                   primary_volume_descriptor,
                                   1 // one sector; 2048 bytes
                                   );
  serial::character('\n');

  auto pvd = reinterpret_cast<const iso9660::primary_volume_descriptor *>(&buf[0]);
  auto root_dr = reinterpret_cast<const iso9660::directory_record *>(&pvd->directory_record_for_root_directory[0]);

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

  /*
  for (int i = 0; i < 2048; i++) {
    serial::hexlify(buf8[i]);
    serial::character(' ');
    if ((i % 16) == 15)
      serial::character('\n');
  }
  */
  serial::character('\n');

  const uint32_t root_directory_extent = root_dr->location_of_extent.get();
  const uint32_t length1 = cd_read(buf,
                                   root_directory_extent + 150, // 150?
                                   1 // one sector; 2048 bytes
                                   );
  serial::character('\n');

  auto buf8 = reinterpret_cast<const uint8_t *>(buf);
  uint32_t offset = 0;
  while (true) {
    serial::string("directory entry offset: ");
    serial::integer<uint32_t>(offset);

    auto dr = reinterpret_cast<const iso9660::directory_record *>(&buf8[offset]);
    if (dr->length_of_directory_record == 0)
      break;

    serial::string("  length_of_directory_record: ");
    serial::integer<uint8_t>(dr->length_of_directory_record);
    serial::string("  length_of_file_identifier: ");
    serial::integer<uint8_t>(dr->length_of_file_identifier);
    serial::string("  file_identifier: ");
    serial::string(dr->file_identifier, dr->length_of_file_identifier);
    serial::character('\n');

    if (dr->file_flags == 0) {
      serial::string("  location_of_extent: ");
      serial::integer<uint32_t>(dr->location_of_extent.get());
      serial::string("  data_length: ");
      serial::integer<uint32_t>(dr->data_length.get());

      if (dr->file_identifier[0] != '1') {
	const uint32_t extent = dr->location_of_extent.get();

        uint16_t buf2[2048 / 2];
        const uint32_t length1 = cd_read(buf2,
                                         extent + 150, // 150?
                                         1 // one sector; 2048 bytes
                                         );

	auto file = reinterpret_cast<const uint8_t *>(&buf2[0]);
	serial::string("---begin file content---\n");
        serial::string(file, dr->data_length.get());
	serial::string("---end file content---\n");
      }
    }

    offset += dr->length_of_directory_record;
  }

  serial::integer<uint32_t>(offset);

  while (1);
}

/*
  for (int i = 0; i < 12; i++) {
    serial::hexlify(arst[i]);
    serial::character(' ');
  }
  serial::character('\n');
*/
