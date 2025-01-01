#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

#include "crc32.c"

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

int main(int argc, char * argv[])
{
  assert(argc == 2);

  uint8_t * buf;
  uint32_t size;
  int ret = read_file(argv[1], &buf, &size);
  assert(ret == 0);

  printf("size: %08x\n", size);
  int chunks = size / 2048;
  printf("crc:\n");
  for (int i = 0; i < chunks; i++) {
    uint32_t crc = crc32(&buf[i * 2048], 2048);
    printf("0x%08x\n", crc);
  }
}
