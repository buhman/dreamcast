#include "aica/aica.hpp"

extern void * _sine_start __asm("_binary_audio_pcm_start");

extern volatile uint32_t dram[0x200000] __asm("dram");

extern "C"
void main()
{
  const uint32_t sine_addr = reinterpret_cast<uint32_t>(&_sine_start);

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

  dram[0] = 0x11229944;
  dram[1] = sine_addr;
  constexpr uint32_t timer_a_interrupt = (1 << 6);
  aica_sound.common.scire = timer_a_interrupt;
  uint32_t next_sa = sine_addr;
  bool started = 0;

  while (1) {
    if (!started || (aica_sound.common.SCIPD() & timer_a_interrupt)) {
      aica_sound.channel[0].SA(next_sa);
      aica_sound.common.tactl_tima =
          aica::tactl_tima::TACTL(0)  // increment once every 128 samples
        | aica::tactl_tima::TIMA(256 - 128) // interrupt after 128 counts
        ;

      if (!started) { aica_sound.channel[0].KYONEX(1); started = 1; }

      aica_sound.common.scire = timer_a_interrupt;
      dram[1] = next_sa;

      segment += 1;
      if (segment >= 3440) segment = 0;
      next_sa = sine_addr + (128 * 2) * segment;
    }
  }
}
