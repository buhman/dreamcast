#include <inttypes.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <ftdi.h>
#include <libusb.h>

#include "crc32.h"
#include "serial_protocol.hpp"
#include "ftdi_transfer.hpp"
#include "ftdi_maple.hpp"

extern "C" int convert_baudrate_UT_export(int baudrate, struct ftdi_context *ftdi,
                                          unsigned short *value, unsigned short *index);

constexpr int current_cks = 0;
static int current_scbrr = -1;

double dreamcast_rate(int cks, int scbrr)
{
  assert(cks >= 0 && cks <= 3);
  assert(scbrr >= 0 && scbrr <= 255);

  double div = 1.0;
  for (; cks > 0; cks--) { div *= 4; };

  return 1562500.0 / (div * ((double)scbrr + 1.0));
}

int init_ftdi_context(struct ftdi_context * ftdi, uint32_t scbrr)
{
  ftdi_set_interface(ftdi, INTERFACE_ANY);
  struct ftdi_device_list * devlist;
  int num_devices;
  num_devices = ftdi_usb_find_all(ftdi, &devlist, 0, 0);
  if (num_devices < 0) {
    fprintf(stderr, "ftdi_usb_find_all: %d\n", num_devices);
    return -1;
  } else if (num_devices == 0) {
    fprintf(stderr, "ftdi_usb_find_all: zero matching devices\n");
    return -1;
  }

  struct libusb_device_descriptor desc;
  struct ftdi_device_list * devlist_item = devlist;
  struct libusb_device * dev = devlist_item->dev;
  int res;
  for (int i = 0; i < num_devices; i++) {
    res = libusb_get_device_descriptor(devlist_item->dev, &desc);
    if (res < 0) {
      fprintf(stderr, "libusb_get_device_descriptor: %d\n", res);
      return -1;
    }
    fprintf(stderr, "[%d]\n", i);
    fprintf(stderr, "  idVendor: %04x; idProduct: %04x;\n", desc.idVendor, desc.idProduct);

    uint8_t port_numbers[7];
    res = libusb_get_port_numbers(devlist_item->dev,
                                  port_numbers,
                                  (sizeof (port_numbers)));
    if (res < 0) {
      fprintf(stderr, "libusb_get_port_numbers: %d\n", res);
      return -1;
    }
    fprintf(stderr, "  libusb port number: ");
    for (int i = 0; i < res; i++) {
      if (i != 0) fprintf(stderr, ":");
      fprintf(stderr, "%o", port_numbers[i]);
    }
    fprintf(stderr, "\n");

    devlist_item = devlist_item->next;
  }

  assert(dev != NULL);
  res = ftdi_usb_open_dev(ftdi, devlist->dev);
  if (res < 0) {
    fprintf(stderr, "ftdi_usb_open_dev: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }
  ftdi_list_free(&devlist);

  res = ftdi_set_baudrate(ftdi, round(dreamcast_rate(current_cks, scbrr)));
  if (res < 0) {
    fprintf(stderr, "ftdi_set_baudrate: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }
  current_scbrr = scbrr;

  res = ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_1, NONE, BREAK_ON);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_line_property2: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }

  res = ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_1, NONE, BREAK_OFF);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_line_property2: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }

  /*
  res = ftdi_set_latency_timer(ftdi, 1);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_latency_timer %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }
  */

  res = ftdi_tciflush(ftdi);
  if (res < 0) {
    fprintf(stderr, "ftdi_tciflush: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }

  res = ftdi_tcoflush(ftdi);
  if (res < 0) {
    fprintf(stderr, "ftdi_tcoflush: %s\n", ftdi_get_error_string(ftdi));
    return -1;
  }

  uint8_t discard[1024];
  res = ftdi_read_data(ftdi, discard, (sizeof (discard)));
  assert(res >= 0);
  (void)discard;

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
  /*
  for (uint32_t i = 0; i < (sizeof (union serial_load::command_reply)); i++) {
    fprintf(stderr, "%02x ", cr.u8[i]);
  }
  fprintf(stderr, "\n");
  */
}

int read_reply(struct ftdi_context * ftdi, uint32_t expected_cmd, union serial_load::command_reply& reply)
{
  using namespace serial_load;

  constexpr long read_length = (sizeof (union serial_load::command_reply));

  long length = read_with_timeout(ftdi, reply.u8, read_length);
  if (length != read_length) {
    fprintf(stderr, "read_reply: short read; want %ld bytes; received: %ld\n", read_length, length);
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
  //dump_command_reply(reply);

  return 0;
}

int do_write(struct ftdi_context * ftdi, const uint32_t dest, const uint8_t * buf, const uint32_t size)
{
  int res;

  if (size > 0xffffff) {
    fprintf(stderr, "write: invalid size %d (bytes)\n", size);
    fprintf(stderr, "write size must be less than or equal to 16777215 bytes\n");
    return -1;
  }

  union serial_load::command_reply command = serial_load::command::write(dest, size);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));
  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::_write, reply);
  if (res != 0) {
    return -2;
  }
  if (reply.arg[0] != command.arg[0] || reply.arg[1] != command.arg[1]) {
    fprintf(stderr, "write: argument mismatch: (%08x, %08x) != (%08x, %08x)\n",
            reply.arg[0],   reply.arg[1],
            command.arg[0], command.arg[1]);
    return -1;
  }

  int clock_res;
  struct timespec start;
  //struct timespec end1;
  struct timespec end2;
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  assert(res == 0);
  res = ftdi_write_data(ftdi, buf, size);
  //clock_res = clock_gettime(CLOCK_MONOTONIC, &end1);
  //assert(clock_res == 0);
  assert(res >= 0);
  assert((uint32_t)res == size);

  uint32_t buf_crc = crc32(buf, size);

  union serial_load::command_reply crc_reply;
  res = read_reply(ftdi, serial_load::reply::_crc, crc_reply);
  clock_res = clock_gettime(CLOCK_MONOTONIC, &end2);
  assert(clock_res == 0);
  if (res != 0) {
    return -1;
  }
  fprintf(stderr, "remote crc: %08x; local crc %08x\n", crc_reply.arg[0], buf_crc);

  // one start bit, one stop bit, 8 data bits: 8/10
  unsigned short value;
  unsigned short index;
  double dreamcast_baud = dreamcast_rate(current_cks, current_scbrr);
  int ftdi_baud = convert_baudrate_UT_export(dreamcast_baud, ftdi, &value, &index);
  double idealized_baud = static_cast<double>(ftdi_baud) * 8.0 / 10.0;
  double idealized_time = static_cast<double>(size) * 8.0 / idealized_baud;

  //double measured_time1 = timespec_difference(&end1, &start);
  double measured_time2 = timespec_difference(&end2, &start);
  // subtract 128 bit-periods (16 bytes) to account for the time spent waiting for the read reply
  // at 1562500 bits per second, this subtracts an (insignificant) 102 µs from the displayed time
  double time_adjustment = (16.0 * 8.0) / (dreamcast_baud * 8.0 / 10.0);
  measured_time2 -= time_adjustment;

  fprintf(stderr, "%d bits/sec:\n", ftdi_baud);
  fprintf(stderr, "  idealized write time : %.03f  seconds\n", idealized_time);
  fprintf(stderr, "   measured write time : %.03f  seconds\n", measured_time2);

  if (crc_reply.arg[0] != buf_crc) {
    return -1;
  }

  return 0;
}

