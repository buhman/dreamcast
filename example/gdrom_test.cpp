#include "gdrom.hpp"
#include "gdrom_bits.hpp"
#include "memorymap.hpp"
#include "systembus.hpp"

#include "sh7091/serial.hpp"

union data {
  uint8_t u8[2];
  uint16_t u16;
};
static_assert((sizeof (data)) == 2);


void test_unit()
{
  serial::string("test_unit\n");
  // wait for BSY == 0 && DRQ == 0
  while ((gdrom_if.status & (gdrom::status::bsy | gdrom::status::drq)) != 0) {
    serial::integer<uint8_t>(gdrom_if.status);
    for (int i = 0; i < 1000000; i++) { asm volatile ("nop;"); }
  };
  serial::string("bsy | drq == 0\n");

  gdrom_if.command = 0xa0; // packet command
  while ((gdrom_if.status & gdrom::status::drq) == 0);
  serial::string("drq != 0; CoD: ");
  serial::integer<uint8_t>(gdrom_if.interrupt_reason & 1);

  serial::string("bsy1: ");
  serial::integer<uint8_t>(gdrom_if.status & gdrom::status::bsy);
  for (int i = 0; i < 6; i++)
    gdrom_if.data = 0;
  serial::integer<uint8_t>(gdrom_if.status & gdrom::status::bsy);

  while ((gdrom_if.status & (gdrom::status::bsy | gdrom::status::drq)) != 0);
  serial::string("bsy2: ");
  serial::integer<uint8_t>(gdrom_if.status);
  serial::string("\n");
}

void pio_data(const uint8_t * data)
{
  while ((gdrom_if.status & (gdrom::status::bsy | gdrom::status::drq)) != 0);
  serial::string("bsy | drq == 0\n");

  gdrom_if.features = 0; // not DMA
  gdrom_if.drive_select = 0b1010'0000; // LUN 0

  gdrom_if.command = 0xa0; // packet command
  // CoD
  //serial::string("wait CoD\n");
  while ((gdrom_if.interrupt_reason & 0b11) != gdrom::interrupt_reason::cod);
  //serial::string("done CoD\n");
  while ((gdrom_if.status & gdrom::status::drq) == 0);
  serial::string("drq == 1\n");

  const uint16_t * buf = reinterpret_cast<const uint16_t *>(&data[0]);
  for (int i = 0; i < 6; i++) {
    gdrom_if.data = buf[i];
  }

  serial::string("status1: ");
  serial::integer<uint8_t>(gdrom_if.status);
  while ((gdrom_if.status & gdrom::status::bsy) != 0) {
    serial::integer<uint8_t>(gdrom_if.status);
    for (int i = 0; i < 10000000; i++) { asm volatile ("nop;"); }
  };
  serial::string("status2: ");
  serial::integer<uint8_t>(gdrom_if.status);

  serial::string("byte_control: ");
  serial::integer<uint8_t>(gdrom_if.byte_control_high, ' ');
  serial::integer<uint8_t>(gdrom_if.byte_control_low);
}

void read_data(uint32_t length)
{
  uint16_t read[length / 2];
  for (uint32_t i = 0; i < (length / 2); i++) {
    read[i] = gdrom_if.data;
  }
  const uint8_t * read_buf = reinterpret_cast<const uint8_t *>(&read[0]);
  for (uint32_t i = 0; i < length; i++) {
    serial::hexlify(read_buf[i]);
    serial::character(' ');
    if ((i & 0xf) == 0xf)
      serial::character('\n');
  }
  serial::character('\n');
  serial::string("status: ");
  serial::integer<uint8_t>(gdrom_if.status);
}

void req_stat()
{
  const uint8_t data[12] = {
    0x10, // req_stat
    0x00,
    0x00, // starting_address
    0x00,
    0x0a, // allocation_length
    0x00,

    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  serial::string("\nreq_stat\n");
  pio_data(data);
  read_data(0xa);
}

void req_mode()
{
  const uint8_t data[12] = {
    0x11, // req_mode
    0x00,
    0x00, // starting_address
    0x00,
    0x20, // allocation_length
    0x00,

    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  serial::string("\nreq_mode\n");
  pio_data(data);
  read_data(0x20);
}

void get_toc()
{
  const uint8_t data[12] = {
    0x14, // GET_TOC
    (0 << 0), // single density
    0x0,
    0xff, // allocation length msb
    0xff, // allocation length lsb
    0x00, // 5
    0x00, // 6
    0x00, // 7
    0x00, // 8
    0x00, // 9
    0x00, // 10
    0x00, // 11
  };

  serial::string("\nget_toc\n");
  pio_data(data);
  const uint32_t length = (gdrom_if.byte_control_high << 8) | (gdrom_if.byte_control_low << 0);
  // 102 entries ; 4 bytes per entry, 408 bytes (0x0198)
  read_data(length);
}

/* TOC:
   01 00 00 96 (track 1 information)
   - audio track
   - FAD track start: 0x000096

   41 00 2e 4c (track 2 information)
   - data track
   - FAD track start: 0x002e4c

   01 01 00 00 (start track information)
   01: first track is audio track
   01: first track is track number 1

   41 02 00 00 (end track information)
   41: last track is data track
   02: last track is track number 2

   41 00 2f 7c (lead-out information)
*/

void cd_read()
{
  // CD-ROM XA mode 2 form 1

  const uint8_t data_select = 0b0010; // data
  const uint8_t expected_data_type = 0b100; // XA mode 2 form 1
  const uint8_t parameter_type = 0b0; // FAD specified


  const uint8_t data[12] = {
    0x31, // CD_READ
    (data_select << 4) | (expected_data_type << 1) | (parameter_type << 0),

    0x00, // starting address (msb)
    0x2e, //
    0x4c, // starting address (lsb)

    0x00, // 5

    0x00, // transfer length (msb)
    0x11, // transfer length (lsb)

    0x00, // next address (msb)
    0x2e, // 9
    0x4c, // next address (lsb)
    0x00, // 11
  };

  serial::string("\ncd_read\n");
  pio_data(data);

  uint32_t read = 0;

  while ((gdrom_if.status & 0x08) != 0) {
    serial::string("offset: ");
    serial::integer<uint32_t>(read);

    const uint32_t length = (gdrom_if.byte_control_high << 8) | (gdrom_if.byte_control_low << 0);
    // 102 entries ; 4 bytes per entry, 408 bytes (0x0198)
    read_data(length);
    read += length;

    serial::string("byte_control: ");
    serial::integer<uint8_t>(gdrom_if.byte_control_high, ' ');
    serial::integer<uint8_t>(gdrom_if.byte_control_low);
  }
  serial::string("read bytes: ");
  serial::integer<uint32_t>(read);
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

  test_unit();
  req_stat();
  req_mode();
  get_toc();

  cd_read();

  while (1);
}
