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
#include "holly/texture_memory_alloc5.hpp"
#include "holly/video_output.hpp"

#include "memorymap.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"

#include "math/float_types.hpp"

#include "assert.h"

//#include "example/arm/xm.bin.h"
#include "xm/xm.h"
#include "xm/milkypack01.xm.h"
#include "xm/middle_c.xm.h"
#include "xm/test.xm.h"
#include "xm/xmtest.xm.h"
#include "xm/catch_this_rebel.xm.h"

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

  int start_offset = 0;
  /*
  if (pf->effect_type == 0x9) { // 9 sample offset
    start_offset += (256 * pf->effect_parameter);
  }
  */

  int start = xm.sample_data_offset[pf->instrument - 1] + start_offset;

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

    tmu0_events();
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
}

void global_polygon_type_0(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        0,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

struct vertex {
  vec3 p;
  uint32_t c;
};

static inline void triangle(ta_parameter_writer& writer,
                            const vertex& a, const vertex& b, const vertex& c)
{
  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        a.p.x, a.p.y, a.p.z,
                                        a.c);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        b.p.x, b.p.y, b.p.z,
                                        b.c);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        c.p.x, c.p.y, c.p.z,
                                        c.c);
}

const vertex triangle_vertices[] = {
  { { 320.000f,   50.f, 0.1f }, 0xffff0000 },
  { { 539.393f,  430.f, 0.1f }, 0xff00ff00 },
  { { 100.607f,  430.f, 0.1f }, 0xff0000ff },
};

void transfer_scene(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer);

  triangle(writer, triangle_vertices[0], triangle_vertices[1], triangle_vertices[2]);

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
  //int buf = (int)&_binary_xm_xmtest_xm_start;
  int buf = (int)&_binary_xm_catch_this_rebel_xm_start;
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

void main()
{
  serial::init(0);

  sound_init();
  graphics_init();
  interrupt_init();

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list;

  static uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 1];
  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  while (1) {
    graphics_event(writer);
  }
}
