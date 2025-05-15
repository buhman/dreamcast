#include "aica/aica.hpp"

#include "xm/xm.h"
#include "xm/milkypack01.xm.h"

extern volatile uint32_t dram[0x200000] __asm("dram");

constexpr int max_patterns = 64;
constexpr int max_instruments = 128;
struct xm_state {
  xm_header_t * header;
  xm_pattern_header_t * pattern_header[max_patterns];
  xm_instrument_header_t * instrument_header[max_instruments];
  xm_sample_header_t * sample_header; // array
};

static xm_state xm = {0};

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

int xm_samples_init(int buf, int offset, int number_of_samples)
{
  xm_sample_header_t * sample_header[number_of_samples];
  for (int i = 0; i < number_of_samples; i++) {
    sample_header[i] = (xm_sample_header_t *)(buf + offset);
    offset += (sizeof (xm_sample_header_t));
  }

  for (int i = 0; i < number_of_samples; i++) {
    offset += s32(&sample_header[i]->sample_length);
  }
  return offset;
}

void print(int i)
{
  for (int i = 0; i < 100000; i++) {
    asm volatile ("nop");
  }
  dram[0] = i;
}

void xm_init()
{
  int buf = (int)(&_binary_xm_milkypack01_xm_start);

  while (xm.header != (xm_header_t *)(buf))
    xm.header = (xm_header_t *)(buf);

  int offset = s32(&xm.header->header_size) + (offsetof (struct xm_header, header_size));

  int number_of_patterns = s16(&xm.header->number_of_patterns);
  print(0x12345678);
  print(number_of_patterns);

  for (int i = 0; i < number_of_patterns; i++) {
    xm_pattern_header_t * pattern_header = (xm_pattern_header_t *)(buf + offset);

    xm.pattern_header[i] = pattern_header;

    offset += s32(&pattern_header->pattern_header_length) + s16(&pattern_header->packed_pattern_data_size);
    print(offset);
  }

  int number_of_instruments = s16(&xm.header->number_of_instruments);
  for (int i = 0; i < number_of_instruments; i++) {
    xm_instrument_header_t * instrument_header = (xm_instrument_header_t *)(buf + offset);

    xm.instrument_header[i] = instrument_header;
    offset += s32(&instrument_header->instrument_size);

    int number_of_samples = s16(&instrument_header->number_of_samples);
    offset = xm_samples_init(buf, offset, number_of_samples);
  }

  print(0x11223344);
  print(offset);
}

extern "C"
void main()
{
  volatile uint32_t * slot = reinterpret_cast<volatile uint32_t*>(0x00800000);
  for (uint32_t i = 0; i < (sizeof (struct aica_channel)) * 64 / 4; i++) {
    slot[i] = 0;
  }

  volatile uint32_t * dsp = reinterpret_cast<volatile uint32_t*>(0x00803000);
  for (int i = 0; i < 0xb00 / 4; i++) {
    dsp[i] = 0;
  }

  aica_sound.channel[0].KYONB(1);
  aica_sound.channel[0].LPCTL(1);
  aica_sound.channel[0].PCMS(0);
  aica_sound.channel[0].LSA(0);
  aica_sound.channel[0].LEA(128);
  aica_sound.channel[0].D2R(0x0);
  aica_sound.channel[0].D1R(0x0);
  aica_sound.channel[0].RR(0x0);
  aica_sound.channel[0].AR(0x1f);

  aica_sound.channel[0].OCT(0);
  aica_sound.channel[0].FNS(0);
  aica_sound.channel[0].DISDL(0xf);
  aica_sound.channel[0].DIPAN(0x0);

  aica_sound.channel[0].Q(0b00100);
  aica_sound.channel[0].TL(0);
  aica_sound.channel[0].LPOFF(1);

  aica_sound.common.mono_mem8mb_dac18b_ver_mvol =
      aica::mono_mem8mb_dac18b_ver_mvol::MONO(0)   // enable panpots
    | aica::mono_mem8mb_dac18b_ver_mvol::MEM8MB(0) // 16Mbit SDRAM
    | aica::mono_mem8mb_dac18b_ver_mvol::DAC18B(0) // 16-bit DAC
    | aica::mono_mem8mb_dac18b_ver_mvol::MVOL(0xf) // 15/15 volume
    ;

  uint32_t segment = 0;

  constexpr uint32_t timer_a_interrupt = (1 << 6);
  aica_sound.common.scire = timer_a_interrupt;
  bool started = 0;

  xm_init();

  while (1) {
    if (!started || (aica_sound.common.SCIPD() & timer_a_interrupt)) {
      //aica_sound.channel[0].SA(next_sa);
      aica_sound.common.tactl_tima =
          aica::tactl_tima::TACTL(0)  // increment once every 128 samples
        | aica::tactl_tima::TIMA(256 - 128) // interrupt after 128 counts
        ;

      if (!started) { aica_sound.channel[0].KYONEX(1); started = 1; }

      aica_sound.common.scire = timer_a_interrupt;
    }
  }
}
