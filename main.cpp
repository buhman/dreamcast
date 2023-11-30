#include <stdint.h>

#include "cache.h"
#include "load.h"
#include "vga.h"

#include "sh7091.h"
#include "sh7091_bits.h"
#include "memorymap.h"

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

  serial();

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


  while (1) {
    v_sync_in();
    scene_holly_init();
    scene_init_texture_memory();
    scene_ta_init();
    scene_geometry_transfer();
    scene_wait_opaque_list();
    scene_start_render();

    // I do not understand why, but flycast does not show the first-rendered
    // framebuffer.
    v_sync_in();
    scene_ta_init();
    scene_geometry_transfer();
    scene_wait_opaque_list();
    scene_start_render();

    // do nothing forever
    while(1);

  /*
  load_init();

  while (1) {
    while ((sh7091.SCIF.SCFSR2 & SCFSR2__RDF) == 0) {
      // wait
    }
    while ((sh7091.SCIF.SCFDR2 & 0b11111) > 0) {
      uint8_t c = sh7091.SCIF.SCFRDR2;
      load_recv(c);
    }
    sh7091.SCIF.SCFSR2 = sh7091.SCIF.SCFSR2 & (~SCFSR2__RDF);
  }
*/
  }
}
