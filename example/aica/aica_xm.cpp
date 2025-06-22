#include <bit>

#include "holly/background.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/texture_memory_alloc7.hpp"
#include "holly/video_output.hpp"

#include "memorymap.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"
#include "printf/unparse.h"

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "math/float_types.hpp"

#include "assert.h"

//#include "example/arm/xm.bin.h"
#include "xm/xm.h"
#include "xm/milkypack01.xm.h"
#include "xm/middle_c.xm.h"
#include "xm/test.xm.h"
#include "xm/xmtest.xm.h"
#include "xm/catch_this_rebel.xm.h"

#include "font/tandy1k/tandy1k.data.h"

#include "interrupt.hpp"

constexpr int max_patterns = 64;
constexpr int max_instruments = 128;
struct xm_state {
  xm_header_t * header;
  xm_pattern_header_t * pattern_header[max_patterns];
  xm_instrument_header_t * instrument_header[max_instruments];
  xm_sample_header_t * sample_header[max_instruments]; // array
  int sample_data_offset[max_instruments];
};

xm_state xm = {0};

struct interpreter_state {
  int tick_rate;
  int ticks_per_line;
  int tick;
  int pattern_order_table_index;
  int pattern_break;
  int pattern_index;
  int line_index; // within the current pattern (for debugging)
  int note_offset; // within the current pattern
  int next_note_offset;
  int number_of_channels;
  int song_length;
};

struct interpreter_state state;

union aica_sandbox_channel {
  struct {
    int instrument;
    int loop;
    int note;

    int krs;
    int ar;
    int d1r;
    int d2r;
    int rr;
    int dl;

    int lfof;
    int plfows;
    int alfows;
    int plfos;
    int alfos;

    int disdl;
    int dipan;

    int kyonb;
  };
  int field[16];
};

struct aica_sandbox_state {
  int channel_ix;
  union aica_sandbox_channel channel[64];
  int pointer_row;
};

struct aica_sandbox_state sandbox_state = {};

struct key_offset_value {
  const char * label;
  const char * description;
  int min;
  int max;
};

struct key_offset_value sandbox_labels[] = {
  {"instrument", NULL, 1, max_instruments},
  {"loop", NULL, 0, 1},
  {"note", NULL, 0, 97},
  {"krs", "key rate scaling", 0, 0xf},
  {"ar", "attack rate", 0, 0x1f},
  {"d1r", "decay 1 rate", 0, 0x1f},
  {"d2r", "decay 2 rate", 0, 0x1f},
  {"rr", "release rate", 0, 0x1f},
  {"dl", "decay level", 0, 0x1f},
  {"lfof", "LFO frequency", 0, 0x1f},
  {"plfows", "pitch LFO waveform select", 0, 3},
  {"alfows", "amplitude LFO waveform select", 0, 3},
  {"plfos", "pitch LFO sensitivity", 0, 7},
  {"alfos", "amplitude LFO sensitivity", 7},
  {"disdl", "direct send level", 0xf},
  {"dipan", "direct panpot", 0x1f},
};
const int sandbox_labels_length = (sizeof (sandbox_labels)) / (sizeof (sandbox_labels[0]));

