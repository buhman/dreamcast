#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include <ftdi.h>
#include <libusb.h>

extern int convert_baudrate_UT_export(int baudrate, struct ftdi_context *ftdi,
				      unsigned short *value, unsigned short *index);

int dreamcast_rates[] = {
  1562500,
  781250,
  520833,
  390625,
  312500,
  260416,
  223214,
  195312,
  173611,
  156250,
  142045,
  130208,
  120192
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

  unsigned short value;
  unsigned short index;

  for (unsigned int i = 0; i < (sizeof (dreamcast_rates)) / (sizeof (dreamcast_rates[0])); i++) {
    int baud = convert_baudrate_UT_export(dreamcast_rates[i], ftdi, &value, &index);
    float baudf = baud;
    float ratef = dreamcast_rates[i];
    float error = (baudf > ratef) ? ratef / baudf : baudf / ratef;
    fprintf(stdout, "%d: best: %d, error: %f\n", dreamcast_rates[i], baud, (1.f - error) * 100.f);
  }

  res = ftdi_set_baudrate(ftdi, 1562500);
  //res = ftdi_set_baudrate(ftdi, 312500);
  if (res < 0) {
    fprintf(stderr, "ftdi_set_baudrate\n");
    return -1;
  }

  res = ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
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
static_assert((sizeof (union data_command)) == 4 * 3);

uint32_t bswap(const uint32_t n)
{
  if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return n;
  else
    return __builtin_bswap32(n);
}

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

void symmetric(struct ftdi_context * ftdi, const uint8_t * tx_buf, const long size)
{
  int res;
  uint8_t rx_buf[size];
  long tx_offset = 0;
  long rx_offset = 0;

  while (tx_offset < size) {
    long txrx_diff = tx_offset - rx_offset;
    long tx_length = max(min(min(chunk_size, size - tx_offset), chunk_size - txrx_diff), 0);

    if (tx_length > 0) {
      res = ftdi_write_data(ftdi, &tx_buf[tx_offset], tx_length);
      assert(res >= 0);
      tx_offset += res;
    }

    res = ftdi_read_data(ftdi, &rx_buf[rx_offset], size - rx_offset);
    assert(res >= 0);
    rx_offset += res;
  }

  for (int i = 0; i < size; i++) {
    if (tx_buf[i] != rx_buf[i]) {
      fprintf(stderr, "mismatch at %d\n", i);
      return;
    }
  }
  fprintf(stderr, "equal\n");
}

double timespec_difference(struct timespec const * const a, struct timespec const * const b)
{
  return (double)(a->tv_sec - b->tv_sec) + (double)(a->tv_nsec - b->tv_nsec) / 1'000'000'000.0;
}

int transfer(struct ftdi_context * ftdi, const uint8_t * buf, const long size)
{
  int res;

  union data_command command = {
    .command = {'D', 'A', 'T', 'A'},
    .size = bswap(size),
    .dest = bswap(0xac010000),
  };

  res = ftdi_write_data(ftdi, command.data, (sizeof (union data_command)));
  assert(res >= 0);

  const char * expect = "data\n";
  const long expect_length = 5;
  uint8_t read_buf[expect_length + 1];
  read_buf[expect_length] = 0;
  long read_length = read_with_timeout(ftdi, read_buf, expect_length);
  if (read_length != expect_length) {
    fprintf(stderr, "want %ld bytes; received: %ld\n", expect_length, read_length);
    return -1;
  }
  res = memcmp(read_buf, expect, expect_length);
  if (res != 0) {
    fprintf(stderr, "expect `%s`; received: `%s`\n", expect, read_buf);
    return -1;
  }

  fprintf(stderr, "OK\n");

  struct timespec start;
  struct timespec end;
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  symmetric(ftdi, buf, size);
  res = clock_gettime(CLOCK_MONOTONIC, &end);
  fprintf(stderr, "symmetric time: %.03f\n", timespec_difference(&end, &start));


  return 0;
}

int main(int argc, char * argv[])
{
  if (argc < 2) {
    fprintf(stderr, "argc\n");
    return EXIT_FAILURE;
  }

  FILE * file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "fopen\n");
    return EXIT_FAILURE;
  }

  int ret;
  ret = fseek(file, 0L, SEEK_END);
  if (ret < 0) {
    fprintf(stderr, "seek(SEEK_END)");
    return EXIT_FAILURE;
  }

  long off = ftell(file);

  ret = fseek(file, 0L, SEEK_SET);
  if (ret < 0) {
    fprintf(stderr, "seek(SEEK_SET)");
    return EXIT_FAILURE;
  }

  fprintf(stderr, "%s off %ld\n", argv[1], off);
  uint8_t buf[off];
  ssize_t size = fread(buf, 1, off, file);
  if (size < 0) {
    fprintf(stderr, "read");
    return EXIT_FAILURE;
  }

  ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "close");
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

  struct timespec start;
  struct timespec end;
  res = clock_gettime(CLOCK_MONOTONIC, &start);
  int transfer_ret = transfer(ftdi, buf, off);
  res = clock_gettime(CLOCK_MONOTONIC, &end);

  fprintf(stderr, "time: %.03f\n", timespec_difference(&end, &start));

  return transfer_ret;
}
