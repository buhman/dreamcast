#include <stdint.h>

#include "memorymap.h"

#include "sh7091.h"
#include "sh7091_bits.h"
#include "holly.h"
#include "holly/core.h"
#include "holly/core_bits.h"
#include "holly/ta_fifo_polygon_converter.h"
#include "systembus.h"

#include "cache.h"
#include "load.h"
#include "vga.h"
#include "rgb.h"
#include "scene.h"

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

void serial()
{
  sh7091.SCIF.SCSCR2 = 0;
  sh7091.SCIF.SCSMR2 = 0;
  sh7091.SCIF.SCBRR2 = 1; // 520833.3

  sh7091.SCIF.SCFCR2 = SCFCR2__TFRST | SCFCR2__RFRST;
  // tx/rx trigger on 1 byte
  sh7091.SCIF.SCFCR2 = 0;

  sh7091.SCIF.SCSPTR2 = 0;
  sh7091.SCIF.SCLSR2 = 0;

  sh7091.SCIF.SCSCR2 = SCSCR2__TE | SCSCR2__RE;
}

inline void serial_char(const char c)
{
  // wait for transmit fifo to become empty
  while ((sh7091.SCIF.SCFSR2 & SCFSR2__TDFE) == 0);

  sh7091.SCIF.SCFTDR2 = static_cast<uint8_t>(c);
}

void serial_string(const char * s)
{
  while (*s != '\0') {
    serial_char(*s++);
  }
}

/* must be aligned to 32-bytes for DMA transfer */
// the aligned(32) attribute does not actually align to 32 bytes; gcc is the best compiler.
// `+ 32` to allow for repositioning _scene to an actual 32-byte alignment.
uint32_t __attribute__((aligned(32))) _scene[((32 * 6) + 32) / 4];

uint32_t * align_32byte(uint32_t * mem)
{
  return reinterpret_cast<uint32_t *>(((reinterpret_cast<uint32_t>(_scene) + 31) & ~31));
}

extern "C"
void main()
{
  cache_init();

  // clear BSS
  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  //serial();

  vga();

  v_sync_in();

  volatile uint16_t * framebuffer = reinterpret_cast<volatile uint16_t *>(&texture_memory[0]);
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      struct hsv hsv = {(y * 255) / 480, 255, 255};
      struct rgb rgb = hsv_to_rgb(hsv);
      framebuffer[y * 640 + x] = ((rgb.r >> 3) << 11) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 0);
    }
  }

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  system.LMMODE0 = 1;
  system.LMMODE1 = 1;

  v_sync_out();
  v_sync_in();

  core_init();
  core_init_texture_memory();

  int frame = 0;

  // the address of `scene` must be a multiple of 32 bytes
  // this is mandatory for ch2-dma to the ta fifo polygon converter
  uint32_t * scene = align_32byte(_scene);
  if ((reinterpret_cast<uint32_t>(scene) & 31) != 0) {
    serial_string("unaligned\n");
    while(1);
  }

  while (true) {
    v_sync_out();
    v_sync_in();

    ta_polygon_converter_init();
    uint32_t ta_parameter_count = scene_transform(&scene[0]);
    uint32_t ta_parameter_size = ta_parameter_count * 32; /* 32 bytes per parameter */
    ta_polygon_converter_transfer(&scene[0], ta_parameter_size);
    ta_wait_opaque_list();
    core_start_render(frame);

    frame = !frame;
  }
}
