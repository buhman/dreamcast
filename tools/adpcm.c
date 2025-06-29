#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

static inline int clamp(int x, int low, int high)
{
  if (x > high)
    return high;
  if (x < low)
    return low;
  return x;
}

static inline int ymz_step(uint8_t step, int16_t * history, int16_t * step_size)
{
  static const int step_table[8] = {
    230, 230, 230, 230, 307, 409, 512, 614
  };

  int sign = step & 8;
  int delta = step & 7;
  int diff = ((1 + (delta << 1)) * *step_size) >> 3;
  int newval = *history;
  int nstep = (step_table[delta] * *step_size) >> 8;
  diff = clamp(diff, 0, 32767);
  if (sign > 0)
    newval -= diff;
  else
    newval += diff;
  *step_size = clamp(nstep, 127, 24576);
  *history = newval = clamp(newval, -32768, 32767);
  return newval;
}

void adpcm_encode(int16_t * buffer, uint8_t * outbuffer, int len)
{
  int16_t step_size = 127;
  int16_t history = 0;
  uint8_t buf_sample = 0, nibble = 0;
  unsigned int adpcm_sample;

  for (int i = 0; i < len; i++) {
    //int step = ((*buffer++) & -8) - history;
    int step = *buffer++ - history;
    adpcm_sample = (abs(step) << 16) / (step_size << 14);
    adpcm_sample = clamp(adpcm_sample, 0, 7);
    if(step < 0)
      adpcm_sample |= 8;
    if(!nibble)
      *outbuffer++ = buf_sample | (adpcm_sample << 4);
    else
      buf_sample = (adpcm_sample & 15);
    nibble ^= 1;
    ymz_step(adpcm_sample, &history, &step_size);
  }
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

int main(int argc, char * argv[])
{
  assert(argc == 3);

  uint8_t * buf;
  int size;
  int ret = read_file(argv[1], &buf, &size);
  assert(ret == 0);

  uint8_t * out = malloc(size / 4);

  adpcm_encode((uint16_t *)buf, out, size / 2);

  ret = write_file(argv[2], out, size / 4);
  assert(ret == 0);
}