int do_jump(struct ftdi_context * ftdi, const uint32_t dest)
{
  int res;

  union serial_load::command_reply command = serial_load::command::jump(dest);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));

  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::_jump, reply);
  if (res != 0) {
    return -2;
  }
  if (reply.arg[0] != command.arg[0] || reply.arg[1] != command.arg[1]) {
    fprintf(stderr, "jump: argument mismatch: (%08x, %08x) != (%08x, %08x)\n",
            reply.arg[0],   reply.arg[1],
            command.arg[0], command.arg[1]);
    return -1;
  }

  return 0;
}

int do_show_baudrate_error(struct ftdi_context * ftdi, uint32_t rows)
{
  /*
    B = (390625 * 4^(1 - n)) / (N + 1)
   */

  fprintf(stderr, "\n");
  fprintf(stderr, " SH7091 baud |   FTDI baud |   error \n");
  fprintf(stderr, " ------------|-------------|---------\n");
  unsigned short value;
  unsigned short index;
  rows = min(rows, 256);
  for (uint32_t i = 0; i < rows; i++) {
    int baud = convert_baudrate_UT_export(dreamcast_rate(0, i), ftdi, &value, &index);
    if (baud < 0) {
      fprintf(stderr, "ftdi_convert_baudrate: %d\n", baud);
      return -1;
    }
    double baudf = baud;
    double ratef = dreamcast_rate(0, i);
    double error = (baudf - ratef) / ratef * 100.0;
    fprintf(stderr, " %11.00f   %11.00f    % 02.03f%%\n", round(ratef), baudf, error);
  }

  fprintf(stderr, "\n  \"\n  Note: As far as possible, the setting should be made so that the\n  error is within 1%%.\n  \"\n");
  fprintf(stderr, "    - SH7091 Hardware Manual, 03/02/1999, page 486\n");

  return 0;
}

