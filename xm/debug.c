#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "xm.h"

int read_file(const char * filename, void ** buf, uint32_t * size_out)
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
    fprintf(stderr, "fread `%s` short read: %d ; expected: %d\n", filename, (int)fread_size, (int)size);
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

int s16(void * buf)
{
  uint8_t * b = (uint8_t *)buf;
  int16_t v = (b[0] << 0) | (b[1] << 8);
  return v;
}

int s32(void * buf)
{
  uint8_t * b = (uint8_t *)buf;
  int32_t v = (b[0] << 0) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  return v;
}

void print_chars(int8_t * chars, int length, const char * end)
{
  for (int i = 0; i < length; i++) {
    int8_t c = chars[i];
    if (c >= 0x20 && c <= 0x7e) {
      fputc(c, stdout);
    } else {
      printf("\\x%02x", c);
    }
  }
  if (end != NULL)
    fputs(end, stdout);
}

void debug_header(void * buf)
{
  xm_header_t * header = (xm_header_t *)buf;
  printf("header:\n");
  printf("  id_text: '");
  print_chars(header->id_text, 17, "'\n");
  printf("  module_name: '");
  print_chars(header->module_name, 20, "'\n");
  printf("  xm_type: 0x%02x\n", header->xm_type);
  printf("  tracker_name: '");
  print_chars(header->tracker_name, 20, "'\n");
  printf("  version_number: 0x%04x\n", s16(&header->version_number));
  printf("  header_size: %d\n", s32(&header->header_size));
  printf("  song_length: %d\n", s16(&header->song_length));
  printf("  restart_position: %d\n", s16(&header->restart_position));
  printf("  number_of_channels: %d\n", s16(&header->number_of_channels));
  printf("  number_of_patterns: %d\n", s16(&header->number_of_patterns));
  printf("  number_of_instruments: %d\n", s16(&header->number_of_instruments));
  printf("  flags: %d\n", s16(&header->flags));
  printf("  default_tempo: %d\n", s16(&header->default_tempo));
  printf("  default_bpm: %d\n", s16(&header->default_bpm));
}

static xm_pattern_format_t column[8];

void debug_pattern_format(int note_ix, xm_pattern_format_t * pf)
{
  /*
  printf("note[%d]\n", note_ix);
  printf("  note: %d\n", pf->note);
  printf("  instrument: %d\n", pf->instrument);
  printf("  volume_column_byte: %d\n", pf->volume_column_byte);
  printf("  effect_type: %d\n", pf->effect_type);
  printf("  effect_parameter: %d\n", pf->effect_parameter);
  */
  column[note_ix & 7] = *pf;
  if ((note_ix & 7) == 7) {
    printf("%3d |", note_ix / 8);
    for (int i = 0; i < 8; i++)
      printf(" n:%2d i:%2d |",
             column[i].note, column[i].instrument);
    printf("\n");
  }
}

void debug_pattern(xm_pattern_header_t * pattern_header)
{
  printf("    | channel 0 | channel 1 | channel 2 | channel 3 | channel 4 | channel 5 | channel 6 | channel 7 |\n");
  uint8_t * pattern = (uint8_t *)(((ptrdiff_t)pattern_header) + s32(&pattern_header->pattern_header_length));
  int ix = 0;
  int note_ix = 0;
  while (ix < s16(&pattern_header->packed_pattern_data_size)) {
    int p = pattern[ix];
    if (p & 0x80) {
      ix += 1;
      xm_pattern_format_t pf = {};
      if (p & (1 << 0))
        pf.note = pattern[ix++];
      if (p & (1 << 1))
        pf.instrument = pattern[ix++];
      if (p & (1 << 2))
        pf.volume_column_byte = pattern[ix++];
      if (p & (1 << 3))
        pf.effect_type = pattern[ix++];
      if (p & (1 << 4))
        pf.effect_parameter = pattern[ix++];
      debug_pattern_format(note_ix, &pf);
    } else {
      xm_pattern_format_t * pf = (xm_pattern_format_t *)&pattern[ix];
      debug_pattern_format(note_ix, pf);
      ix += 5;
    }
    note_ix += 1;
  }
  assert(ix == s16(&pattern_header->packed_pattern_data_size));
}

int debug_pattern_headers(void * buf)
{
  xm_header_t * header = (xm_header_t *)buf;
  int pattern_header_offset = s32(&header->header_size) + (offsetof (struct xm_header, header_size));

  for (int i = 0; i < s16(&header->number_of_patterns); i++) {
    xm_pattern_header_t * pattern_header = (xm_pattern_header_t *)(((ptrdiff_t)buf) + pattern_header_offset);

    printf("pattern_header[%d]:\n", i);
    printf("  pattern_header_length: %d\n", s32(&pattern_header->pattern_header_length));
    printf("  packing_type: %d\n", pattern_header->packing_type);
    printf("  number_of_rows_in_pattern: %d\n", s16(&pattern_header->number_of_rows_in_pattern));
    printf("  packed_pattern_data_size: %d\n", s16(&pattern_header->packed_pattern_data_size));
    //debug_pattern(pattern_header);
    pattern_header_offset += s32(&pattern_header->pattern_header_length) + s16(&pattern_header->packed_pattern_data_size);
  }
  return pattern_header_offset;
}