void print_u8(int8_t * chars, int length, const char * end)
{
  for (int i = 0; i < length; i++) {
    int8_t c = chars[i];
    if (c >= 0x20 && c <= 0x7e) {
      sh7091_character(c);
    } else {
      printf("\\x%02x", c);
    }
  }
  if (end != NULL) {
    while (*end != 0)
      sh7091_character(*end++);
  }
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

uint8_t __attribute__((aligned(32))) sample_data[1024 * 1024];
int sample_data_ix;

int unpack_sample(int buf, int offset, xm_sample_header_t * sample_header)
{
  int size = s32(&sample_header->sample_length);
  int loop_start = s32(&sample_header->sample_loop_start);
  int loop_length = s32(&sample_header->sample_loop_length);

  printf("unpack %d %d\n", offset, size);

  int loop_type = sample_header->type & 0b11;

  if (sample_header->type & (1 << 4)) { // 16-bit samples
    int num_samples = size / 2;
    int lsa = loop_start / 2;
    int len = loop_length / 2;

    int old = 0;
    volatile int16_t * out = (volatile int16_t *)(&sample_data[sample_data_ix]);
    int16_t * in = (int16_t *)(buf + offset);
    for (int i = 0; i < num_samples; i++) {
      old += s16(&in[i]);
      out[i] = old;
    }

    if (loop_type == 2) { // bidirectional
      for (int i = 0; i < len - 2; i++) {
        out[num_samples + i] = out[lsa + (len - i - 2)];
      }

      size += (len - 2) * 2;
    }

  } else { // 8-bit
    int num_samples = size;
    int lsa = loop_start;
    int len = loop_length;

    int old = 0;
    volatile int8_t * out = (volatile int8_t *)(&sample_data[sample_data_ix]);
    int8_t * in = (int8_t *)(buf + offset);
    for (int i = 0; i < num_samples; i++) {
      old += in[i];
      out[i] = old;
    }

    if (loop_type == 2) { // bidirectional
      for (int i = 0; i < len - 2; i++) {
        out[num_samples + i] = out[lsa + (len - i - 2)];
      }

      size += (len - 2);
    }
  }

  if (size & 1) {
    size += 1;
  }

  return size;
}

void debug_xm_sample_header(int instrument_ix, xm_sample_header_t * sample_header)
{
  printf("sample header: instrument_ix: %d:\n", instrument_ix);
  printf("  volume %d\n", sample_header->volume);
  printf("  finetune %d\n", sample_header->finetune);
  printf("  type %x\n", sample_header->type);
  printf("  panning %d\n", sample_header->panning);
  printf("  relative_note_number %d\n", sample_header->relative_note_number);
  printf("  sample_length      % 6d\n", s32(&sample_header->sample_length));
  printf("  sample_loop_start  % 6d\n", s32(&sample_header->sample_loop_start));
  printf("  sample_loop_length % 6d\n", s32(&sample_header->sample_loop_length));
}

int xm_samples_init(int buf, int offset, int instrument_ix, int number_of_samples)
{
  xm_sample_header_t * sample_header[number_of_samples];
  xm.sample_header[instrument_ix] = (xm_sample_header_t *)(buf + offset);
  if (instrument_ix <= 12)
    debug_xm_sample_header(instrument_ix, xm.sample_header[instrument_ix]);

  for (int i = 0; i < number_of_samples; i++) {
    sample_header[i] = (xm_sample_header_t *)(buf + offset);
    offset += (sizeof (xm_sample_header_t));
  }

  for (int i = 0; i < number_of_samples; i++) {
    int sample_length = s32(&sample_header[i]->sample_length);
    if (sample_length > 0) {
      //printf("  sample_length % 6d\n", sample_length);
      xm.sample_data_offset[instrument_ix] = sample_data_ix;
      sample_data_ix += unpack_sample(buf, offset, sample_header[i]);
      assert(sample_data_ix <= (int)(sizeof (sample_data)));
    }
    offset += sample_length;
  }
  return offset;
}

void xm_init(int buf)
{
  sample_data_ix = 0;

  xm.header = (xm_header_t *)(buf);

  int offset = s32(&xm.header->header_size) + (offsetof (struct xm_header, header_size));
  int number_of_patterns = s16(&xm.header->number_of_patterns);
  printf("number_of_patterns: %d\n", number_of_patterns);

  for (int i = 0; i < number_of_patterns; i++) {
    xm_pattern_header_t * pattern_header = (xm_pattern_header_t *)(buf + offset);
    xm.pattern_header[i] = pattern_header;
    offset += s32(&pattern_header->pattern_header_length) + s16(&pattern_header->packed_pattern_data_size);
  }
  printf("end_of_patterns: %d\n", offset);

  int number_of_instruments = s16(&xm.header->number_of_instruments);
  for (int i = 0; i < number_of_instruments; i++) {
    xm_instrument_header_t * instrument_header = (xm_instrument_header_t *)(buf + offset);

    xm.instrument_header[i] = instrument_header;
    offset += s32(&instrument_header->instrument_size);

    int number_of_samples = s16(&instrument_header->number_of_samples);
    offset = xm_samples_init(buf, offset, i, number_of_samples);
  }
  printf("end_of_instruments: %d\n", offset);

  int number_of_channels = s16(&xm.header->number_of_channels);
  state.number_of_channels = number_of_channels;
  printf("number_of_channels: %d\n", number_of_channels);

  int song_length = s16(&xm.header->song_length);
  state.song_length = song_length;
  printf("song_length: %d\n", song_length);

  //for (int i = 0; i < song_length; i++) {
  //printf("  %x\n", xm.header->pattern_order_table[i]);
  //}
}

void wait()
{
  uint32_t ffst = system.FFST;
  while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
	| ffst::holly_g2_if_block_internal_write_buffer(ffst)
	| ffst::aica_internal_write_buffer(ffst)) {
    ffst = system.FFST;
  };
}

constexpr uint32_t dma_address_mask = 0x1fffffe0;

void g2_aica_dma(uint32_t g2_address, uint32_t system_address, int length)
{
  using namespace dmac;

  length = (length + 31) & (~31);

  // is DMAOR needed?
  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */


  g2_if.ADEN = 0; // disable G2-AICA-DMA

  g2_if.G2APRO = 0x4659007f; // disable protection

  g2_if.ADSTAG = dma_address_mask & g2_address; // G2 address
  g2_if.ADSTAR = dma_address_mask & system_address; // system memory address
  g2_if.ADLEN = length;
  g2_if.ADDIR = 0; // from root bus to G2 device
  g2_if.ADTSEL = 0; // CPU controlled trigger
  g2_if.ADEN = 1; // enable G2-AICA-DMA
  g2_if.ADST = 1; // start G2-AICA-DMA
}

void g2_aica_dma_wait_complete()
{
  // wait for maple DMA completion
  while ((system.ISTNRM & istnrm::end_of_dma_aica_dma) == 0);
  system.ISTNRM = istnrm::end_of_dma_aica_dma;
  assert(g2_if.ADST == 0);
}

void writeback(void const * const buf, uint32_t size)
{
  uint8_t const * const buf8 = reinterpret_cast<uint8_t const * const>(buf);

  for (uint32_t i = 0; i < size / (32); i++) {
    asm volatile ("ocbwb @%0"
		  :                          // output
		  : "r" (&buf8[i * 32]) // input
                  : "memory"
		  );
  }
}

// quater-semitones
const static int cent_to_fns[] = {
    0,  15,  30,  45,  61,  77,  93, 109, 125, 142, 159, 176,
  194, 211, 229, 248, 266, 285, 304, 323, 343, 363, 383, 403,
  424, 445, 467, 488, 510, 533, 555, 578, 601, 625, 649, 673,
  698, 723, 749, 774, 801, 827, 854, 881, 909, 937, 966, 995
};
const int cent_to_fns_length = (sizeof (cent_to_fns)) / (sizeof (cent_to_fns[0]));

uint16_t
note_to_oct_fns(const int8_t note)
{
  const float base_ratio = -2.3986861877015477;

  float c4_note = (float)note - 49.0;
  float ratio = base_ratio + (c4_note / 12.0);

  float whole = (int)ratio;
  float fraction;
  if (ratio < 0) {
    if (whole > ratio)
      whole -= 1;
    fraction = -(whole - ratio);
  } else {
    fraction = ratio - whole;
  }

  assert(fraction >= 0.0);
  assert(fraction <= 1.0);

  int fns = cent_to_fns[(int)(fraction * cent_to_fns_length)];

  return aica::oct_fns::OCT((int)whole) | aica::oct_fns::FNS((int)fns);
}

int8_t volume_table[] = {
    0,  3,  5,  6,  7,  8,  8,  9,  9,  9, 10, 10, 10, 10, 11, 11,
   11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13,
   13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   15
};