int do_list_baudrates(struct ftdi_context * ftdi, uint32_t rows)
{
  (void)ftdi;

  fprintf(stderr, "   scbrr |      cks 0 |     cks 1 |     cks 2 |     cks 3\n");
  fprintf(stderr, "---------------------------------------------------------\n");
  rows = min(rows, 256);
  for (uint32_t i = 0; i < rows; i++) {
    fprintf(stderr, "    0x%02x  % 11.2f % 11.2f % 11.2f % 11.2f\n",
            i,
            dreamcast_rate(0, i),
            dreamcast_rate(1, i),
            dreamcast_rate(2, i),
            dreamcast_rate(3, i));
  }
  return 0;
}

int read_file(const char * filename, uint8_t ** buf, uint32_t * size_out)
{
  FILE * file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "fopen(\"%s\", \"rb\"): %s\n", filename, strerror(errno));
    return -1;
  }

  int ret;
  ret = fseek(file, 0L, SEEK_END);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_END)");
    return -1;
  }

  long offset = ftell(file);
  if (offset < 0) {
    fprintf(stderr, "ftell");
    return -1;
  }
  size_t size = offset;

  ret = fseek(file, 0L, SEEK_SET);
  if (ret < 0) {
    fprintf(stderr, "fseek(SEEK_SET)");
    return -1;
  }

  fprintf(stderr, "read_file: %s size %ld\n", filename, size);
  *buf = (uint8_t *)malloc(size);
  size_t fread_size = fread(*buf, 1, size, file);
  if (fread_size != size) {
    fprintf(stderr, "fread `%s` short read: %" PRIu64 " ; expected: %" PRIu64 "\n", filename, fread_size, size);
    return -1;
  }

  ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "fclose");
    return -1;
  }

  *size_out = size;

  return 0;
}

int write_file(const char * filename, uint8_t * buf, uint32_t size)
{
  FILE * file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr, "fopen(\"%s\", \"wb\"): %s\n", filename, strerror(errno));
    return -1;
  }

  size_t fwrite_size = fwrite(buf, 1, size, file);
  if (fwrite_size != size) {
    fprintf(stderr, "fwrite `%s` short write: %" PRIu64 " ; expected: %" PRIu32 "\n", filename, fwrite_size, size);
    return -1;
  }

  int ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "fclose");
    return -1;
  }

  return 0;
}

