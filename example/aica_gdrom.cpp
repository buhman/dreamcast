#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "aica/aica.hpp"

#include "gdrom/gdrom.hpp"
#include "gdrom/gdrom_bits.hpp"
#include "gdrom/command_packet_format.hpp"
#include "gdrom/toc.hpp"

#include "iso9660/primary_volume_descriptor.hpp"
#include "iso9660/directory_record.hpp"

extern void * _binary_start __asm("_binary_example_arm_sh4_interrupt_bin_start");
extern void * _binary_size __asm("_binary_example_arm_sh4_interrupt_bin_size");

constexpr uint32_t mcipd__sh4_interrupt = (1 << 5);
constexpr uint32_t scipd__arm_interrupt = (1 << 5);

constexpr uint32_t sectors_per_chunk = 16;
constexpr uint32_t chunk_size = 2048 * sectors_per_chunk;

void aica_wait_write()
{
  while (ffst::aica_internal_write_buffer(system.FFST));
}

void aica_wait_read()
{
  uint32_t ffst = system.FFST;
  while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
	| ffst::holly_g2_if_block_internal_write_buffer(ffst)
	| ffst::aica_internal_write_buffer(ffst)) {
    ffst = system.FFST;
  };
}

void aica_fill_chunk(volatile uint32_t * dest_chunk, const uint32_t * src_chunk, const uint32_t size)
{
  for (uint32_t i = 0; i < size / 4; i++) {
    if (i % 8 == 0) aica_wait_write();
    dest_chunk[i] = src_chunk[i];
  }
}

static volatile uint32_t (* chunk)[2][chunk_size / 4];

void aica_init(uint32_t& chunk_index, const uint32_t * src_chunk)
{
  const uint32_t * binary = reinterpret_cast<uint32_t *>(&_binary_start);
  const uint32_t binary_size = reinterpret_cast<uint32_t>(&_binary_size);

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  aica_wait_write(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0);
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    while (aica_wave_memory[i] != binary[i]) {
      aica_wait_write();
      aica_wave_memory[i] = binary[i];
    }
  }

  chunk = reinterpret_cast<decltype (chunk)>(&aica_wave_memory[0x001f0000 / 4]);

  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[0][0]));
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[1][0]));

  aica_fill_chunk(&(*chunk)[chunk_index][0],
                  src_chunk,
                  chunk_size);
  chunk_index = (chunk_index + 1) % 2;

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(0);
}

void aica_step(uint32_t& chunk_index, const uint32_t * src_chunk)
{
  aica_wait_read();
  { // wait for interrupt from arm
    while ((aica_sound.common.MCIPD() & mcipd__sh4_interrupt) == 0) { aica_wait_read(); };
    aica_wait_write(); aica_sound.common.mcire = mcipd__sh4_interrupt;
  }

  { // fill the requested chunk
    aica_fill_chunk(&(*chunk)[chunk_index][0],
                    src_chunk,
                    chunk_size);

    chunk_index = (chunk_index + 1) % 2;
  }
}

// gdrom

void gdrom_pio_data(const uint8_t * data)
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

void gdrom_read_data(uint16_t * buf, const uint32_t length)
{
  //serial::string("read_data drq interrupt_reason: ");
  //serial::integer<uint8_t>(gdrom::status::drq(gdrom_if.status), ' ');
  //serial::integer<uint8_t>(gdrom_if.interrupt_reason);
  for (uint32_t i = 0; i < (length / 2); i++) {
    buf[i] = gdrom_if.data;
  }
}

