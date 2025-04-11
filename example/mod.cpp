#include "aica/aica.hpp"
#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "mod/mod.hpp"
#include "mod/getfunk/getfunk.mod.h"

void wait()
{
  uint32_t ffst = system.FFST;
  while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
	| ffst::holly_g2_if_block_internal_write_buffer(ffst)
	| ffst::aica_internal_write_buffer(ffst)) {
    ffst = system.FFST;
  };
}

void main()
{
  serial::init(0);

  const uint8_t * start = reinterpret_cast<uint8_t *>(&_binary_mod_getfunk_getfunk_mod_start);
  const uint32_t size = reinterpret_cast<uint32_t>(&_binary_mod_getfunk_getfunk_mod_size);

  struct reader r = {
    .buf = start,
    .length = (int)size,
    .ix = 0,
  };

  struct mod mod;

  parse_mod_file(&r, &mod);

  serial::string(mod.tag, 4);

  serial::integer<uint32_t>(r.ix);
  serial::integer<uint32_t>(r.ix);
  serial::integer<uint32_t>(r.ix);

  // reset ARM
  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  // disable DSP / ARM / slot access ; enable SH4 access
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0111);

  volatile uint32_t * slot = reinterpret_cast<volatile uint32_t*>(0xa0700000);
  for (uint32_t i = 0; i < (sizeof (struct aica_channel)) * 64 / 4; i++) {
    wait(); slot[i] = 0;
  }

  volatile uint32_t * dsp = reinterpret_cast<volatile uint32_t*>(0xa0700000 + 0x3000);
  for (int i = 0; i < 0xb00 / 4; i++) {
    wait(); dsp[i] = 0;
  }

  uint32_t * buf32 = (uint32_t *)&start[r.ix];
  struct sample * sample = &mod.samples[0];
  uint32_t sample_bytes = sample->length * 2;
  for (uint32_t i = 0; i < sample_bytes / 4; i++) {
    wait();
    while (aica_wave_memory[i] != buf32[i]) {
      wait();
      aica_wave_memory[i] = buf32[i];
    }
  }

  // disable SH4 access / enable slot access
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b1101);

  wait(); aica_sound.channel[0].KYONB(1);
  wait(); aica_sound.channel[0].LPCTL(1);
  wait(); aica_sound.channel[0].PCMS(1); // 8-bit pcm
  wait(); aica_sound.channel[0].LSA(0);
  wait(); aica_sound.channel[0].LEA(sample_bytes);
  wait(); aica_sound.channel[0].D2R(0x0);
  wait(); aica_sound.channel[0].D1R(0x0);
  wait(); aica_sound.channel[0].RR(0x0);
  wait(); aica_sound.channel[0].AR(0x1f);

  wait(); aica_sound.channel[0].OCT(-2);
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

  wait(); aica_sound.channel[0].KYONEX(1);

  while (1);
}
