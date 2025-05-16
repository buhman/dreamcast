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

extern void * _binary_start __asm("_binary_example_arm_channel_bin_start");
extern void * _binary_size __asm("_binary_example_arm_channel_bin_size");

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

uint8_t __attribute__((aligned(32))) sample_data[512 * 1024];
int sample_data_ix;

int unpack_sample(int buf, int offset, xm_sample_header_t * sample_header)
{
  int size = s32(&sample_header->sample_length);
  if (sample_header->type & (1 << 4)) { // 16-bit samples
    int num_samples = size / 2;
    int old = 0;
    volatile int16_t * out = (volatile int16_t *)(&sample_data[sample_data_ix]);
    int16_t * in = (int16_t *)(buf + offset);
    for (int i = 0; i < num_samples; i++) {
      old += s16(&in[i]);
      out[i] = old;
    }
  } else { // 8-bit
    int num_samples = size;
    int old = 0;
    volatile int8_t * out = (volatile int8_t *)(&sample_data[sample_data_ix]);
    int8_t * in = (int8_t *)(buf + offset);
    for (int i = 0; i < num_samples; i++) {
      old += in[i];
      out[i] = old;
    }
  }

  if (size & 1) {
    size += 1;
  }

  return size;
}

int xm_samples_init(int buf, int offset, int instrument_ix, int number_of_samples)
{
  xm_sample_header_t * sample_header[number_of_samples];
  xm.sample_header[instrument_ix] = (xm_sample_header_t *)(buf + offset);
  for (int i = 0; i < number_of_samples; i++) {
    sample_header[i] = (xm_sample_header_t *)(buf + offset);
    offset += (sizeof (xm_sample_header_t));
  }

  for (int i = 0; i < number_of_samples; i++) {
    int sample_length = s32(&sample_header[i]->sample_length);
    if (sample_length > 0) {
      printf("instrument % 2d sample_length % 6d ix %d\n", instrument_ix, sample_length, sample_data_ix);
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
  column[note_ix & 7].note = pf->note;
  column[note_ix & 7].instrument = pf->instrument;
  column[note_ix & 7].volume_column_byte = pf->volume_column_byte;
  column[note_ix & 7].effect_type = pf->effect_type;
  column[note_ix & 7].effect_parameter = pf->effect_parameter;

  if ((note_ix & 7) == 7) {
    printf("%3d |", note_ix / 8);
    for (int i = 0; i < 8; i++)
      printf(" n:%2d i:%2d |",
             column[i].note, column[i].instrument);
    printf("\n");
  }
}

void debug_pattern(xm_pattern_header_t * pattern_header, int pattern_ix)
{
  uint8_t * pattern = (uint8_t *)(((int)pattern_header) + s32(&pattern_header->pattern_header_length));
  int ix = pattern_ix;
  int note_ix = 0;
  int end = s16(&pattern_header->packed_pattern_data_size);
  while (ix < end) {
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

void debug_pattern1(xm_pattern_header_t * pattern_header, int pattern_ix, int len, int& note_ix)
{
  uint8_t * pattern = (uint8_t *)(((int)pattern_header) + s32(&pattern_header->pattern_header_length));
  int ix = pattern_ix;
  int end = note_ix + len;
  while (note_ix < end) {
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
}

uint16_t
note_to_oct_fns(const int8_t note)
{
  static const uint16_t _cent_to_fns[] = {
    0x0,
    0x3d,
    0x7d,
    0xc2,
    0x10a,
    0x157,
    0x1a8,
    0x1fe,
    0x25a,
    0x2ba,
    0x321,
    0x38d,
  };

  const int8_t a440_note = note - 42;
  const int8_t a440_note_d = (a440_note < 0) ? a440_note - 11 : a440_note;
  const int8_t div12 = a440_note_d / static_cast<int8_t>(12);
  const int8_t mod12 = a440_note % static_cast<int8_t>(12);

  const uint16_t oct = div12;
  const uint16_t cent = (a440_note < 0) ? 12 + mod12 : mod12;
  const uint16_t fns = _cent_to_fns[cent];

  return aica::oct_fns::OCT(oct - 3) | aica::oct_fns::FNS(fns);
}

void pattern_note(int ch, xm_pattern_format_t * pf, int rekey)
{
  if (pf->note == 97) {
    if (!rekey) {
      wait(); aica_sound.channel[ch].KYONB(0);
    }
  } else if (pf->note != 0 && pf->instrument != 0) {
    if (rekey) {
      wait(); aica_sound.channel[ch].KYONB(0);
    } else {
      wait(); aica_sound.channel[ch].SA(xm.sample_data_offset[pf->instrument - 1]);
      int lsa = xm.sample_header[pf->instrument - 1]->sample_loop_start / 2;
      int lea = xm.sample_header[pf->instrument - 1]->sample_loop_length / 2;
      wait(); aica_sound.channel[ch].LSA(lsa);
      wait(); aica_sound.channel[ch].LEA(lsa + lea);
      wait(); aica_sound.channel[ch].oct_fns = note_to_oct_fns(pf->note);
      wait(); aica_sound.channel[ch].KYONB(1);
    }
  }
}

int pattern_channels(xm_pattern_header_t * pattern_header, int pattern_ix, int rekey)
{
  uint8_t * pattern = (uint8_t *)(((int)pattern_header) + s32(&pattern_header->pattern_header_length));

  for (int i = 0; i < 8; i++) {
    int p = pattern[pattern_ix];
    if (p & 0x80) {
      pattern_ix += 1;
      xm_pattern_format_t pf = {};
      if (p & (1 << 0))
        pf.note = pattern[pattern_ix++];
      if (p & (1 << 1))
        pf.instrument = pattern[pattern_ix++];
      if (p & (1 << 2))
        pf.volume_column_byte = pattern[pattern_ix++];
      if (p & (1 << 3))
        pf.effect_type = pattern[pattern_ix++];
      if (p & (1 << 4))
        pf.effect_parameter = pattern[pattern_ix++];
      pattern_note(i, &pf, rekey);
    } else {
      xm_pattern_format_t * pf = (xm_pattern_format_t *)&pattern[pattern_ix];
      pattern_note(i, pf, rekey);
      pattern_ix += 5;
    }
  }
  return pattern_ix;
}


uint8_t __attribute__((aligned(32))) zero[0x28c0] = {};

void main()
{
  serial::init(0);

  int buf = (int)&_binary_xm_milkypack01_xm_start;
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

  for (int i = 0; i < 8; i++) {
    wait(); aica_sound.channel[i].KYONB(0);
    wait(); aica_sound.channel[i].LPCTL(1);
    wait(); aica_sound.channel[i].PCMS(0);
    wait(); aica_sound.channel[i].LSA(0);
    wait(); aica_sound.channel[i].LEA(0);
    wait(); aica_sound.channel[i].D2R(0xa);
    wait(); aica_sound.channel[i].D1R(0xa);
    wait(); aica_sound.channel[i].RR(0xa);
    wait(); aica_sound.channel[i].AR(0x1f);

    wait(); aica_sound.channel[i].OCT(0);
    wait(); aica_sound.channel[i].FNS(0);
    wait(); aica_sound.channel[i].DISDL(0xf);
    wait(); aica_sound.channel[i].DIPAN(0x0);

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

  wait(); aica_sound.channel[0].SA(xm.sample_data_offset[0]);
  int lsa = xm.sample_header[0]->sample_loop_start / 2;
  int lea = xm.sample_header[0]->sample_loop_length / 2;
  printf("sa %d lsa %d lea %d\n", xm.sample_data_offset[0], lsa, lsa + lea);
  wait(); aica_sound.channel[0].LSA(lsa);
  wait(); aica_sound.channel[0].LEA(lsa + lea);
  //wait(); aica_sound.channel[0].KYONB(1);
  //wait(); aica_sound.channel[0].KYONEX(1);

  int tick = 0;

  //debug_pattern(xm.pattern_header[12], 0);

  int ix_ix = 0;
  int pattern_ix = 0;
  int note_ix = 0;

  printf("pattern %d\n", ix_ix);

  while (1) {
    xm_pattern_header_t * pattern_header = xm.pattern_header[ix_ix];
    int pattern_data_size = s16(&pattern_header->packed_pattern_data_size);

    int start = sh7091.TMU.TCNT0;
    int end = sh7091.TMU.TCNT0;
    while ((start - end) < 30000) {
      end = sh7091.TMU.TCNT0;
    }

    debug_pattern1(pattern_header, pattern_ix, 8, note_ix);
    pattern_channels(pattern_header, pattern_ix, true); // rekey
    wait(); aica_sound.channel[0].KYONEX(1);
    pattern_ix = pattern_channels(pattern_header, pattern_ix, false);
    wait(); aica_sound.channel[0].KYONEX(1);

    if (pattern_ix >= pattern_data_size) {
      note_ix = 0;
      pattern_ix = 0;
      ix_ix += 1;
      if (ix_ix >= 0xe)
        ix_ix = 0;
      printf("pattern %d\n", ix_ix);
    }

    tick += 1;
  }

  while (1);
}