void debug_note(interpreter_state& state, int ch, xm_pattern_format_t * pf)
{
  static xm_pattern_format_t column[8];
  /*
  printf("note[%d]\n", note_ix);
  printf("  note: %d\n", pf->note);
  printf("  instrument: %d\n", pf->instrument);
  printf("  volume_column_byte: %d\n", pf->volume_column_byte);
  printf("  effect_type: %d\n", pf->effect_type);
  printf("  effect_parameter: %d\n", pf->effect_parameter);
  */
  column[ch].note = pf->note;
  column[ch].instrument = pf->instrument;
  column[ch].volume_column_byte = pf->volume_column_byte;
  column[ch].effect_type = pf->effect_type;
  column[ch].effect_parameter = pf->effect_parameter;

  if (ch == 7) {
    printf("%3d %3d |", state.pattern_index, state.line_index);
    for (int i = 0; i < state.number_of_channels; i++)
      printf(" %2d %2d %2x%02x |",
             column[i].note,
             column[i].instrument,
             column[i].effect_type,
             column[i].effect_parameter);
    printf("\n");
  }
}

void _play_note(int ch, xm_pattern_format_t * pf)
{
  xm_sample_header_t * sample_header = xm.sample_header[pf->instrument - 1];

  int sample_type = ((sample_header->type & (1 << 4)) != 0);
  int bytes_per_sample = 1 + sample_type;

  int start = xm.sample_data_offset[pf->instrument - 1];

  int loop_type = sample_header->type & 0b11;
  int lpctl = (loop_type == 0) ? 0 : 1;
  int lsa = s32(&sample_header->sample_loop_start) / bytes_per_sample;
  int len = s32(&sample_header->sample_loop_length) / bytes_per_sample;
  if (len == 0) {
    len = s32(&sample_header->sample_length) / bytes_per_sample;
  }
  if (len >= 65535) {
    len = 65532;
  }
  assert(start >= 0);
  assert(lsa >= 0);
  assert(len >= 0);

  if (loop_type == 2) // bidirectional
    len += len - 2;

  assert(sample_header->volume >= 0 && sample_header->volume <= 64);
  int disdl = volume_table[sample_header->volume];
  bool pcms = !sample_type;

  if (pf->effect_type == 0x04) { // vibrato
    wait(); aica_sound.channel[ch].LFOF(0x12);
    wait(); aica_sound.channel[ch].ALFOWS(2);
    wait(); aica_sound.channel[ch].PLFOWS(2);
    wait(); aica_sound.channel[ch].ALFOS(0);
    wait(); aica_sound.channel[ch].PLFOS(4);
  } else {
    //wait(); aica_sound.channel[ch].LFOF(0x11);
    //wait(); aica_sound.channel[ch].ALFOWS(2);
    //wait(); aica_sound.channel[ch].PLFOWS(2);
    wait(); aica_sound.channel[ch].ALFOS(0);
    wait(); aica_sound.channel[ch].PLFOS(0);
  }

  wait(); aica_sound.channel[ch].PCMS(pcms);
  wait(); aica_sound.channel[ch].SA(start);
  wait(); aica_sound.channel[ch].LPCTL(lpctl);
  wait(); aica_sound.channel[ch].LSA((lsa) & ~(0b11));
  wait(); aica_sound.channel[ch].LEA((lsa + len) & ~(0b11));
  wait(); aica_sound.channel[ch].oct_fns = note_to_oct_fns(pf->note + sample_header->relative_note_number);
  wait(); aica_sound.channel[ch].DISDL(disdl);
  wait(); aica_sound.channel[ch].KYONB(1);
}

void play_note_effect(interpreter_state& state, int ch, xm_pattern_format_t * pf)
{
  int effect_tick = (state.tick / 2) % state.ticks_per_line;

  switch (pf->effect_type) {
  case 0x04: // 4 vibrato
    wait(); aica_sound.channel[ch].LFOF(0x12);
    wait(); aica_sound.channel[ch].ALFOWS(2);
    wait(); aica_sound.channel[ch].PLFOWS(2);
    wait(); aica_sound.channel[ch].ALFOS(0);
    wait(); aica_sound.channel[ch].PLFOS(4);
    break;
  case 0x0d: // D pattern break
    state.pattern_break = pf->effect_parameter;
    break;
  case 0x0e: // E
    switch (pf->effect_parameter & 0xf0) {
    case 0xd0: // ED note delay
      if (effect_tick == (pf->effect_parameter & 0x0f)) {
        _play_note(ch, pf);
      }
      break;
    }
    break;
  case 0x14: // K delayed tick
    if (effect_tick == pf->effect_parameter) {
      wait(); aica_sound.channel[ch].KYONB(0);
    }
    break;
  }
}

void play_note(interpreter_state& state, int ch, xm_pattern_format_t * pf)
{
  if (pf->note == 97) {
    wait(); aica_sound.channel[ch].KYONB(0);
  } else if (pf->note != 0 && pf->instrument != 0) {
    bool note_delay = (pf->effect_type == 0xe) && ((pf->effect_parameter & 0xf0) == 0xd0); // ED note delay
    if (!note_delay)
      _play_note(ch, pf);
  }

  play_note_effect(state, ch, pf);
}

void play_debug_note(interpreter_state& state, int ch, xm_pattern_format_t * pf)
{
  debug_note(state, ch, pf);
  play_note(state, ch, pf);
}

void rekey_note(interpreter_state& state, int ch, xm_pattern_format_t * pf)
{
  if (pf->note == 97) {
  } else if (pf->note != 0 && pf->instrument != 0) {
    wait(); aica_sound.channel[ch].KYONB(0);
  }
}

