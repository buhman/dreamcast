#include "aica/aica.hpp"
#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "mod/mod.hpp"
#include "mod/getfunk/getfunk.mod.h"

/*


Octave 1: 856, C
 */

struct oct_fns
{
  int oct;
  int fns;
};

struct oct_fns oct_fns(int x)
{
  const uint16_t _cent[] = {
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

  switch (x) {
  case 856: return {2, _cent[0]}; // C

  case 808: return {1, _cent[11]}; // C#
  case 762: return {1, _cent[10]}; // D
  case 720: return {1, _cent[9]}; // D#
  case 678: return {1, _cent[8]}; // E
  case 640: return {1, _cent[7]}; // F
  case 604: return {1, _cent[6]}; // F#
  case 570: return {1, _cent[5]}; // G
  case 538: return {1, _cent[4]}; // G#
  case 508: return {1, _cent[3]}; // A
  case 480: return {1, _cent[2]}; // A#
  case 453: return {1, _cent[1]}; // B
  case 428: return {1, _cent[0]}; // C

  case 404: return {0, _cent[11]}; // D#
  case 381: return {0, _cent[10]}; // D
  case 360: return {0, _cent[9]}; // D#
  case 339: return {0, _cent[8]}; // E
  case 320: return {0, _cent[7]}; // F
  case 302: return {0, _cent[6]}; // F#
  case 285: return {0, _cent[5]}; // G
  case 269: return {0, _cent[4]}; // G#
  case 254: return {0, _cent[3]}; // A
  case 240: return {0, _cent[2]}; // A#
  case 226: return {0, _cent[1]}; // B
  case 214: return {0, _cent[0]}; // C

  case 202: return {-1, _cent[11]}; // D#
  case 190: return {-1, _cent[10]}; // D
  case 180: return {-1, _cent[9]}; // D#
  case 170: return {-1, _cent[8]}; // E
  case 160: return {-1, _cent[7]}; // F
  case 151: return {-1, _cent[6]}; // F#
  case 143: return {-1, _cent[5]}; // G
  case 135: return {-1, _cent[4]}; // G#
  case 127: return {-1, _cent[3]}; // A
  case 120: return {-1, _cent[2]}; // A#
  case 113: return {-1, _cent[1]}; // B
  case 107: return {-1, _cent[0]}; // C

  default:
    return {-1, -1};
  }
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
  wait(); aica_sound.channel[0].LPCTL(0);
  wait(); aica_sound.channel[0].PCMS(1); // 8-bit pcm
  wait(); aica_sound.channel[0].LSA(0);
  wait(); aica_sound.channel[0].LEA(sample_bytes);
  wait(); aica_sound.channel[0].D2R(0x0);
  wait(); aica_sound.channel[0].D1R(0x0);
  wait(); aica_sound.channel[0].RR(0x0);
  wait(); aica_sound.channel[0].AR(0x1f);

  wait(); aica_sound.channel[0].OCT(-2);
  wait(); aica_sound.channel[0].FNS(0x1a8);
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

  int division = 0;
  struct pattern * pattern = &mod.patterns[3];

  constexpr uint32_t timer_a_interrupt = (1 << 6);
  int started = 0;
  while (1) {
    wait();
    if (!started || (aica_sound.common.SCIPD() & timer_a_interrupt)) {
      started = 1;
      aica_sound.common.scire = timer_a_interrupt;
      aica_sound.common.tactl_tima =
          aica::tactl_tima::TACTL(6)  // increment once every 128 samples
        | aica::tactl_tima::TIMA(256 - 128) // interrupt after 128 counts
        ;


      struct channel * c = &pattern->division[division][0];
      if (c->parameter != 0) {
        struct oct_fns of = oct_fns(c->parameter);
        wait(); aica_sound.channel[0].KYONB(0);
        wait(); aica_sound.channel[0].KYONEX(1);

        wait(); aica_sound.channel[0].OCT(of.oct - 1);
        wait(); aica_sound.channel[0].FNS(of.fns);

        wait(); aica_sound.channel[0].KYONB(1);
        wait(); aica_sound.channel[0].KYONEX(1);
      }


      division += 1;
      if (division >= 64) {
        break;
        division = 0;
      }
    }
  }
}