uint32_t gdrom_toc__get_data_track_fad()
{
  auto packet = gdrom_command_packet_format::get_toc(0,     // single-density
                                                     0x0198 // maximum toc length
                                                     );
  serial::string("get_toc\n");
  gdrom_pio_data(packet._data());

  serial::string("byte_count: ");
  serial::integer<uint16_t>(gdrom_if.byte_count());
  uint16_t buf[gdrom_if.byte_count() / 2];
  gdrom_read_data(buf, gdrom_if.byte_count());

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

uint32_t gdrom_cd_read2(uint16_t * buf,
                        const uint32_t starting_address,
                        const uint32_t transfer_length,
                        const uint32_t next_address)
{
  const uint8_t data_select = 0b0010; // data
  const uint8_t expected_data_type = 0b100; // XA mode 2 form 1
  const uint8_t parameter_type = 0b0; // FAD specified
  const uint8_t data = (data_select << 4) | (expected_data_type << 1) | (parameter_type << 0);

  auto packet = gdrom_command_packet_format::cd_read2(data,
                                                      starting_address,
                                                      transfer_length,
                                                      next_address);
  //serial::string("cd_read\n");
  //serial::string("starting_address: ");
  //serial::integer<uint32_t>(starting_address);
  //serial::string("transfer_length: ");
  //serial::integer<uint32_t>(transfer_length);
  //serial::string("next_address: ");
  //serial::integer<uint32_t>(next_address);
  gdrom_pio_data(packet._data());

  uint32_t length = 0;
  while ((gdrom::status::drq(gdrom_if.status)) != 0) {
    const uint32_t byte_count = gdrom_if.byte_count();
    length += byte_count;
    gdrom_read_data(buf, byte_count);

    serial::string("read status: ");
    serial::integer<uint8_t>(gdrom_if.status);

    while ((gdrom::status::bsy(gdrom_if.status)) != 0); // wait for drive to become not-busy
  }

  serial::string("read length: ");
  serial::integer<uint32_t>(length);
  return length;
}

void gdrom_unlock()
{
  // gdrom unlock undocumented register
  g1_if.GDUNLOCK = 0x1fffff;

  // Without this read from system_boot_rom, the read value of
  // gdrom_if.status is always 0xff
  for(uint32_t i = 0; i < 0x200000 / 4; i++) {
    (void)system_boot_rom[i];
  }
}

bool str_equal(const uint8_t * a,
               const uint32_t a_len,
               const char * b,
               const uint32_t b_len)
{
  if (a_len != b_len)
    return false;

  uint32_t len = a_len;

  while (len != 0) {
    if (*a++ != *b++)
      return false;

    len--;
  }

  return true;
}

struct extent
{
  const uint32_t location;
  const uint32_t data_length;
};

struct extent gdrom_find_file()
{
  const uint32_t fad = gdrom_toc__get_data_track_fad();
  serial::character('\n');

  const uint32_t primary_volume_descriptor = fad + 16;
  uint16_t buf[2048 / 2];
  const uint32_t length0 = gdrom_cd_read2(buf,
                                          primary_volume_descriptor,     // starting address
                                          1,                             // one sector; 2048 bytes
                                          primary_volume_descriptor + 1  // next address
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

  const uint32_t root_directory_extent = root_dr->location_of_extent.get();
  const uint32_t length1 = gdrom_cd_read2(buf,
                                          root_directory_extent + 150, // 150?
                                          1, // one sector; 2048 bytes
                                          root_directory_extent + 151  // 150?
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

    bool equal = str_equal(dr->file_identifier, dr->length_of_file_identifier,
                           "REIGN.PCM;1", 11);

    if (dr->file_flags == 0) {
      serial::string("  location_of_extent: ");
      serial::integer<uint32_t>(dr->location_of_extent.get());
      serial::string("  data_length: ");
      serial::integer<uint32_t>(dr->data_length.get());

      if (equal) {
        serial::string("FOUND\n");
        return {
          dr->location_of_extent.get(),
          dr->data_length.get()
        };
      }
    }

    offset += dr->length_of_directory_record;
  }

  return { 0 , 0 };
}

void gdrom_read_chunk(uint32_t * buf, const uint32_t extent, const uint32_t num_extents)
{
  const uint32_t length1 = gdrom_cd_read2(reinterpret_cast<uint16_t *>(buf),
                                          extent + 150, // 150?
                                          num_extents,  // one sector; 2048 bytes
                                          extent + 150 + num_extents  // 150?
                                          );
}

void next_segment(const struct extent& extent, uint32_t& segment_index)
{
  segment_index += sectors_per_chunk;
  if ((segment_index * 2048) > extent.data_length)
    segment_index = 0;
}

void main()
{
  uint32_t chunk_index = 0;
  uint32_t segment_index = 0;

  gdrom_unlock();
  const auto extent = gdrom_find_file();
  uint32_t gdrom_buf[chunk_size / 4];
  gdrom_read_chunk(gdrom_buf, extent.location + segment_index, sectors_per_chunk);
  next_segment(extent, segment_index);

  aica_init(chunk_index, &gdrom_buf[0]);

  serial::string("aica wave memory:\n");
  while (aica_wave_memory[0] == 0xeaffffff) { aica_wait_read(); };
  aica_wait_read(); serial::integer<uint32_t>(aica_wave_memory[0]);
  aica_wait_read(); serial::integer<uint32_t>(aica_wave_memory[1]);

  while (1) {
    //serial::integer<uint8_t>(chunk_index);
    gdrom_read_chunk(gdrom_buf, extent.location + segment_index, sectors_per_chunk);
    next_segment(extent, segment_index);

    aica_step(chunk_index, &gdrom_buf[0]);
  }

  while (1);
}