int do_read(struct ftdi_context * ftdi, const uint32_t src, uint8_t * buf, const uint32_t size)
{
  int res;

  if (size > 0xffffff) {
    fprintf(stderr, "read: invalid size %d (bytes)\n", size);
    fprintf(stderr, "read size must be less than or equal to 16777215 bytes\n");
    return -1;
  }

  union serial_load::command_reply command = serial_load::command::read(src, size);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));
  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::_read, reply);
  if (res != 0) {
    return -2;
  }
  if (reply.arg[0] != command.arg[0] || reply.arg[1] != command.arg[1]) {
    fprintf(stderr, "read: argument mismatch: (%08x, %08x) != (%08x, %08x)\n",
            reply.arg[0],   reply.arg[1],
            command.arg[0], command.arg[1]);
    return -1;
  }

  uint32_t read_length = 0;
  while (read_length < size) {
    res = ftdi_read_data(ftdi, (uint8_t *)&buf[read_length], size - read_length);
    assert(res >= 0);
    read_length += res;
    if (read_length < size)
      fprintf(stderr, "read: short read; want %x out of %x\n", size - read_length, size);
  }

  uint32_t buf_crc = crc32((uint8_t*)buf, size);

  union serial_load::command_reply crc_reply;
  res = read_reply(ftdi, serial_load::reply::_crc, crc_reply);
  if (res != 0) {
    return -1;
  }

  fprintf(stderr, "remote crc: %08x; local crc %08x\n", crc_reply.arg[0], buf_crc);
  if (crc_reply.arg[0] != buf_crc) {
    return -1;
  }

  return 0;
}

int do_speed(struct ftdi_context * ftdi, uint32_t scbrr)
{
  int res;

  if (scbrr > 255) {
    fprintf(stderr, "speed: invalid speed %d\n", scbrr);
    fprintf(stderr, "speed is expressed as a raw SCBRR value; see `list_baudrates`\n");
    return -1;
  }

  union serial_load::command_reply command = serial_load::command::speed(scbrr);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));

  union serial_load::command_reply reply;
  res = read_reply(ftdi, serial_load::reply::_speed, reply);
  if (res != 0) {
    return -2;
  }

  if (reply.arg[0] != command.arg[0] || reply.arg[1] != command.arg[1]) {
    fprintf(stderr, "speed: argument mismatch: (%08x, %08x) != (%08x, %08x)\n",
            reply.arg[0],   reply.arg[1],
            command.arg[0], command.arg[1]);
    return -1;
  }

  res = ftdi_set_baudrate(ftdi, round(dreamcast_rate(current_cks, scbrr)));
  if (res < 0) {
    fprintf(stderr, "ftdi_set_baudrate\n");
    return -1;
  }
  current_scbrr = scbrr;

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

  uint8_t discard[1024];
  res = ftdi_read_data(ftdi, discard, (sizeof (discard)));
  assert(res >= 0);
  (void)discard;

  return 0;
}

void do_console(struct ftdi_context * ftdi)
{
  int res;

  uint8_t read_buf[ftdi->readbuffer_chunksize];

  while (1) {
    res = ftdi_read_data(ftdi, read_buf, ftdi->readbuffer_chunksize);
    if (res < 0) {
      fprintf(stderr, "ftdi_read_data: %s\n", ftdi_get_error_string(ftdi));
    }
    assert(res >= 0);
    if (res > 0) {
      fwrite(read_buf, 1, res, stdout);
      fflush(stdout);
    }
  }
}

