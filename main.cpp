#include <stdint.h>

#include "memorymap.h"

#include "sh7091.h"
#include "sh7091_bits.h"
#include "holly.h"
#include "holly/core.h"
#include "holly/core_bits.h"
#include "holly/ta_fifo_polygon_converter.h"
#include "systembus.h"
#include "maple.h"
#include "maple_bits.h"

#include "holly/texture_memory_alloc.h"

#include "cache.h"
#include "load.h"
#include "vga.h"
#include "rgb.h"
#include "string.h"
#include "scene.h"

#include "macaw.h"

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

void serial_int(const uint32_t n)
{
  char num_buf[9];
  string::hex<char>(num_buf, 8, n);
  num_buf[8] = 0;
  serial_string("0x");
  serial_string(num_buf);
  serial_string("\n");
}

void maple_test()
{
  uint32_t _command_buf[(32 + 32) / 4];
  uint32_t _receive_address[(32 + 32) / 4];
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_address = align_32byte(_receive_address);

  serial_int(mdstar::table_address(reinterpret_cast<uint32_t>(command_buf)));
  maple_init_host_command(command_buf, receive_address);
  maple_dma_start(command_buf);

  for (int i = 0; i < 32; i++) {
    serial_int(receive_address[i]);
  }
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

  maple_test();

  volatile uint16_t * framebuffer = reinterpret_cast<volatile uint16_t *>(&texture_memory[0]);
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      struct hsv hsv = {(y * 255) / 480, 255, 255};
      struct rgb rgb = hsv_to_rgb(hsv);
      framebuffer[y * 640 + x] = ((rgb.r >> 3) << 11) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 0);
    }
  }

  volatile texture_memory_alloc * mem = reinterpret_cast<volatile texture_memory_alloc *>(0xa400'0000);

  volatile uint8_t * macaw = reinterpret_cast<volatile uint8_t *>(&_binary_macaw_data_start);
  uint32_t macaw_size = reinterpret_cast<uint32_t>(&_binary_macaw_data_size);
  for (uint32_t px = 0; px < macaw_size / 3; px++) {
    uint8_t r = macaw[px * 3 + 0];
    uint8_t g = macaw[px * 3 + 1];
    uint8_t b = macaw[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    mem->texture[px] = rgb565;
  }

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  //system.LMMODE0 = 1; // texture memory through TA FIFO
  //system.LMMODE1 = 1; // texture memory through TA FIFO (mirror)

  v_sync_out();
  v_sync_in();

  core_init();
  core_init_texture_memory();

  // the address of `scene` must be a multiple of 32 bytes
  // this is mandatory for ch2-dma to the ta fifo polygon converter
  uint32_t * scene = align_32byte(_scene);
  if ((reinterpret_cast<uint32_t>(scene) & 31) != 0) {
    serial_string("unaligned\n");
    while(1);
  }

  int frame = 0;

  while (true) {
    v_sync_out();
    v_sync_in();

    ta_polygon_converter_init();
    uint32_t ta_parameter_size = scene_transform(scene);
    ta_polygon_converter_transfer(scene, ta_parameter_size);
    ta_wait_opaque_list();
    core_start_render(frame);

    frame = !frame;
  }
}
