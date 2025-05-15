#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

#include "example/arm/xm.bin.h"

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
  const uint32_t * binary = reinterpret_cast<uint32_t *>(&_binary_example_arm_xm_bin_start);
  const uint32_t binary_size = reinterpret_cast<uint32_t>(&_binary_example_arm_xm_bin_size);

  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0111);
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    while (aica_wave_memory[i] != binary[i]) {
      wait();
      aica_wave_memory[i] = binary[i];
    }
  }
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0001);
  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(0);

  wait(); aica_sound.common.afsel_mslc_mobuf = aica::afsel_mslc_mobuf::MSLC(0);
  serial::string("mrwinh: ");
  wait();
  serial::integer<uint8_t>(aica_sound.common.MRWINH());
  int last_dram = -1;
  while (1) {
    wait();
    int read = aica_wave_memory[0];
    if (read != last_dram) {
      serial::integer<uint32_t>(read);
    }
    last_dram = read;
  };
}
