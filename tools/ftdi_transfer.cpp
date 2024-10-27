#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include <ftdi.h>
#include <libusb.h>

#include "crc32.h"
#include "serial_protocol.hpp"

extern int convert_baudrate_UT_export(int baudrate, struct ftdi_context *ftdi,
				      unsigned short *value, unsigned short *index);

int dreamcast_rates[] = {
  1562500, // 0
  781250,  // 1
  520833,  // 2
  390625,  // 3
  312500,  // 4
  260416,  // 5
  223214,  // 6
  195312,  // 7
  173611,  // 8
  156250,  // 9
  142045,  // 10
  130208,  // 11
  120192   // 12
};

int init_ftdi_context(struct ftdi_context * ftdi)
{
  ftdi_set_interface(ftdi, INTERFACE_ANY);
  struct ftdi_device_list * devlist;
  int res;
  if ((res = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0) {
    fprintf(stderr, "ftdi_usb_find_all\n");
    return -1;
  }

  if (res == 0) {
    fprintf(stderr, "no device\n");
    return -1;
  }

  struct libusb_device_descriptor desc;
  struct ftdi_device_list * devlist_item = devlist;
  for (int i = 0; i < res; i++) {
    res = libusb_get_device_descriptor(devlist_item->dev, &desc);
    if (res < 0) {
      fprintf(stderr, "libusb_get_device_descriptor\n");
      return -1;
    }
    fprintf(stdout, "idVendor: %04x; idProduct: %04x;\n", desc.idVendor, desc.idProduct);
    fprintf(stdout, "bNumConfigurations: %d;\n", desc.bNumConfigurations);
    devlist_item = devlist_item->next;
  }

  res = ftdi_usb_open_dev(ftdi, devlist->dev);
  if (res < 0) {
    fprintf(stderr, "ftdi_usb_open_dev\n");
    return -1;
  }
  ftdi_list_free(&devlist);

  /*
  unsigned short value;
  unsigned short index;
  for (unsigned int i = 0; i < (sizeof (dreamcast_rates)) / (sizeof (dreamcast_rates[0])); i++) {
    int baud = convert_baudrate_UT_export(dreamcast_rates[i], ftdi, &value, &index);
    float baudf = baud;
    float ratef = dreamcast_rates[i];
    float error = (baudf > ratef) ? ratef / baudf : baudf / ratef;
    fprintf(stdout, "%d: best: %d, error: %f\n", dreamcast_rates[i], baud, (1.f - error) * 100.f);
  }
  */

  res = ftdi_set_baudrate(ftdi, dreamcast_rates[0]);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_baudrate\n");
    return -1;
  }

  res = ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_1, NONE, BREAK_ON);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_line_property\n");
    return -1;
  }

  res = ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_1, NONE, BREAK_OFF);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_line_property\n");
    return -1;
  }

  res = ftdi_set_latency_timer(ftdi, 1);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_latency_timer\n");
    return -1;
  }

  res = ftdi_tciflush(ftdi);
  if (res < 0) {
    fprintf(stderr, "ftdi_tciflush\n");
    return -1;
  }

  res = ftdi_tcoflush(ftdi);
  if (res < 0) {
    fprintf(stderr, "ftdi_tcoflush\n");
    return -1;
  }


  return 0;
}

union data_command {
  struct {
    uint8_t command[4];
    uint32_t size;
    uint32_t dest;
  };
  uint8_t data[4 * 3];
};
static_assert((sizeof (union data_command)) == 12);

long read_with_timeout(struct ftdi_context * ftdi, uint8_t * read_buf, const long expect_length)
{
  int res;
  struct timespec tp0;
  res = clock_gettime(CLOCK_MONOTONIC, &tp0);
  assert(res >= 0);

  long read_length = 0;
  while (true) {
    res = ftdi_read_data(ftdi, read_buf, expect_length - read_length);
    assert(res >= 0);

    read_length += res;
    if (read_length >= expect_length)
      break;

    struct timespec tp1;
    res = clock_gettime(CLOCK_MONOTONIC, &tp1);
    assert(res >= 0);

    if (tp1.tv_sec - tp0.tv_sec > 1) {
      fprintf(stderr, "read timeout: %ld expect: %ld\n", read_length, expect_length);
      break;
    }
  }
  return read_length;
}

