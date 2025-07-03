#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "../twiddle.hpp"

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

int write_file(const char * filename, const uint8_t * buf, uint32_t size)
{
  FILE * file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr, "fopen(\"%s\", \"wb\"): %s\n", filename, strerror(errno));
    return -1;
  }

  size_t write_size = fwrite(buf, 1, size, file);
  if (write_size != size) {
    fprintf(stderr, "fwrite: %d %ld %s\n", size, write_size, strerror(errno));
    return -1;
  }

  int ret = fclose(file);
  if (ret < 0) {
    fprintf(stderr, "fclose");
    return -1;
  }

  return 0;
}

int main(int argc, const char * argv[])
{
  if (argc != 3) {
    fprintf(stderr, "argc != 3\n");
    return -1;
  }

  uint8_t * buf;
  uint32_t size;
  int ret = read_file(argv[1], &buf, &size);
  assert(ret == 0);

  // fixme: add pgm support

  for (int i = 0; i < size; i++) {
    buf[i] = (buf[i] == 255) ? 1 : 0;
  }

  uint8_t * twiddle = (uint8_t *)malloc(size / 2);
  twiddle::texture_4bpp(twiddle, buf, 64, 32);

  ret = write_file(argv[2], twiddle, size / 2);
  assert(ret == 0);
}
