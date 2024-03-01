#include "memorymap.hpp"
#include "aica/aica.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

extern void * _binary_start __asm("_binary_example_arm_channel_bin_start");
extern void * _binary_size __asm("_binary_example_arm_channel_bin_size");

void wait()
{
  while (ffst::aica_internal_write_buffer(system.FFST));
}

void wait_read()
{
  uint32_t ffst = ~0;
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

  wait(); aica.common.reg_2c00 = 1;
  wait(); aica.common.reg_2880 = 0;
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    aica_wave_memory[i] = binary[i];
    if (i % 8 == 7) wait();
  }
  wait(); aica.common.reg_2c00 = 0;

  serial::integer<uint32_t>(aica_wave_memory[0]);

  wait(); aica.common.MSLC(0);
  serial::string("mrwinh: ");
  wait_read();
  serial::integer<uint8_t>(aica.common.MRWINH());
  while (1) {
    wait_read();
    serial::string("sgc: ");
    serial::integer<uint8_t>(aica.common.SGC(), ' ');
    serial::string("; ca: ");
    serial::integer<uint8_t>(aica.common.CA(), ' ');
    serial::string("; eg: ");
    serial::integer<uint8_t>(aica.common.EG(), ' ');
    serial::string("; lp: ");
    serial::integer<uint8_t>(aica.common.LP(), ' ');
    serial::character('\n');
    for (int i = 0; i < 10000000; i++) {
      asm volatile ("nop");
    }
  }

  while (1);
}