const int chunk_size = 1024;

long min(long a, long b)
{
  return a > b ? b : a;
}

long max(long a, long b)
{
  return a > b ? a : b;
}

double timespec_difference(struct timespec const * const a, struct timespec const * const b)
{
  return (double)(a->tv_sec - b->tv_sec) + (double)(a->tv_nsec - b->tv_nsec) / 1'000'000'000.0;
}

void dump_command_reply(union serial_load::command_reply& cr)
{
  for (uint32_t i = 0; i < (sizeof (union serial_load::command_reply)) / (sizeof (uint32_t)); i++) {
    fprintf(stderr, "  %08x\n", serial_load::le_bswap(cr.u32[i]));
  }
}

int read_reply(struct ftdi_context * ftdi, uint32_t expected_cmd, union serial_load::command_reply& reply)
{
  using namespace serial_load;

  constexpr long read_length = (sizeof (union serial_load::command_reply));

  long length = read_with_timeout(ftdi, reply.u8, read_length);
  if (length != read_length) {
    fprintf(stderr, "short read; want %ld bytes; received: %ld\n", read_length, length);
    return -1;
  }

  uint32_t crc = crc32(&reply.u8[0], 12);
  if (crc != reply.crc) {
    fprintf(stderr, "reply crc mismatch; remote crc: %08x; local crc: %08x\n", reply.crc, crc);
    dump_command_reply(reply);

    /*
    uint8_t buf[16] = {0};
    long length = read_with_timeout(ftdi, reply.u8, 16);
    if (length > 0) {
      fprintf(stderr, "trailing data:\n");
      for (int i = 0; i < length; i++) {
	fprintf(stderr, "%02x ", buf[i]);
      }
      fprintf(stderr, "\n");
    }
    */
    return -2;
  }

  if (reply.cmd != expected_cmd) {
    fprintf(stderr, "invalid reply; remote cmd %08x; expected cmd: %08x\n", reply.cmd, expected_cmd);
    dump_command_reply(reply);
    return -1;
  }
  dump_command_reply(reply);

  return 0;
}

int do_write(struct ftdi_context * ftdi, const uint8_t * buf, const uint32_t size)
{
  fprintf(stderr, "do_write\n");
  int res;

  const uint32_t dest = 0xac010000;
  union serial_load::command_reply command = serial_load::write_command(dest, size);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));
  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::write, reply);
  if (res != 0) {
    return -2;
  }
  fprintf(stderr, "remote: dest: %08x size: %08x\n", reply.arg[0], reply.arg[1]);
  if (reply.arg[0] != dest || reply.arg[1] != size) {
    return -1;
  }

  struct timespec start;
  struct timespec end;
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  assert(res == 0);
  res = ftdi_write_data(ftdi, buf, size);
  assert(res >= 0);
  assert((uint32_t)res == size);
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  assert(res == 0);
  fprintf(stderr, "symmetric time: %.03f\n", timespec_difference(&end, &start));

  uint32_t buf_crc = crc32(buf, size);

  union serial_load::command_reply crc_reply;
  res = read_reply(ftdi, serial_load::reply::write_crc, crc_reply);
  if (res != 0) {
    return -1;
  }
  fprintf(stderr, "remote crc: %08x; local crc %08x\n", crc_reply.arg[0], buf_crc);

  return 0;
}