int parse_pattern_line(interpreter_state& state, xm_pattern_header_t * pattern_header, int note_offset, void (*func)(interpreter_state&, int, xm_pattern_format_t*))
{
  uint8_t * pattern = (uint8_t *)(((int)pattern_header) + s32(&pattern_header->pattern_header_length));

  for (int i = 0; i < state.number_of_channels; i++) {
    int p = pattern[note_offset];
    if (p & 0x80) {
      note_offset += 1;
      xm_pattern_format_t pf = {};
      if (p & (1 << 0))
        pf.note = pattern[note_offset++];
      if (p & (1 << 1))
        pf.instrument = pattern[note_offset++];
      if (p & (1 << 2))
        pf.volume_column_byte = pattern[note_offset++];
      if (p & (1 << 3))
        pf.effect_type = pattern[note_offset++];
      if (p & (1 << 4))
        pf.effect_parameter = pattern[note_offset++];
      func(state, i, &pf);
    } else {
      xm_pattern_format_t * pf = (xm_pattern_format_t *)&pattern[note_offset];
      func(state, i, pf);
      note_offset += 5;
    }
  }
  return note_offset;
}

void next_pattern(interpreter_state& state, int pattern_break)
{
  state.line_index = 0;
  state.next_note_offset = 0;
  state.pattern_break = -1;

  state.pattern_order_table_index += 1;
  printf("pattern_order_table_index: %d\n", state.pattern_order_table_index);
  if (state.pattern_order_table_index >= state.song_length)
    state.pattern_order_table_index = 0;
  state.pattern_index = xm.header->pattern_order_table[state.pattern_order_table_index];
}

void vbr100()
{
  serial::string("vbr100\n");
  interrupt_exception();
}

void vbr400()
{
  serial::string("vbr400\n");
  interrupt_exception();
}

constexpr int div(int n, int d)
{
  return (n + 32 - 1) / 32;
}

struct framebuffer {
  int px_width;
  int px_height;

  framebuffer(int width, int height)
    : px_width(width), px_height(height)
  {}

  int tile_width() {
    return div(px_width, 32);
  }
  int tile_height() {
    return div(px_height, 32);
  }
};
struct framebuffer framebuffer(640, 480);
const int bytes_per_pixel = 2;

constexpr uint32_t ta_alloc = 0
                            | ta_alloc_ctrl::pt_opb::no_list
                            | ta_alloc_ctrl::tm_opb::no_list
                            | ta_alloc_ctrl::t_opb::no_list
                            | ta_alloc_ctrl::om_opb::no_list
                            | ta_alloc_ctrl::o_opb::_32x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
    .translucent = 0,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};

static volatile int ta_in_use = 0;
static volatile int core_in_use = 0;
static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static inline void pump_events(uint32_t istnrm)
{
  if (istnrm & istnrm::v_blank_in) {
    system.ISTNRM = istnrm::v_blank_in;

    next_frame = 1;
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[next_frame_ix].start;
  }

  if (istnrm & istnrm::end_of_render_tsp) {
    system.ISTNRM = istnrm::end_of_render_tsp
                  | istnrm::end_of_render_isp
                  | istnrm::end_of_render_video;

    next_frame_ix = framebuffer_ix;
    framebuffer_ix += 1;
    if (framebuffer_ix >= 3) framebuffer_ix = 0;

    core_in_use = 0;
  }

  if (istnrm & istnrm::end_of_transferring_opaque_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

    core_in_use = 1;
    core_start_render2(texture_memory_alloc.region_array.start,
                       texture_memory_alloc.isp_tsp_parameters.start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[framebuffer_ix].start,
                       framebuffer.px_width);

    ta_in_use = 0;
  }
}

static inline void tmu0_events()
{
  xm_pattern_header_t * pattern_header = xm.pattern_header[state.pattern_index];
  int pattern_data_size = s16(&pattern_header->packed_pattern_data_size);

  bool keyoff_tick = (state.tick + 1) % (state.ticks_per_line * 2) == 0;
  bool note_tick = state.tick % (state.ticks_per_line * 2) == 0;
  bool effect_tick = (state.tick & 1) == 0;
  bool pattern_break_tick = (state.tick % (state.ticks_per_line * 2)) == (state.ticks_per_line * 2 - 1);
  if (keyoff_tick) {
    // execute keyoffs
    parse_pattern_line(state, pattern_header, state.next_note_offset, rekey_note);
    wait(); aica_sound.channel[0].KYONEX(1);
  }

  if (state.pattern_break >= 0 && pattern_break_tick) {
    printf("pattern_break\n");
    next_pattern(state, -1);
  }

  if (note_tick) {
    state.note_offset = state.next_note_offset;
    state.next_note_offset = parse_pattern_line(state, pattern_header, state.note_offset, play_debug_note);
    //state.next_note_offset = parse_pattern_line(state, pattern_header, state.note_offset, play_note);
    state.line_index += 1;
    wait(); aica_sound.channel[0].KYONEX(1);
  }
  if (effect_tick && !note_tick) {
    // execute effects
    parse_pattern_line(state, pattern_header, state.note_offset, play_note_effect);
    wait(); aica_sound.channel[0].KYONEX(1);
  }

  if (state.next_note_offset >= pattern_data_size && pattern_break_tick) {
    printf("pattern_data_size\n");
    next_pattern(state, -1);
  }

  state.tick += 1;
}

void vbr600()
{
  uint32_t sr;
  asm volatile ("stc sr,%0" : "=r" (sr));
  sr |= sh::sr::imask(15);
  asm volatile ("ldc %0,sr" : : "r" (sr));

  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) { // Holly
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(system.ISTERR);
    }

    pump_events(istnrm);
  } else if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x400) { // TMU0
    sh7091.TMU.TCR0
      = tmu::tcr0::UNIE
      | tmu::tcr0::tpsc::p_phi_256; // clear underflow

    //tmu0_events();
  } else {
    serial::string("vbr600\n");
    interrupt_exception();
  }

  sr &= ~sh::sr::imask(15);
  asm volatile ("ldc %0,sr" : : "r" (sr));
}