int do_maple_raw(struct ftdi_context * ftdi,
                 uint8_t * send_buf,
                 uint32_t send_size,
                 uint8_t * recv_buf,
                 uint32_t recv_size)
{
  int res;

  union serial_load::command_reply command = serial_load::command::maple_raw(send_size, recv_size);
  //dump_command_reply(command);
  res = ftdi_write_data(ftdi, command.u8, (sizeof (command)));
  assert(res == (sizeof (command)));
  union serial_load::command_reply reply;
  fprintf(stderr, "maple_raw: wait maple_raw reply\n");
  res = read_reply(ftdi, serial_load::reply::_maple_raw, reply);
  if (res != 0) {
    return -2;
  }
  if (reply.arg[0] != command.arg[0] || reply.arg[1] != command.arg[1]) {
    fprintf(stderr, "maple_raw: argument mismatch: (%08x, %08x) != (%08x, %08x)\n",
            reply.arg[0],   reply.arg[1],
            command.arg[0], command.arg[1]);
    return -1;
  }

  res = ftdi_write_data(ftdi, send_buf, send_size);
  assert(res >= 0);
  assert((uint32_t)res == send_size);

  uint32_t send_buf_crc = crc32(send_buf, send_size);
  /*
  fprintf(stderr, "send_size: %d\n", send_size);
  for (uint32_t i = 0; i < send_size; i++) {
    fprintf(stderr, "%02x ", send_buf[i]);
    if (i % 4 == 3)
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
  */

  union serial_load::command_reply send_crc_reply;
  fprintf(stderr, "maple_raw: send: wait crc reply\n");
  res = read_reply(ftdi, serial_load::reply::_crc, send_crc_reply);
  if (res != 0) {
    return -1;
  }
  fprintf(stderr, "maple_raw: send: remote crc: %08x; local crc %08x\n", send_crc_reply.arg[0], send_buf_crc);
  if (send_crc_reply.arg[0] != send_buf_crc) {
    dump_command_reply(send_crc_reply);
    return -1;
  }

  uint32_t read_length = 0;
  while (read_length < recv_size) {
    res = ftdi_read_data(ftdi, &recv_buf[read_length], recv_size - read_length);
    assert(res >= 0);
    read_length += res;
    if (read_length < recv_size)
      fprintf(stderr, "maple raw: short read; want %x out of %x\n", recv_size - read_length, recv_size);
  }

  uint32_t recv_buf_crc = crc32(recv_buf, recv_size);

  union serial_load::command_reply recv_crc_reply;
  fprintf(stderr, "maple_raw: recv: wait crc reply\n");
  res = read_reply(ftdi, serial_load::reply::_crc, recv_crc_reply);
  if (res != 0) {
    return -1;
  }

  fprintf(stderr, "maple_raw: recv: remote crc: %08x; local crc %08x\n", recv_crc_reply.arg[0], recv_buf_crc);
  if (recv_crc_reply.arg[0] != recv_buf_crc) {
    return -1;
  }

  return 0;
}

enum struct argument_type {
  string,
  integer
};

struct cli_command {
  const char * name;
  int num_arguments;
  void * func;
};

struct cli_command commands[] = {
  { "read"               , 3, (void *)&do_read                 },
  { "write"              , 2, (void *)&do_write                },
  { "jump"               , 1, (void *)&do_jump                 },
  { "speed"              , 1, (void *)&do_speed                },
  { "console"            , 0, (void *)&do_console              },
  { "maple_storage_dump" , 0, (void *)&do_maple_storage_dump   },
  { "list_baudrates"     , 1, (void *)&do_list_baudrates       },
  { "show_baudrate_error", 1, (void *)&do_show_baudrate_error  },
};

constexpr int commands_length = (sizeof (commands)) / (sizeof (commands[0]));

typedef int (*func_0_arg)(struct ftdi_context *);
typedef int (*func_1_arg)(struct ftdi_context *, uint32_t);
typedef int (*func_2_arg)(struct ftdi_context *, uint32_t, uint8_t *, uint32_t);
typedef int (*func_3_arg)(struct ftdi_context *, uint32_t, uint32_t, uint8_t *, uint32_t);

int parse_integer(const char * s, uint32_t * value)
{
  if (s[0] == '0' && s[1] == 'x') {
    s = &s[2];
  }

  uint32_t n = 0;
  while (*s != 0) {
    char c = *s++;
    n = n << 4;
    switch (c) {
    case '0': n += 0; break;
    case '1': n += 1; break;
    case '2': n += 2; break;
    case '3': n += 3; break;
    case '4': n += 4; break;
    case '5': n += 5; break;
    case '6': n += 6; break;
    case '7': n += 7; break;
    case '8': n += 8; break;
    case '9': n += 9; break;
    case 'A': [[fallthrough]];
    case 'a': n += 0xa; break;
    case 'B': [[fallthrough]];
    case 'b': n += 0xb; break;
    case 'C': [[fallthrough]];
    case 'c': n += 0xc; break;
    case 'D': [[fallthrough]];
    case 'd': n += 0xd; break;
    case 'E': [[fallthrough]];
    case 'e': n += 0xe; break;
    case 'F': [[fallthrough]];
    case 'f': n += 0xf; break;
    default:
      return -1;
    }
  }

  *value = n;
  return 0;
}

