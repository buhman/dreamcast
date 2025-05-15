#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"
#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

#include "assert.h"

#include "example/arm/xm.bin.h"

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

constexpr uint32_t dma_address_mask = 0x1fffffe0;

void g2_aica_dma(uint32_t g2_address, uint32_t system_address, int length)
{
  using namespace dmac;

  length = (length + 31) & (~31);

  // is DMAOR needed?
  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */


  g2_if.G2APRO = 0x4659007f; // disable protection

  g2_if.ADSTAG = dma_address_mask & g2_address; // G2 address
  g2_if.ADSTAR = dma_address_mask & system_address; // system memory address
  g2_if.ADLEN = length;
  g2_if.ADDIR = 0; // from root bus to G2 device
  g2_if.ADTSEL = 0; // CPU controlled trigger
  g2_if.ADEN = 1; // enable G2-DMA
  g2_if.ADST = 1; // start G2-DMA
}

void g2_aica_dma_wait_complete()
{
  // wait for maple DMA completion
  while ((system.ISTNRM & istnrm::end_of_dma_aica_dma) == 0);
  system.ISTNRM = istnrm::end_of_dma_aica_dma;
}

uint8_t __attribute__((aligned(32))) zero[32768] = {0};

void main()
{
  serial::init(0);

  const int start = reinterpret_cast<int>(&_binary_start);
  const int size = reinterpret_cast<int>(&_binary_size);

  wait(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0111);
  system.ISTNRM = istnrm::end_of_dma_aica_dma;

  g2_aica_dma((uint32_t)&aica_sound, (int)zero, 32768);
  g2_aica_dma_wait_complete();
  assert(g2_if.ADST == 0);
  g2_aica_dma((uint32_t)aica_wave_memory, start, size);
  g2_aica_dma_wait_complete();
  assert(g2_if.ADST == 0);

  for (int i = 0; i < size / 4; i++) {
    wait();
    assert(aica_wave_memory[i] == ((uint32_t*)start)[i]);
  }

  wait(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0b0001);

  wait(); aica_sound.channel[0].KYONB(1);
  wait(); aica_sound.channel[0].LPCTL(1);
  wait(); aica_sound.channel[0].PCMS(0);
  wait(); aica_sound.channel[0].LSA(0);
  wait(); aica_sound.channel[0].LEA(44100);
  wait(); aica_sound.channel[0].D2R(0x0);
  wait(); aica_sound.channel[0].D1R(0x0);
  wait(); aica_sound.channel[0].RR(0x0);
  wait(); aica_sound.channel[0].AR(0x1f);

  wait(); aica_sound.channel[0].OCT(0);
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

  wait(); aica_sound.channel[0].SA(44100 * 2);
  wait(); aica_sound.channel[0].KYONEX(1);

  while (1) {
    for (int i = 0; i < 10000000; i++) {
      asm volatile ("nop");
    }
  }
}
