#include "aica/aica.hpp"

extern volatile uint32_t dram[0x200000] __asm("dram");

constexpr uint32_t sectors_per_chunk = 16;
constexpr uint32_t chunk_size = 2048 * sectors_per_chunk;

__attribute__((section(".buffers.chunk")))
static uint32_t chunk[2][chunk_size / 4];

void request_chunk()
{
  constexpr uint32_t mcipd__sh4_interrupt = (1 << 5);
  aica_sound.common.mcipd = mcipd__sh4_interrupt;
}

void wait_sh4_response()
{
  constexpr uint32_t scipd__arm_interrupt = (1 << 5);
  while ((aica_sound.common.SCIPD() & scipd__arm_interrupt) == 0) {
  }
  aica_sound.common.scire = scipd__arm_interrupt;
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
  aica_sound.channel[0].LEA(chunk_size / 2);
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

  constexpr uint32_t tactl = 7;        // increment once every 1 samples
  constexpr uint32_t tima = 256 - 128; // interrupt after 128 samples

  dram[0] = reinterpret_cast<uint32_t>(&chunk[0][0]);
  dram[1] = reinterpret_cast<uint32_t>(&chunk[1][0]);

  uint32_t next_chunk = 0;
  aica_sound.channel[0].SA(reinterpret_cast<const uint32_t>(&chunk[next_chunk][0]));
  aica_sound.channel[0].KYONEX(1);
  next_chunk = (next_chunk + 1) % 2;
  request_chunk();

  constexpr uint32_t timer_a_interrupt = (1 << 6);
  aica_sound.common.tactl_tima = aica::tactl_tima::TACTL(tactl)
			       | aica::tactl_tima::TIMA(tima);
  aica_sound.common.scire = timer_a_interrupt;

  uint32_t sample = 0;
  while (1) {
    if (aica_sound.common.SCIPD() & timer_a_interrupt) {
      aica_sound.channel[0].SA(reinterpret_cast<const uint32_t>(&chunk[next_chunk][0]));
      aica_sound.common.tactl_tima = aica::tactl_tima::TACTL(tactl)
        | aica::tactl_tima::TIMA(tima);
      next_chunk = (next_chunk + 1) % 2;
      request_chunk();

      aica_sound.common.scire = timer_a_interrupt;
    }
  }
}