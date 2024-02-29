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

  aica.channel[0].KYONB(1);
  aica.channel[0].LPCTL(1);
  aica.channel[0].PCMS(0);
  aica.channel[0].SA(sine_addr);
  aica.channel[0].LSA(0);
  aica.channel[0].LEA(128);
  aica.channel[0].D2R(0x0);
  aica.channel[0].D1R(0x0);
  aica.channel[0].RR(0x0);
  aica.channel[0].AR(0x1f);

  aica.channel[0].OCT(0);
  aica.channel[0].FNS(0);
  aica.channel[0].DISDL(0xf);
  aica.channel[0].DIPAN(0x0);

  aica.channel[0].Q(0b00100);
  aica.channel[0].TL(0);
  aica.channel[0].LPOFF(1);

  aica.common.MVOL(0xf);

  uint32_t segment = 0;
  aica.common.TACTL(7); // increment once every 128 samples
  aica.common.TIMA(0);
  aica.channel[0].KYONEX(1);

  dram[0] = 0x11223344;

  while (1) {
    if (aica.common.TIMA() >= 1) {
      aica.common.TIMA(0);
      segment += 1;
      if (segment >= 3440) segment = 0;
      aica.channel[0].SA(sine_addr + (128 * 2) * segment);
    }
  }
}
