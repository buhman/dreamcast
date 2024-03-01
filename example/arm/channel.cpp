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
  aica_sound.channel[0].SA(sine_addr);
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

  aica_sound.common.MVOL(0xf);

  uint32_t segment = 0;
  aica_sound.common.TACTL(7); // increment once every 128 samples
  aica_sound.common.TIMA(255);
  aica_sound.channel[0].KYONEX(1);

  dram[0] = 0x11223344;
  dram[1] = sine_addr;
  constexpr uint32_t timer_a_interrupt = (1 << 6);
  aica_sound.common.scire = timer_a_interrupt;
  while (1) {
    if (aica_sound.common.SCIPD() & timer_a_interrupt) {
      aica_sound.common.scire = timer_a_interrupt;
      aica_sound.common.TIMA(255);
      segment += 1;
      if (segment >= 3440) segment = 0;
      uint32_t sa = sine_addr + (128 * 2) * segment;
      dram[1] = sa;
      aica_sound.channel[0].SA(sa);
    }
  }
}
