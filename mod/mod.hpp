#pragma once

#include "stdint.h"

struct sample
{
  const uint8_t * name;
  int length;
  int nibble;
  int volume;
  int repeat_offset;
  int repeat_length;
};

struct channel {
  uint16_t sample;
  uint16_t parameter;
  uint16_t effect;
  uint16_t _pad;
};

struct pattern
{
  struct channel division[64][4];
};

struct mod
{
  const uint8_t * title;
  struct sample samples[31];
  int song_positions;
  int ignored;
  uint8_t pattern_table[128];
  uint8_t tag[4];
  struct pattern patterns[128];
};

struct reader
{
  const uint8_t * buf;
  const int length;
  int ix;
};

void parse_mod_file(struct reader * r, struct mod * mod);
