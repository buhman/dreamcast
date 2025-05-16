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


  g2_if.G2APRO = 0x4659007f; // disable protection

  g2_if.ADSTAG = dma_address_mask & g2_address; // G2 address
  g2_if.ADSTAR = dma_address_mask & system_address; // system memory address
  g2_if.ADLEN = length;
  g2_if.ADDIR = 0; // from root bus to G2 device
  g2_if.ADTSEL = 0; // CPU controlled trigger
  g2_if.ADEN = 1; // enable G2-DMA
  g2_if.ADST = 1; // start G2-DMA
}

void g2_aica_dma_wait_complete()
{
  // wait for maple DMA completion
  while ((system.ISTNRM & istnrm::end_of_dma_aica_dma) == 0);
  system.ISTNRM = istnrm::end_of_dma_aica_dma;
  assert(g2_if.ADST == 0);
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

  for (int i = 0; i < 16; i++) {
    serial::hexlify(&sample_data[i * 16], 16);
  }

  printf("transfer %08x %08x %d\n", (int)aica_wave_memory, (int)sample_data, sample_data_ix);
  // wave memory
  g2_aica_dma((int)aica_wave_memory, (int)sample_data, (sample_data_ix + 31) & (~31));
  g2_aica_dma_wait_complete();
  g2_aica_dma((int)aica_wave_memory, (int)sample_data, (sample_data_ix + 31) & (~31));
  g2_aica_dma_wait_complete();

  /*
  for (int i = 0; i < 16; i++) {
    volatile uint8_t * s = &((volatile uint8_t*)aica_wave_memory)[i * 16];
    for (int j = 0; j < 16; j++) {
      wait();
      serial::hexlify(s[j]);
      serial::character(' ');
    }
    serial::character('\n');
  }
  */

  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0001);

  /*
  wait(); aica_sound.channel[0].KYONB(1);
  wait(); aica_sound.channel[0].LPCTL(1);
  wait(); aica_sound.channel[0].PCMS(0);
  wait(); aica_sound.channel[0].LSA(0);
  wait(); aica_sound.channel[0].LEA(44100);
  wait(); aica_sound.channel[0].D2R(0x0);
  wait(); aica_sound.channel[0].D1R(0x0);
  wait(); aica_sound.channel[0].RR(0x0);
  wait(); aica_sound.channel[0].AR(0x1f);

  wait(); aica_sound.channel[0].OCT(-3);
  wait(); aica_sound.channel[0].FNS(0);
  wait(); aica_sound.channel[0].DISDL(0xf);
  wait(); aica_sound.channel[0].DIPAN(0x0);

  wait(); aica_sound.channel[0].Q(0b00100);
  wait(); aica_sound.channel[0].TL(0);
  wait(); aica_sound.channel[0].LPOFF(1);

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
  wait(); aica_sound.channel[0].KYONB(1);
  wait(); aica_sound.channel[0].KYONEX(1);
  */

  while (1) {
  }
}
