#pragma once

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) xm_header {
  int8_t id_text[17];
  int8_t module_name[20];
  uint8_t xm_type;
  int8_t tracker_name[20];
  int16_t version_number;
  int32_t header_size;
  int16_t song_length;
  int16_t restart_position;
  int16_t number_of_channels;
  int16_t number_of_patterns;
  int16_t number_of_instruments;
  int16_t flags;
  int16_t default_tempo;
  int16_t default_bpm;
  uint8_t pattern_order_table[];
} xm_header_t;

static_assert((offsetof (struct xm_header, id_text)) == 0);
static_assert((offsetof (struct xm_header, module_name)) == 17);
static_assert((offsetof (struct xm_header, xm_type)) == 37);
static_assert((offsetof (struct xm_header, tracker_name)) == 38);
static_assert((offsetof (struct xm_header, version_number)) == 58);
static_assert((offsetof (struct xm_header, header_size)) == 60);
static_assert((offsetof (struct xm_header, song_length)) == 64);
static_assert((offsetof (struct xm_header, restart_position)) == 66);
static_assert((offsetof (struct xm_header, number_of_channels)) == 68);
static_assert((offsetof (struct xm_header, number_of_patterns)) == 70);
static_assert((offsetof (struct xm_header, number_of_instruments)) == 72);
static_assert((offsetof (struct xm_header, flags)) == 74);
static_assert((offsetof (struct xm_header, default_tempo)) == 76);
static_assert((offsetof (struct xm_header, default_bpm)) == 78);
static_assert((offsetof (struct xm_header, pattern_order_table)) == 80);

typedef struct __attribute__((packed)) xm_pattern_header {
  int32_t pattern_header_length;
  int8_t packing_type;
  int16_t number_of_rows_in_pattern;
  int16_t packed_pattern_data_size;
  //int8_t packed_pattern_data[];
} xm_pattern_header_t;

static_assert((offsetof (struct xm_pattern_header, pattern_header_length)) == 0);
static_assert((offsetof (struct xm_pattern_header, packing_type)) == 4);
static_assert((offsetof (struct xm_pattern_header, number_of_rows_in_pattern)) == 5);
static_assert((offsetof (struct xm_pattern_header, packed_pattern_data_size)) == 7);
//static_assert((offsetof (struct xm_pattern_header, packed_pattern_data)) == 9);

typedef struct __attribute__((packed)) xm_instrument_header {
  int32_t instrument_size;
  int8_t instrument_name[22];
  uint8_t instrument_type;
  int16_t number_of_samples;
  int32_t sample_header_size;
  uint8_t sample_keymap_assignments[96];
  int16_t points_for_volume_envelope[24];
  int16_t points_for_panning_envelope[24];
  int8_t number_of_volume_points;
  int8_t number_of_panning_points;
  int8_t volume_sustain_point;
  int8_t volume_loop_start_point;
  int8_t volume_loop_end_point;
  int8_t panning_sustain_point;
  int8_t panning_loop_start_point;
  int8_t panning_loop_end_point;
  int8_t volume_type;
  int8_t panning_type;
  int8_t vibrato_type;
  int8_t vibrato_sweep;
  int8_t vibrato_depth;
  int8_t vibrato_rate;
  int8_t volume_fadeout;
} xm_instrument_header_t;

static_assert((offsetof (struct xm_instrument_header, instrument_size)) == 0);
static_assert((offsetof (struct xm_instrument_header, instrument_name)) == 4);
static_assert((offsetof (struct xm_instrument_header, instrument_type)) == 26);
static_assert((offsetof (struct xm_instrument_header, number_of_samples)) == 27);
static_assert((offsetof (struct xm_instrument_header, sample_header_size)) == 29);
static_assert((offsetof (struct xm_instrument_header, sample_keymap_assignments)) == 33);
static_assert((offsetof (struct xm_instrument_header, points_for_volume_envelope)) == 129);
static_assert((offsetof (struct xm_instrument_header, points_for_panning_envelope)) == 177);
static_assert((offsetof (struct xm_instrument_header, number_of_volume_points)) == 225);
static_assert((offsetof (struct xm_instrument_header, number_of_panning_points)) == 226);
static_assert((offsetof (struct xm_instrument_header, volume_sustain_point)) == 227);
static_assert((offsetof (struct xm_instrument_header, volume_loop_start_point)) == 228);
static_assert((offsetof (struct xm_instrument_header, volume_loop_end_point)) == 229);
static_assert((offsetof (struct xm_instrument_header, panning_sustain_point)) == 230);
static_assert((offsetof (struct xm_instrument_header, panning_loop_start_point)) == 231);
static_assert((offsetof (struct xm_instrument_header, panning_loop_end_point)) == 232);
static_assert((offsetof (struct xm_instrument_header, volume_type)) == 233);
static_assert((offsetof (struct xm_instrument_header, panning_type)) == 234);
static_assert((offsetof (struct xm_instrument_header, vibrato_type)) == 235);
static_assert((offsetof (struct xm_instrument_header, vibrato_sweep)) == 236);
static_assert((offsetof (struct xm_instrument_header, vibrato_depth)) == 237);
static_assert((offsetof (struct xm_instrument_header, vibrato_rate)) == 238);
static_assert((offsetof (struct xm_instrument_header, volume_fadeout)) == 239);

typedef struct __attribute__((packed)) xm_sample_header {
  int32_t sample_length;
  int32_t sample_loop_start;
  int32_t sample_loop_length;
  uint8_t volume;
  uint8_t finetune;
  uint8_t type;
  uint8_t panning;
  int8_t relative_note_number;
  uint8_t sample_data_type;
  int8_t sample_name[22];
} xm_sample_header_t;

static_assert((offsetof (struct xm_sample_header, sample_length)) == 0);
static_assert((offsetof (struct xm_sample_header, sample_loop_start)) == 4);
static_assert((offsetof (struct xm_sample_header, sample_loop_length)) == 8);
static_assert((offsetof (struct xm_sample_header, volume)) == 12);
static_assert((offsetof (struct xm_sample_header, finetune)) == 13);
static_assert((offsetof (struct xm_sample_header, type)) == 14);
static_assert((offsetof (struct xm_sample_header, panning)) == 15);
static_assert((offsetof (struct xm_sample_header, relative_note_number)) == 16);
static_assert((offsetof (struct xm_sample_header, sample_data_type)) == 17);
static_assert((offsetof (struct xm_sample_header, sample_name)) == 18);

typedef struct __attribute__((packed)) xm_pattern_format {
  uint8_t note;
  uint8_t instrument;
  uint8_t volume_column_byte;
  uint8_t effect_type;
  uint8_t effect_parameter;
} xm_pattern_format_t;

static_assert((offsetof (struct xm_pattern_format, note)) == 0);
static_assert((offsetof (struct xm_pattern_format, instrument)) == 1);
static_assert((offsetof (struct xm_pattern_format, volume_column_byte)) == 2);
static_assert((offsetof (struct xm_pattern_format, effect_type)) == 3);
static_assert((offsetof (struct xm_pattern_format, effect_parameter)) == 4);

#ifdef __cplusplus
}
#endif
