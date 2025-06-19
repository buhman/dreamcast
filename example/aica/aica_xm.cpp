#include "memorymap.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"

#include "assert.h"

//#include "example/arm/xm.bin.h"
#include "xm/xm.h"
#include "xm/milkypack01.xm.h"
#include "xm/middle_c.xm.h"
#include "xm/test.xm.h"

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
  int pattern_break;
  int pattern_index;
  int line_index; // within the current pattern (for debugging)
  int note_offset; // within the current pattern
  int next_note_offset;
};

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
    for (int i = 0; i < 8; i++)
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
  wait(); aica_sound.channel[ch].LSA(lsa);
  wait(); aica_sound.channel[ch].LEA(lsa + len);
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

  for (int i = 0; i < 8; i++) {
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

  state.pattern_index += 1;
  printf("pattern_index: %d\n", state.pattern_index);
  if (state.pattern_index >= 0xe)
    state.pattern_index = 1;
}

uint8_t __attribute__((aligned(32))) zero[0x28c0] = {};

void main()
{
  serial::init(0);

  int buf = (int)&_binary_xm_milkypack01_xm_start;
  //int buf = (int)&_binary_xm_middle_c_xm_start;
  //int buf = (int)&_binary_xm_test_xm_start;
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

  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0001);

  for (int i = 0; i < 64; i++) {
    wait(); aica_sound.channel[i].KYONB(0);
    wait(); aica_sound.channel[i].LPCTL(0);
    wait(); aica_sound.channel[i].PCMS(0);
    wait(); aica_sound.channel[i].LSA(0);
    wait(); aica_sound.channel[i].LEA(0);
    wait(); aica_sound.channel[i].D2R(0x1);
    wait(); aica_sound.channel[i].D1R(0x1);
    wait(); aica_sound.channel[i].RR(0xc);
    wait(); aica_sound.channel[i].AR(0x1c);

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
    | aica::mono_mem8mb_dac18b_ver_mvol::MVOL(0xf) // 15/15 volume
    ;

  // 195 = 1ms
  // 2500 / bpm milliseconds
  printf("default_bpm %d\n", xm.header->default_bpm);
  printf("default_tempo %d\n", xm.header->default_tempo);

  struct interpreter_state state;

  state.tick_rate = 195.32 * 2500 / xm.header->default_bpm;
  state.ticks_per_line = xm.header->default_tempo;
  state.tick = 0;
  state.pattern_break = -1;
  state.pattern_index = 0x1;
  state.line_index = 0;
  state.note_offset = 0;
  state.next_note_offset = 0;

  printf("tick_rate %d\n", state.tick_rate);

  printf("pattern %d\n", state.pattern_index);

  int start = sh7091.TMU.TCNT0;

  while (1) {
    xm_pattern_header_t * pattern_header = xm.pattern_header[state.pattern_index];
    int pattern_data_size = s16(&pattern_header->packed_pattern_data_size);

    int end = sh7091.TMU.TCNT0;
    while ((start - end) < (state.tick_rate / 2)) {
      end = sh7091.TMU.TCNT0;
    }
    start = sh7091.TMU.TCNT0;

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

  while (1);
}