#define CHECK_ARGC(__name__) \
  if (arg_index >= argc) { \
    fprintf(stderr, "while processing command `%s` expected argument `%s`\n", name, #__name__); \
    return -1; \
  }

#define INTEGER_ARGUMENT(__name__) \
  CHECK_ARGC(__name__); \
  uint32_t __name__; \
  const char * __name__##str = argv[arg_index++]; \
  { int res = parse_integer(__name__##str, &__name__);   \
  if (res < 0) { \
    fprintf(stderr, "while processing command `%s` expected integer at `%s`", name, __name__##str); \
    return -1; \
  } }

#define STRING_ARGUMENT(__name__) \
  CHECK_ARGC(__name__); \
  const char * __name__ = argv[arg_index++];

int handle_command(int argc, const char * argv[], struct ftdi_context * ftdi)
{
  assert(argc >= 1);
  int arg_index = 0;
  const char * name = argv[arg_index++];
  int func_ret;

  for (int i = 0; i < commands_length; i++) {
    if (strcmp(commands[i].name, name) == 0) {
      switch (commands[i].num_arguments) {
      case 0:
        {
          fprintf(stderr, "handle command: %s ()\n", commands[i].name);
          func_0_arg func = (func_0_arg)commands[i].func;
          fprintf(stderr, "%p\n", &do_console);
          fprintf(stderr, "%p\n", func);
          func_ret = func(ftdi);
        }
        break;
      case 1:
        {
          INTEGER_ARGUMENT(arg0);

          fprintf(stderr, "handle command: %s (0x%08x)\n", commands[i].name, arg0);
          func_1_arg func = (func_1_arg)commands[i].func;
          func_ret = func(ftdi, arg0);
        }
        break;
      case 2:
        {
          INTEGER_ARGUMENT(dest_addr);
          STRING_ARGUMENT(filename);

          uint8_t * buf = NULL;
          uint32_t write_size;
          int res = read_file(filename, &buf, &write_size);
          if (res < 0) {
            return -1;
          }

          fprintf(stderr, "handle command: %s (0x%08x, %s)\n", commands[i].name, dest_addr, filename);
          func_2_arg func = (func_2_arg)commands[i].func;
          func_ret = func(ftdi, dest_addr, buf, write_size);

          assert(buf != NULL);
          free(buf);
        }
        break;
      case 3:
        {
          INTEGER_ARGUMENT(src_addr);
          INTEGER_ARGUMENT(read_size);
          STRING_ARGUMENT(filename);

          uint8_t * buf = (uint8_t *)malloc(read_size);

          fprintf(stderr, "handle command %s (0x%08x, 0x%08x) → %s\n", commands[i].name, src_addr, read_size, filename);
          func_2_arg func = (func_2_arg)commands[i].func;
          func_ret = func(ftdi, src_addr, buf, read_size);

          int res = write_file(filename, buf, read_size);
          if (res < 0) {
            return -1;
          }

          assert(buf != NULL);
          free(buf);
        }
        break;
      default:
        assert(false); // unimplemented
      }

      if (func_ret < 0)
        return func_ret;
      else
        return arg_index;
    }
  }

  fprintf(stderr, "unknown command `%s`\n", name);
  return -1;
}

int main(int argc, const char * argv[])
{
  struct ftdi_context * ftdi;

  ftdi = ftdi_new();
  if (ftdi == 0) {
    fprintf(stderr, "ftdi_new\n");
    return EXIT_FAILURE;
  }

  int res;
  res = init_ftdi_context(ftdi, 0);
  if (res < 0) {
    return EXIT_FAILURE;
  }

  assert(argc >= 1);
  argc--;
  argv++;
  while (argc > 0) {
    res = handle_command(argc, argv, ftdi);
    if (res < 0) {
      return -1;
    }
    argc -= res;
    argv += res;
  }

  return 0;
}
