#include "mod.hpp"
#include "assert.h"

static const uint8_t * reader_s(struct reader * r, int length)
{
  const uint8_t * ptr = &r->buf[r->ix];
  r->ix += length;
  return ptr;
}

static void reader_bytes(struct reader * r, int length, uint8_t * dst)
{
  const uint8_t * ptr = reader_s(r, length);
  for (int i = 0; i < length; i++)
    dst[i] = ptr[i];
}

static int reader_u8(struct reader * r)
{
  const uint8_t * ptr = reader_s(r, 1);
  return *ptr;
}

static int reader_u16(struct reader * r)
{
  const uint8_t * ptr = reader_s(r, 2);
  int value = (((uint32_t)ptr[0]) << 8) | (((uint32_t)ptr[1]) << 0);
  return value;
}

static void parse_sample(struct reader * r, struct sample * s)
{
  s->name = reader_s(r, 22);
  s->length = reader_u16(r);
  s->nibble = reader_u8(r);
  s->volume = reader_u8(r);
  s->repeat_offset = reader_u16(r);
  s->repeat_length = reader_u16(r);
}

static int parse_num_patterns(uint8_t * table)
{
  int max = 0;
  for (int i = 0; i < 128; i++) {
    if ((int)table[i] > max) {
      max = table[i];
    }
  }
  return max + 1;
}

static void parse_channel(const uint8_t * data, struct channel * c)
{
  uint32_t w = ((uint32_t)data[0] >> 4) & 0xf;
  uint32_t x = ((uint32_t)data[1]) | (((uint32_t)data[0] & 0xf) << 8);
  uint32_t y = ((uint32_t)data[2] >> 4) & 0xf;
  uint32_t z = ((uint32_t)data[3]) | (((uint32_t)data[2] & 0xf) << 8);

  c->sample = (w << 4) | y;
  c->parameter = x;
  c->effect = z;
}

static void parse_pattern(struct reader * r, struct pattern * pattern)
{
  const uint8_t * raw_pattern = reader_s(r, 1024);
  for (int division = 0; division < 64; division++) {
    int ix = division * 16;
    parse_channel(&raw_pattern[ix+0 ], &pattern->division[division][0]);
    parse_channel(&raw_pattern[ix+4 ], &pattern->division[division][1]);
    parse_channel(&raw_pattern[ix+8 ], &pattern->division[division][2]);
    parse_channel(&raw_pattern[ix+12], &pattern->division[division][3]);
  }
}

void parse_mod_file(struct reader * r, struct mod * mod)
{
  mod->title = reader_s(r, 20);

  for (int i = 0; i < 31; i++) {
    parse_sample(r, &mod->samples[i]);
  }

  mod->song_positions = reader_u8(r);
  mod->ignored = reader_u8(r);
  reader_bytes(r, 128, mod->pattern_table);

  assert(r->ix == 0x438);
  reader_bytes(r, 4, mod->tag);

  assert(mod->tag[0] == 'M' && mod->tag[1] == '.' &&
         mod->tag[2] == 'K' && mod->tag[3] == '.');

  int num_patterns = parse_num_patterns(mod->pattern_table);
  for (int i = 0; i < num_patterns; i++) {
    parse_pattern(r, &mod->patterns[i]);
  }
}
