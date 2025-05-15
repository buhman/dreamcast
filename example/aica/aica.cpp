#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

extern void * _binary_start __asm("_binary_example_arm_channel_bin_start");
extern void * _binary_size __asm("_binary_example_arm_channel_bin_size");

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
  const uint32_t * binary = reinterpret_cast<uint32_t *>(&_binary_start);
  const uint32_t binary_size = reinterpret_cast<uint32_t>(&_binary_size);

  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0);
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    while (aica_wave_memory[i] != binary[i]) {
      wait();
      aica_wave_memory[i] = binary[i];
    }
  }
  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(0);

  wait(); aica_sound.common.afsel_mslc_mobuf = aica::afsel_mslc_mobuf::MSLC(0);
  serial::string("mrwinh: ");
  wait();
  serial::integer<uint8_t>(aica_sound.common.MRWINH());
  while (1) {
    wait();
    serial::string("sgc: ");
    serial::integer<uint8_t>(aica_sound.common.SGC(), ' ');
    serial::string("; ca: ");
    serial::integer<uint8_t>(aica_sound.common.CA(), ' ');
    serial::string("; eg: ");
    serial::integer<uint8_t>(aica_sound.common.EG(), ' ');
    serial::string("; lp: ");
    serial::integer<uint8_t>(aica_sound.common.LP(), ' ');
    serial::character('\n');
    for (int i = 0; i < 10000000; i++) {
      asm volatile ("nop");
    }
    serial::integer<uint32_t>(aica_wave_memory[0], ' ');
    serial::integer<uint32_t>(aica_wave_memory[1], '\n');
  }

  while (1);
}
