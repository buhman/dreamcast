#include "memorymap.hpp"
#include "sh7091/serial.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"
#include "aica/aica.hpp"

extern void * _audio_pcm_start __asm("_binary_audio_pcm_start");

extern void * _binary_start __asm("_binary_example_arm_sh4_interrupt_bin_start");
extern void * _binary_size __asm("_binary_example_arm_sh4_interrupt_bin_size");

constexpr uint32_t mcipd__sh4_interrupt = (1 << 5);
constexpr uint32_t scipd__arm_interrupt = (1 << 5);

void aica_wait_write()
{
  while (ffst::aica_internal_write_buffer(system.FFST));
}

void aica_wait_read()
{
  uint32_t ffst = system.FFST;
  while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
	| ffst::holly_g2_if_block_internal_write_buffer(ffst)
	| ffst::aica_internal_write_buffer(ffst)) {
    ffst = system.FFST;
  };
}

void fill_chunk(volatile uint32_t * chunk, const uint32_t segment_index)
{
  const uint32_t * audio_pcm = reinterpret_cast<const uint32_t *>(&_audio_pcm_start);
  const uint32_t * segment = &audio_pcm[(segment_index * 128 * 2) / 4];

  for (int i = 0; i < (128 * 2) / 4; i++) {
    if (i % 8 == 0) aica_wait_write();
    chunk[i] = segment[i];
  }
}

static volatile uint32_t (* chunk)[2][(128 * 2) / 4];

void aica_init(uint32_t& chunk_index, uint32_t& segment_index)
{
  const uint32_t * binary = reinterpret_cast<uint32_t *>(&_binary_start);
  const uint32_t binary_size = reinterpret_cast<uint32_t>(&_binary_size);

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  aica_wait_write(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0);
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    while (aica_wave_memory[i] != binary[i]) {
      aica_wait_write();
      aica_wave_memory[i] = binary[i];
    }
  }

  chunk = reinterpret_cast<decltype (chunk)>(&aica_wave_memory[0x001ff000 / 4]);

  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[0][0]));
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[1][0]));

  fill_chunk(&(*chunk)[chunk_index][0], segment_index);
  chunk_index = (chunk_index + 1) % 2;
  segment_index += 1;

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(0);
}

void aica_step(uint32_t& chunk_index, uint32_t& segment_index)
{
  aica_wait_read();
  { // wait for interrupt from arm
    while ((aica_sound.common.MCIPD() & mcipd__sh4_interrupt) == 0) { aica_wait_read(); };
    aica_wait_write(); aica_sound.common.mcire = mcipd__sh4_interrupt;
  }

  { // fill the requested chunk
    fill_chunk(&(*chunk)[chunk_index][0], segment_index);

    chunk_index = (chunk_index + 1) % 2;
    segment_index += 1;
    if (segment_index >= 3440) segment_index = 0;
  }
}

void main()
{
  uint32_t chunk_index = 0;
  uint32_t segment_index = 0;

  aica_init(chunk_index, segment_index);

  while (1) {
    aica_step(chunk_index, segment_index);
  }

  while (1);
}