void write_file(const char * filename, void * buf, int size)
{
  printf("write %s\n", filename);
  FILE * file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr, "fopen(\"%s\", \"wb\"): %s\n", filename, strerror(errno));
    return;
  }

  size_t write = fwrite(buf, 1, size, file);
  assert(write == size);

  int ret = fclose(file);
  assert(ret == 0);
}

int saturation16(int v)
{
  /*
  if (v > 32767)
    return 32767;
  if (v < -32768)
    return -32768;
  */
  return v;
}

int saturation8(int v)
{
  /*
  if (v > 127)
    return 127;
  if (v < -128)
    return -128;
  */
  return v;
}

void dump_sample(void * buf, int offset, int sample_ix, xm_sample_header_t * sample_header)
{
  assert(sample_header->sample_data_type == 0);
  int old = 0;
  int size = s32(&sample_header->sample_length);
  printf("%d offset %d\n", sample_ix, offset);
  if (sample_header->type & (1 << 4)) { // 16-bit samples
    int num_samples = size / 2;
    int old = 0;
    int16_t out[num_samples];
    int16_t * in = (int16_t *)(((ptrdiff_t)buf) + offset);
    for (int i = 0; i < num_samples; i++) {
      old += in[i];
      out[i] = saturation16(old);
    }
    char filename[64];
    snprintf(filename, 64, "sample%03d.s16le.pcm", sample_ix);
    write_file(filename, out, size);

  } else { // 8-bit
    int num_samples = size;
    int old = 0;
    int8_t out[num_samples];
    int8_t * in = (int8_t *)(((ptrdiff_t)buf) + offset);
    for (int i = 0; i < num_samples; i++) {
      old += in[i];
      out[i] = old;
    }
    char filename[64];
    snprintf(filename, 64, "sample%03d.s8.pcm", sample_ix);
    write_file(filename, out, size);
  }
}

int debug_samples(void * buf, int offset, int instrument_ix, int number_of_samples)
{
  xm_sample_header_t * sample_header[number_of_samples];
  printf("A offset %d\n", offset);
  for (int i = 0; i < number_of_samples; i++) {
    sample_header[i] = (xm_sample_header_t *)(((ptrdiff_t)buf) + offset);
    printf("  sample header %d offset %d\n", i, offset);
    printf("  sample[%d]\n", i);
    printf("    sample_length: %d\n", s32(&sample_header[i]->sample_length));
    printf("    sample_loop_start: %d\n", s32(&sample_header[i]->sample_loop_start));
    printf("    sample_loop_length: %d\n", s32(&sample_header[i]->sample_loop_length));
    printf("    volume: %d\n", sample_header[i]->volume);
    printf("    finetune: %d\n", sample_header[i]->finetune);
    printf("    type: %d\n", sample_header[i]->type);
    printf("    panning: %d\n", sample_header[i]->panning);
    printf("    relative_note_number: %d\n", sample_header[i]->relative_note_number);
    printf("    sample_data_type: %d\n", sample_header[i]->sample_data_type);
    printf("    sample_name: '");
    print_chars(sample_header[i]->sample_name, 22, "'\n");
    offset += (sizeof (xm_sample_header_t));
  }

  printf("B offset %d\n", offset);
  for (int i = 0; i < number_of_samples; i++) {
    if (s32(&sample_header[i]->sample_length) > 0)
      dump_sample(buf, offset, instrument_ix, sample_header[i]);
    offset += s32(&sample_header[i]->sample_length);
  }
  return offset;
}

int debug_instruments(void * buf, int offset)
{
  xm_header_t * header = (xm_header_t *)buf;

  for (int i = 0; i < s16(&header->number_of_instruments); i++) {
    printf("instrument offset %d: %d\n", i, offset);
    xm_instrument_header_t * instrument_header = (xm_instrument_header_t *)(((ptrdiff_t)buf) + offset);

    printf("instrument[%d]\n", i);
    printf("  instrument_size: %d\n", s32(&instrument_header->instrument_size));
    printf("  instrument_name: '");
    print_chars(instrument_header->instrument_name, 22, "'\n");
    printf("  instrument_type: %d\n", instrument_header->instrument_type);
    printf("  number_of_samples: %d\n", s16(&instrument_header->number_of_samples));

    offset += s32(&instrument_header->instrument_size);
    printf("this offset %d\n", offset);

    if (s16(&instrument_header->number_of_samples) > 0) {
      printf("  sample_header_size: %d\n", s32(&instrument_header->sample_header_size));

      offset = debug_samples(buf, offset, i, s16(&instrument_header->number_of_samples));
    }
  }
}

int main(int argc, const char *argv[])
{
  assert(argc == 2);
  const char * filename = argv[1];

  void * buf;
  uint32_t size;
  int res = read_file(filename, &buf, &size);
  if (res != 0)
    return EXIT_FAILURE;

  debug_header(buf);
  int end_of_patterns = debug_pattern_headers(buf);
  debug_instruments(buf, end_of_patterns);
}