void framebuffer_init()
{
  int x_size = framebuffer.px_width;
  int y_size = framebuffer.px_height;

  // write

  holly.FB_X_CLIP = fb_x_clip::fb_x_clip_max(x_size - 1)
                  | fb_x_clip::fb_x_clip_min(0);

  holly.FB_Y_CLIP = fb_y_clip::fb_y_clip_max(y_size - 1)
                  | fb_y_clip::fb_y_clip_min(0);

  // read

  holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                  | fb_r_size::fb_y_size(y_size - 1)
                  | fb_r_size::fb_x_size((x_size * bytes_per_pixel) / 4 - 1);

  holly.FB_R_CTRL = fb_r_ctrl::vclk_div::pclk_vclk_1
                  | fb_r_ctrl::fb_depth::_565_rgb_16bit
                  | fb_r_ctrl::fb_enable;
}

void scaler_init()
{
  holly.Y_COEFF = y_coeff::coefficient_1(0x80)
                | y_coeff::coefficient_0_2(0x40);

  // in 6.10 fixed point; 0x0400 is 1x vertical scale
  holly.SCALER_CTL = scaler_ctl::vertical_scale_factor(0x0400);

  holly.FB_BURSTCTRL = fb_burstctrl::wr_burst(0x09)
                     | fb_burstctrl::vid_lat(0x3f)
                     | fb_burstctrl::vid_burst(0x39);
}

void spg_set_mode_720x480()
{
  holly.SPG_CONTROL
    = spg_control::sync_direction::output;

  holly.SPG_LOAD
    = spg_load::vcount(525 - 1)    // number of lines per field
    | spg_load::hcount(858 - 1);   // number of video clock cycles per line

  holly.SPG_HBLANK
    = spg_hblank::hbend(117)       // H Blank ending position
    | spg_hblank::hbstart(837);    // H Blank starting position

  holly.SPG_VBLANK
    = spg_vblank::vbend(40)        // V Blank ending position
    | spg_vblank::vbstart(520);    // V Blank starting position

  holly.SPG_WIDTH
    = spg_width::eqwidth(16 - 1)   // Specify the equivalent pulse width (number of video clock cycles - 1)
    | spg_width::bpwidth(794 - 1)  // Specify the broad pulse width (number of video clock cycles - 1)
    | spg_width::vswidth(3)        // V Sync width (number of lines)
    | spg_width::hswidth(64 - 1);  // H Sync width (number of video clock cycles - 1)

  holly.VO_STARTX
    = vo_startx::horizontal_start_position(117);

  holly.VO_STARTY
    = vo_starty::vertical_start_position_on_field_2(40)
    | vo_starty::vertical_start_position_on_field_1(40);

  holly.VO_CONTROL
    = vo_control::pclk_delay(22);

  holly.SPG_HBLANK_INT
    = spg_hblank_int::line_comp_val(837);

  holly.SPG_VBLANK_INT
    = spg_vblank_int::vblank_out_interrupt_line_number(21)
    | spg_vblank_int::vblank_in_interrupt_line_number(520);
}

void spg_set_mode_640x480()
{
  holly.SPG_CONTROL
    = spg_control::sync_direction::output;

  holly.SPG_LOAD
    = spg_load::vcount(525 - 1)    // number of lines per field
    | spg_load::hcount(858 - 1);   // number of video clock cycles per line

  holly.SPG_HBLANK
    = spg_hblank::hbend(126)       // H Blank ending position
    | spg_hblank::hbstart(837);    // H Blank starting position

  holly.SPG_VBLANK
    = spg_vblank::vbend(40)        // V Blank ending position
    | spg_vblank::vbstart(520);    // V Blank starting position

  holly.SPG_WIDTH
    = spg_width::eqwidth(16 - 1)   // Specify the equivalent pulse width (number of video clock cycles - 1)
    | spg_width::bpwidth(794 - 1)  // Specify the broad pulse width (number of video clock cycles - 1)
    | spg_width::vswidth(3)        // V Sync width (number of lines)
    | spg_width::hswidth(64 - 1);  // H Sync width (number of video clock cycles - 1)

  holly.VO_STARTX
    = vo_startx::horizontal_start_position(168);

  holly.VO_STARTY
    = vo_starty::vertical_start_position_on_field_2(40)
    | vo_starty::vertical_start_position_on_field_1(40);

  holly.VO_CONTROL
    = vo_control::pclk_delay(22);

  holly.SPG_HBLANK_INT
    = spg_hblank_int::line_comp_val(837);

  holly.SPG_VBLANK_INT
    = spg_vblank_int::vblank_out_interrupt_line_number(21)
    | spg_vblank_int::vblank_in_interrupt_line_number(520);
}

void core_param_init()
{
  uint32_t region_array_start = texture_memory_alloc.region_array.start;
  uint32_t isp_tsp_parameters_start = texture_memory_alloc.isp_tsp_parameters.start;
  uint32_t background_start = texture_memory_alloc.framebuffer[0].start;

  holly.REGION_BASE = region_array_start;
  holly.PARAM_BASE = isp_tsp_parameters_start;

  uint32_t background_offset = background_start - isp_tsp_parameters_start;

  holly.ISP_BACKGND_T
    = isp_backgnd_t::tag_address(background_offset / 4)
    | isp_backgnd_t::tag_offset(0)
    | isp_backgnd_t::skip(1);
  holly.ISP_BACKGND_D = _i(1.f/100000.f);

  holly.FB_W_CTRL
    = fb_w_ctrl::fb_dither
    | fb_w_ctrl::fb_packmode::_565_rgb_16bit;

  holly.FB_W_LINESTRIDE = (framebuffer.px_width * bytes_per_pixel) / 8;
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffe0) / 4];
  uint32_t * src32 = reinterpret_cast<uint32_t *>(src);

  length = (length + 31) & ~31; // round up to nearest multiple of 32
  while (length > 0) {
    base[0] = src32[0];
    base[1] = src32[1];
    base[2] = src32[2];
    base[3] = src32[3];
    base[4] = src32[4];
    base[5] = src32[5];
    base[6] = src32[6];
    base[7] = src32[7];
    asm volatile ("pref @%0"
                  :                // output
                  : "r" (&base[0]) // input
                  : "memory");
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  uint32_t offset = texture_memory_alloc.texture.start + 0;
  void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
  void * src = reinterpret_cast<void *>(&_binary_font_tandy1k_tandy1k_data_start);
  int size = reinterpret_cast<int>(&_binary_font_tandy1k_tandy1k_data_size);
  transfer_ta_fifo_texture_memory_32byte(dst, src, size);
}