int do_jump(struct ftdi_context * ftdi)
{
  fprintf(stderr, "do_jump\n");
  int res;

  const uint32_t dest = 0xac010000;

  union serial_load::command_reply command = serial_load::jump_command(dest);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));

  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::jump, reply);
  if (res != 0) {
    return -2;
  }
  fprintf(stderr, "remote: jump: %08x\n", reply.arg[0]);
  if (reply.arg[0] != dest || reply.arg[1] != 0) {
    return -1;
  }

  return 0;
}

int read_file(const char * filename, uint8_t ** buf, uint32_t * size)
{
  FILE * file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "fopen\n");
    return -1;
  }

  int ret;
  ret = fseek(file, 0L, SEEK_END);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_END)");
    return -1;
  }

  long off = ftell(file);

  ret = fseek(file, 0L, SEEK_SET);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_SET)");
    return -1;
  }

  fprintf(stderr, "%s size %ld\n", filename, off);
  *buf = (uint8_t *)malloc(off);
  ssize_t fread_size = fread(*buf, 1, off, file);
  if (fread_size < 0) {
    fprintf(stderr, "fread");
    return -1;
  }

  ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "fclose");
    return -1;
  }

  *size = off;

  return 0;
}

int do_read(struct ftdi_context * ftdi)
{
  fprintf(stderr, "do_read\n");

  int res;

  const uint32_t src = 0xac010000;
  const uint32_t size = 51584;
  union serial_load::command_reply command = serial_load::read_command(src, size);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));
  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::read, reply);
  if (res != 0) {
    return -2;
  }
  fprintf(stderr, "remote: src: %08x size: %08x\n", reply.arg[0], reply.arg[1]);
  if (reply.arg[0] != src || reply.arg[1] != size) {
    return -1;
  }

  uint32_t * buf = (uint32_t *)malloc(size);
  res = ftdi_read_data(ftdi, (uint8_t *)buf, size);
  assert(res >= 0);
  assert((uint32_t)res == size);

  uint32_t buf_crc = crc32((uint8_t*)buf, size);

  union serial_load::command_reply crc_reply;
  res = read_reply(ftdi, serial_load::reply::read_crc, crc_reply);
  if (res != 0) {
    return -1;
  }
  fprintf(stderr, "remote crc: %08x; local crc %08x\n", crc_reply.arg[0], buf_crc);


  return 0;
}

void console(struct ftdi_context * ftdi)
{
  int res;

  ftdi->usb_read_timeout = 1;

  uint8_t read_buf[ftdi->readbuffer_chunksize];

  while (1) {
    res = ftdi_read_data(ftdi, read_buf, ftdi->readbuffer_chunksize);
    if (res > 0) {
      fwrite(read_buf, 1, res, stderr);
    }
  }
}

int main(int argc, char * argv[])
{
  if (argc < 2) {
    fprintf(stderr, "argc\n");
    return EXIT_FAILURE;
  }

  struct ftdi_context * ftdi;

  ftdi = ftdi_new();
  if (ftdi == 0) {
    fprintf(stderr, "ftdi_new\n");
    return EXIT_FAILURE;
  }

  int res;
  res = init_ftdi_context(ftdi);
  if (res < 0) {
    return EXIT_FAILURE;
  }

  int return_code = EXIT_SUCCESS;

  uint8_t * buf;
  uint32_t size;
  res = read_file(argv[1], &buf, &size);
  if (res < 0) {
    return EXIT_FAILURE;
  }

  struct timespec start;
  struct timespec end;
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  assert(res >= 0);
  int do_write_ret = do_write(ftdi, buf, size);
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  assert(res >= 0);
  fprintf(stderr, "do_write time: %.03f\n", timespec_difference(&end, &start));
  if (do_write_ret == 0) {
    //do_read(ftdi);
    do_jump(ftdi);
    console(ftdi);
  } else {
    return_code = EXIT_FAILURE;
  }

  res = clock_gettime(CLOCK_MONOTONIC, &end);
  assert(res >= 0);
  fprintf(stderr, "total time: %.03f\n", timespec_difference(&end, &start));

  return return_code;
}
