#include <cstdint>

#include "align.hpp"
#include "twiddle.hpp"
#include "memorymap.hpp"

#include "holly/video_output.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/texture_memory_alloc3.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_bits.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "sh7091/vbr.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "font/font.h"

#include "font/terminus/ter_u20n.data.h"

#include "gap_buffer.hpp"
#include "viewport_window.hpp"
#include "keyboard.hpp"
#include "render.hpp"

void vbr100()
{
  serial::string("vbr100\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);
  while (1);
}

void vbr400()
{
  serial::string("vbr400");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);
  while (1);
}

static int render_done = 0;

void vbr600()
{
  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) {
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(system.ISTERR);
    }

    if (istnrm & istnrm::end_of_render_tsp) {
      system.ISTNRM = istnrm::end_of_render_tsp
                    | istnrm::end_of_render_isp
                    | istnrm::end_of_render_video;
      render_done = 1;
      return;
    }
  }

  serial::string("vbr600");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);

  serial::string("istnrm: ");
  serial::integer<uint32_t>(system.ISTNRM);
  serial::string("isterr: ");
  serial::integer<uint32_t>(system.ISTERR);


  serial::string("halt\n");
  while (1);
}

void interrupt_init()
{
  system.IML2NRM = 0;
  system.IML2ERR = 0;
  system.IML2EXT = 0;

  system.IML4NRM = 0;
  system.IML4ERR = 0;
  system.IML4EXT = 0;

  system.IML6NRM = 0;
  system.IML6ERR = 0;
  system.IML6EXT = 0;

  system.ISTERR = 0xffffffff;
  system.ISTNRM = 0xffffffff;

  sh7091.CCN.INTEVT = 0;
  sh7091.CCN.EXPEVT = 0;

  uint32_t vbr = reinterpret_cast<uint32_t>(&__vbr_link_start) - 0x100;
  serial::string("vbr ");
  serial::integer<uint32_t>(vbr);
  serial::string("vbr100 ");
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&vbr100));

  asm volatile ("ldc %0,vbr"
		:
		: "r" (vbr));

  uint32_t sr;
  asm volatile ("stc sr,%0"
		: "=r" (sr));
  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  sr &= ~sh::sr::bl; // BL
  sr &= ~sh::sr::imask(15); // imask

  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  asm volatile ("ldc %0,sr"
		:
		: "r" (sr));
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, const void * src, int length)
{
  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffc0) / 4];
  const uint32_t * src32 = reinterpret_cast<const uint32_t *>(src);

  length = (length + 31) & ~31; // round up to nearest multiple of 32
  while (length > 0) {
    base[0] = src32[0];
    base[1] = src32[1];
    base[2] = src32[2];
    base[3] = src32[3];
    base[4] = src32[4];
    base[5] = src32[5];
    base[6] = src32[6];
    base[7] = src32[7];
    asm volatile ("pref @%0"
                  :                // output
                  : "r" (&base[0]) // input
                  : "memory");
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_texture(const void * src, uint32_t size)
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  uint32_t offset = texture_memory_alloc.texture.start;
  void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
  transfer_ta_fifo_texture_memory_32byte(dst, src, size);
}

uint32_t ta_parameter_buf[1024 * 1024 / 4] __attribute__((aligned(32)));

struct editor_state {
  gap_buffer gb;
  viewport_window window;
};

constexpr uint32_t buf_size = 16 * 1024;
char_type buf[buf_size];
constexpr uint32_t offsets_size = 1024;
int32_t offsets[offsets_size];

void transfer_palettes()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;

  holly.PALETTE_RAM[0] = 0x0000;
  holly.PALETTE_RAM[1] = 0xffff;
}

void main()
{
  serial::init(0);

  const struct font * font = reinterpret_cast<const struct font *>(&_binary_font_terminus_ter_u20n_data_start);
  const struct glyph * glyphs = reinterpret_cast<const struct glyph *>(&font[1]);
  const void * texture = reinterpret_cast<const void *>(&glyphs[font->glyph_count]);

  transfer_texture(texture, font->max_z_curve_ix);
  transfer_palettes();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::_16x4byte
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  const int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 16 * 4,
      .opaque_modifier = 0,
      .translucent = 16 * 4,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         render_passes,
                         texture_memory_alloc.region_array[0].start,
                         texture_memory_alloc.object_list[0].start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff202040);

  core_init();

  uint32_t frame_ix = 0;

  struct editor_state state = { 0 };
  gap_init_from_buf(state.gb, buf, buf_size, 0);
  line_init_from_buf(state.gb, offsets, offsets_size);
  viewport_init_fullscreen(state.window, font);

  ft6::data_transfer::data_format keyboards[2] = { 0 };

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  interrupt_init();
  system.IML6NRM = istnrm::end_of_render_tsp;
  video_output::set_mode_vga();

  while (true) {
    keyboard_do_get_condition(keyboards[frame_ix & 1]);
    keyboard_debug(keyboards, frame_ix);
    keyboard_update(keyboards, frame_ix, state.gb, state.window);

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[0].start,
			       texture_memory_alloc.isp_tsp_parameters[0].end,
			       texture_memory_alloc.object_list[0].start,
			       texture_memory_alloc.object_list[0].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    writer.offset = 0;
    render(writer, font, glyphs, state.gb, state.window);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_translucent_list();

    render_done = 0;
    core_start_render2(texture_memory_alloc.region_array[0].start,
                       texture_memory_alloc.isp_tsp_parameters[0].start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[frame_ix].start,
                       framebuffer_width);
    while (render_done == 0) {
      asm volatile ("nop");
    };

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[frame_ix].start;
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