void transfer_palettes()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;

  holly.PALETTE_RAM[0] = 0;
  holly.PALETTE_RAM[1] = 0x7fff;
}

void graphics_init()
{
  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  scaler_init();
  core_init();
  core_param_init();
  spg_set_mode_640x480();
  framebuffer_init();

  background_parameter2(texture_memory_alloc.framebuffer[0].start,
                        0xff800080);

  region_array_multipass(framebuffer.tile_width(),
                         framebuffer.tile_height(),
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  transfer_textures();
  transfer_palettes();
}

void global_polygon_type_0(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
    | obj_control::texture
    ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(128)
                                      | tsp_instruction_word::texture_v_size::from_int(256);

  const uint32_t texture_address = texture_memory_alloc.texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

struct vertex {
  vec3 p;
  vec2 t;
};

static inline void quad(ta_parameter_writer& writer,
                        const vec3& ap, const vec2& at,
                        const vec3& bp, const vec2& bt,
                        const vec3& cp, const vec2& ct,
                        const vec3& dp, const vec2& dt)
{
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        0, 0);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        0, 0);
}

const vertex quad_vertices[] = {
  { { 0, 0, 0.1f }, {0, 0} },
  { { 1, 0, 0.1f }, {1, 0} },
  { { 1, 1, 0.1f }, {1, 1} },
  { { 0, 1, 0.1f }, {0, 1} },
};

const int texture_width = 128;
const int texture_height = 256;
const int glyph_width = 9;
const int glyph_height = 12;
const int glyphs_per_row = texture_width / glyph_width;
const int glyph_hori_advance = 8;
const int glyph_vert_advance = 9;

static inline vec2 transform_glyph_texture(const vec2& t, int char_code)
{
  int row = char_code / glyphs_per_row;
  int col = char_code % glyphs_per_row;

  return {
    (float)(col * glyph_width  + t.x * glyph_width) / (float)(texture_width),
    (float)(row * glyph_height + t.y * glyph_height) / (float)(texture_height),
  };
}

static inline vec3 transform_glyph_position(const vec3& p, float x, float y)
{
  return {
    p.x * glyph_width + x,
    p.y * glyph_height + y,
    p.z
  };
}

void transfer_glyph(ta_parameter_writer& writer, char c, int x, int y)
{
  vec3 ap = transform_glyph_position(quad_vertices[0].p, x, y);
  vec3 bp = transform_glyph_position(quad_vertices[1].p, x, y);
  vec3 cp = transform_glyph_position(quad_vertices[2].p, x, y);
  vec3 dp = transform_glyph_position(quad_vertices[3].p, x, y);

  vec2 at = transform_glyph_texture(quad_vertices[0].t, c);
  vec2 bt = transform_glyph_texture(quad_vertices[1].t, c);
  vec2 ct = transform_glyph_texture(quad_vertices[2].t, c);
  vec2 dt = transform_glyph_texture(quad_vertices[3].t, c);

  quad(writer,
       ap, at,
       bp, bt,
       cp, ct,
       dp, dt);
}

int transfer_string(ta_parameter_writer& writer, const char * s, int x, int y)
{
  int len = 0;
  while (*s) {
    len += 1;
    transfer_glyph(writer, *s++, x, y);
    x += glyph_hori_advance;
  }
  return len;
}

int transfer_integer(ta_parameter_writer& writer, int n, int x, int y, int offset)
{
  char buf[16];

  int len = unparse_base10(buf, n, 3, ' ');
  buf[len] = 0;

  int shift = 0;
  if (offset >= 0) {
    shift = 10 - offset;
    if (shift < 0)
      shift = 0;
  }
  x += glyph_hori_advance * shift;

  transfer_string(writer, buf, x, y);

  return len + shift;
}

void transfer_scene(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer);

  int x = 32;
  int y = 32;

  int xi = x;
  int len = transfer_string(writer, "channel", xi, y);
  xi += glyph_hori_advance * len;
  xi += glyph_hori_advance;
  len = transfer_integer(writer, sandbox_state.channel_ix, xi, y, -1);
  xi += glyph_hori_advance * len;
  xi += glyph_hori_advance * 2;

  if (sandbox_state.channel[sandbox_state.channel_ix].kyonb)
    transfer_string(writer, "key on", xi, y);
  else
    transfer_string(writer, "key off", xi, y);

  y += glyph_vert_advance * 2;

  for (int i = 0; i < sandbox_labels_length; i++) {
    int xi = x + glyph_hori_advance * 2;

    int len = transfer_string(writer, sandbox_labels[i].label, xi, y);
    xi += glyph_hori_advance * len;
    xi += glyph_hori_advance;

    len = transfer_integer(writer, sandbox_state.channel[sandbox_state.channel_ix].field[i], xi, y, len);
    xi += glyph_hori_advance * len;
    xi += glyph_hori_advance * 3;

    if (sandbox_labels[i].description)
      transfer_string(writer, sandbox_labels[i].description, xi, y);

    if (i == sandbox_state.pointer_row) {
      transfer_glyph(writer, 16, x, y); // ►
    }

    y += glyph_vert_advance;
  }

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void graphics_event(ta_parameter_writer& writer)
{
  writer.offset = 0;

  transfer_scene(writer);

  while (ta_in_use);
  while (core_in_use);
  ta_in_use = 1;
  ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters.start,
                             texture_memory_alloc.isp_tsp_parameters.end,
                             texture_memory_alloc.object_list.start,
                             texture_memory_alloc.object_list.end,
                             opb_size[0].total(),
                             ta_alloc,
                             framebuffer.tile_width(),
                             framebuffer.tile_height());
  ta_polygon_converter_writeback(writer.buf, writer.offset);
  ta_polygon_converter_transfer(writer.buf, writer.offset);

  while (next_frame == 0);
  next_frame = 0;
}

