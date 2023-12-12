#include <cstdint>

#include "memorymap.hpp"

#include "sh7091.hpp"
#include "sh7091_bits.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "systembus.hpp"
#include "maple/maple.hpp"
#include "maple/maple_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "holly/texture_memory_alloc.hpp"

#include "cache.hpp"
#include "load.hpp"
#include "vga.hpp"
#include "rgb.hpp"
#include "string.hpp"
#include "scene.hpp"

#include "macaw.hpp"

/* must be aligned to 32-bytes for DMA transfer */
// the aligned(32) attribute does not actually align to 32 bytes; gcc is the best compiler.
// `+ 32` to allow for repositioning _scene to an actual 32-byte alignment.
// __attribute__((aligned(32)))
uint32_t _scene[((32 * 6) + 32) / 4];

uint32_t _receive_address[(256 + 32) / 4] = {0};
uint32_t _command_buf[(256 + 32) / 4] = {0};

bool maple_test()
{
  v_sync_out();

  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_address = align_32byte(_receive_address);
  if ((((uint32_t)command_buf) & 31) != 0) serial_string("misaligned\n");
  if ((((uint32_t)receive_address) & 31) != 0) serial_string("misaligned\n");

  //maple_init_device_request(command_buf, receive_address);
  //maple_init_get_condition(command_buf, receive_address);

  serial_int32(command_buf[0]);
  serial_char('\n');

  maple_dma_start(command_buf);

  v_sync_in();

  for (int i = 0; i < 32; i++) {
    serial_int32(receive_address[i]);
  }

  /*
  for (int i = 0; i < (4 + 4 + 8); i++) {
    serial_int8(reinterpret_cast<volatile uint8_t *>(receive_address)[i]);
  }
  */

  // the data format for a FT0 (controller) data read
  auto data_format = reinterpret_cast<volatile data_transfer::data_fields<ft0::data_transfer::data_format> *>(&receive_address[1]);
  return !(data_format->data.digital_button & ft0::data_transfer::digital_button::a);
}

void main()
{
  //serial();

  vga();

  maple_test();
  //((void(*)(void))0xac010000)();

  /*
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
  */

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
  bool a_pressed = 0;
  uint32_t color;

  while (true) {
    v_sync_out();
    v_sync_in();

    ta_polygon_converter_init();
    if (a_pressed) { color = 0xffffffff; } else { color = 0xffff7f00; }
    uint32_t ta_parameter_size = scene_transform_quad(scene, color);
    ta_polygon_converter_transfer(scene, ta_parameter_size);
    ta_wait_opaque_list();
    core_start_render(frame);

    //a_pressed = maple_test();

    frame = !frame;
  }
}