uint8_t __attribute__((aligned(32))) zero[0x28c0] = {};

void sound_init()
{
  //int buf = (int)&_binary_xm_milkypack01_xm_start;
  //int buf = (int)&_binary_xm_middle_c_xm_start;
  //int buf = (int)&_binary_xm_test_xm_start;
  int buf = (int)&_binary_xm_xmtest_xm_start;
  //int buf = (int)&_binary_xm_catch_this_rebel_xm_start;
  xm_init(buf);

  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0111);
  system.ISTNRM = istnrm::end_of_dma_aica_dma;

  // slot/common: 00700000 - 007028c0 (excludes vreg_armrst)
  g2_aica_dma((uint32_t)0x00700000, (int)zero, 0x28c0);
  g2_aica_dma_wait_complete();

  // dsp        : 00703000 - 007045c8
  g2_aica_dma((uint32_t)0x00703000, (int)zero, 0x15e0);
  g2_aica_dma_wait_complete();

  printf("i[0] start %d size %d\n",
         xm.sample_data_offset[0],
         s32(&xm.sample_header[0]->sample_length));

  printf("i[1] start %d size %d\n",
         xm.sample_data_offset[1],
         s32(&xm.sample_header[1]->sample_length));

  for (int i = 0; i < 16; i++) {
    serial::hexlify(&sample_data[i * 16], 16);
  }

  printf("transfer %08x %08x %d\n", (int)aica_wave_memory, (int)sample_data, sample_data_ix);
  // wave memory

  int size = (sample_data_ix + 31) & (~31);
  writeback(sample_data, size);
  system.ISTERR = 0xffffffff;
  g2_aica_dma((int)aica_wave_memory, (int)sample_data, size);
  g2_aica_dma_wait_complete();
  printf("sar0 %08x\n", sh7091.DMAC.SAR0);
  printf("dar0 %08x\n", sh7091.DMAC.DAR0);
  printf("dmatcr0 %08x\n", sh7091.DMAC.DMATCR0);
  printf("chcr0 %08x\n", sh7091.DMAC.CHCR0);
  printf("isterr %08x\n", system.ISTERR);
  //g2_aica_dma((int)aica_wave_memory, (int)sample_data, size);
  //g2_aica_dma_wait_complete();

  for (int i = 0; i < 16; i++) {
    volatile uint8_t * s = &((volatile uint8_t*)aica_wave_memory)[i * 16];
    for (int j = 0; j < 16; j++) {
      wait();
      serial::hexlify(s[j]);
      serial::character(' ');
    }
    serial::character('\n');
  }

  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0001);

  for (int i = 0; i < 64; i++) {
    wait(); aica_sound.channel[i].KYONB(0);
    wait(); aica_sound.channel[i].LPCTL(0);
    wait(); aica_sound.channel[i].PCMS(0);
    wait(); aica_sound.channel[i].LSA(0);
    wait(); aica_sound.channel[i].LEA(0);
    wait(); aica_sound.channel[i].D2R(0);
    wait(); aica_sound.channel[i].D1R(0);
    wait(); aica_sound.channel[i].RR(0x1f);
    wait(); aica_sound.channel[i].AR(0x1f);

    wait(); aica_sound.channel[i].ALFOS(0);
    wait(); aica_sound.channel[i].PLFOS(0);

    wait(); aica_sound.channel[i].OCT(0);
    wait(); aica_sound.channel[i].FNS(0);
    wait(); aica_sound.channel[i].DISDL(0);
    wait(); aica_sound.channel[i].DIPAN(0);

    wait(); aica_sound.channel[i].Q(0b00100);
    wait(); aica_sound.channel[i].TL(0);
    wait(); aica_sound.channel[i].LPOFF(1);
  }

  wait(); aica_sound.common.mono_mem8mb_dac18b_ver_mvol =
      aica::mono_mem8mb_dac18b_ver_mvol::MONO(0)   // enable panpots
    | aica::mono_mem8mb_dac18b_ver_mvol::MEM8MB(0) // 16Mbit SDRAM
    | aica::mono_mem8mb_dac18b_ver_mvol::DAC18B(0) // 16-bit DAC
    | aica::mono_mem8mb_dac18b_ver_mvol::MVOL(0xc) // volume
    ;

  // 195 = 1ms
  // 2500 / bpm milliseconds
  printf("default_bpm %d\n", xm.header->default_bpm);
  printf("default_tempo %d\n", xm.header->default_tempo);

  state.tick_rate = 195.32 * 2500 / xm.header->default_bpm;
  state.ticks_per_line = xm.header->default_tempo;
  state.tick = 0;
  state.pattern_break = -1;
  state.pattern_order_table_index = 0;
  state.pattern_index = xm.header->pattern_order_table[state.pattern_order_table_index];
  state.line_index = 0;
  state.note_offset = 0;
  state.next_note_offset = 0;

  printf("tick_rate %d\n", state.tick_rate);

  printf("pattern %d\n", state.pattern_index);

  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TCOR0 = state.tick_rate / 2;
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0
    = tmu::tcr0::UNIE
    | tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCNT0 = 0;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  sh7091.INTC.IPRA = intc::ipra::TMU0(1);
}

static ft0::data_transfer::data_format data[4];

uint8_t send_buf[1024] __attribute__((aligned(32)));
uint8_t recv_buf[1024] __attribute__((aligned(32)));

void do_get_condition()
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  {
    using command_type = maple::device_request;
    using response_type = maple::device_status;

    writer.append_command_all_ports<command_type, response_type>();
  }

  for (int port = 0; port < 4; port++) {
    auto& data_fields = host_command[port].bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::controller);
  }
  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((std::byteswap(data_fields.function_type) & function_type::controller) == 0) {
      return;
    }

    data[port].digital_button = data_fields.data.digital_button;
    for (int i = 0; i < 6; i++) {
      data[port].analog_coordinate_axis[i]
        = data_fields.data.analog_coordinate_axis[i];
    }
  }
}

void execute_note(int ch, const aica_sandbox_channel& channel)
{
  xm_sample_header_t * sample_header = xm.sample_header[channel.instrument - 1];
  int sample_type = ((sample_header->type & (1 << 4)) != 0);
  int bytes_per_sample = 1 + sample_type;

  int start = xm.sample_data_offset[channel.instrument - 1];

  int loop_type = sample_header->type & 0b11;

  int lsa = s32(&sample_header->sample_loop_start) / bytes_per_sample;
  int len = s32(&sample_header->sample_loop_length) / bytes_per_sample;

  if (len == 0) {
    len = s32(&sample_header->sample_length) / bytes_per_sample;
  }
  if (len >= 65535) {
    len = 65532;
  }
  assert(start >= 0);
  assert(lsa >= 0);
  assert(len >= 0);

  if (channel.loop && loop_type == 2) // bidirectional
    len += len - 2;

  assert(sample_header->volume >= 0 && sample_header->volume <= 64);
  bool pcms = !sample_type;

  wait(); aica_sound.channel[ch].oct_fns = note_to_oct_fns(channel.note + sample_header->relative_note_number);
  wait(); aica_sound.channel[ch].PCMS(pcms);
  wait(); aica_sound.channel[ch].SA(start);
  wait(); aica_sound.channel[ch].LSA((lsa) & ~(0b11));
  wait(); aica_sound.channel[ch].LEA((lsa + len) & ~(0b11));
  wait(); aica_sound.channel[ch].LPCTL(channel.loop);

  wait(); aica_sound.channel[ch].KRS(channel.krs);
  wait(); aica_sound.channel[ch].AR(channel.ar);
  wait(); aica_sound.channel[ch].D1R(channel.d1r);;
  wait(); aica_sound.channel[ch].D2R(channel.d2r);
  wait(); aica_sound.channel[ch].RR(channel.rr);
  wait(); aica_sound.channel[ch].DL(channel.dl);

  wait(); aica_sound.channel[ch].LFOF(channel.lfof);
  wait(); aica_sound.channel[ch].PLFOWS(channel.plfows);
  wait(); aica_sound.channel[ch].ALFOWS(channel.alfows);
  wait(); aica_sound.channel[ch].PLFOS(channel.plfos);
  wait(); aica_sound.channel[ch].ALFOS(channel.alfos);

  wait(); aica_sound.channel[ch].DISDL(channel.disdl);
  wait(); aica_sound.channel[ch].DIPAN(channel.dipan);
}

void label_update(int delta)
{
  int ix = sandbox_state.pointer_row;

  const key_offset_value& label = sandbox_labels[ix];

  union aica_sandbox_channel& channel = sandbox_state.channel[sandbox_state.channel_ix];

  channel.field[ix] += delta;
  if (channel.field[ix] > label.max)
    channel.field[ix] = label.max;
  if (channel.field[ix] < label.min)
    channel.field[ix] = label.min;

  execute_note(sandbox_state.channel_ix, channel);
}

void execute_key(bool on)
{
  aica_sandbox_channel& channel = sandbox_state.channel[sandbox_state.channel_ix];

  int ch = sandbox_state.channel_ix;

  execute_note(ch, channel);

  channel.kyonb = on;
  wait(); aica_sound.channel[ch].KYONB(on);
  wait(); aica_sound.channel[ch].KYONEX(1);
}

void input_update()
{
  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;
  int da = ft0::data_transfer::digital_button::da(data[0].digital_button) == 0;
  int ua = ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0;

  int x = ft0::data_transfer::digital_button::x(data[0].digital_button) == 0;
  int y = ft0::data_transfer::digital_button::y(data[0].digital_button) == 0;
  int a = ft0::data_transfer::digital_button::a(data[0].digital_button) == 0;
  int b = ft0::data_transfer::digital_button::b(data[0].digital_button) == 0;

  static int last_ra = 0;
  static int last_la = 0;
  static int last_da = 0;
  static int last_ua = 0;

  static int last_x = 0;
  static int last_y = 0;
  static int last_a = 0;
  static int last_b = 0;

  if (ra && ra != last_ra) {
    sandbox_state.channel_ix += 1;
    if (sandbox_state.channel_ix > 63)
      sandbox_state.channel_ix = 0;
  }
  if (la && la != last_la) {
    sandbox_state.channel_ix -= 1;
    if (sandbox_state.channel_ix < 0)
      sandbox_state.channel_ix = 63;
  }
  if (da && da != last_da) {
    sandbox_state.pointer_row += 1;
    if (sandbox_state.pointer_row >= sandbox_labels_length)
      sandbox_state.pointer_row = 0;
  }
  if (ua && ua != last_ua) {
    sandbox_state.pointer_row -= 1;
    if (sandbox_state.pointer_row < 0)
      sandbox_state.pointer_row = sandbox_labels_length - 1;
  }

  if (x && x != last_x) {
    label_update(-1);
  }
  if (y && y != last_y) {
    execute_key(0);
  }
  if (a && a != last_a) {
    execute_key(1);
  }
  if (b && b != last_b) {
    label_update(1);
  }

  last_ra = ra;
  last_la = la;
  last_ua = ua;
  last_da = da;

  last_x = x;
  last_y = y;
  last_a = a;
  last_b = b;
}

void channel_sandbox_defaults()
{
  for (int ch = 0; ch < 64; ch++) {
    aica_sandbox_channel& channel = sandbox_state.channel[ch];

    channel.instrument = 1;

    channel.note = 49;

    channel.ar = 0x1f;
    channel.rr = 0x1f;

    channel.disdl = 0xf;
  }
}

void main()
{
  serial::init(0);

  sound_init();
  graphics_init();
  interrupt_init();
  channel_sandbox_defaults();

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list;

  static uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 1];
  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    input_update();

    graphics_event(writer);
  }
}
