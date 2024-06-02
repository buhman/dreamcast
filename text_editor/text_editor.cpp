#include <cstdint>

#include "align.hpp"
#include "twiddle.hpp"
#include "memorymap.hpp"

#include "holly/video_output.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_bits.hpp"

#include "sh7091/serial.hpp"

#include "font/font.hpp"

#include "ter_u20n.hpp"

#include "gap_buffer.hpp"
#include "viewport_window.hpp"
#include "keyboard.hpp"
#include "render.hpp"

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff0000ff);
}

void inflate_font(const uint32_t * src,
                  const uint32_t stride,
                  const uint32_t curve_end_ix)
{
  auto texture = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);

  twiddle::texture3<4, 1>(texture, reinterpret_cast<const uint8_t *>(src),
                          stride,
                          curve_end_ix);
}

uint32_t _ta_parameter_buf[((32 * 10 * 17) + 32) / 4];

struct editor_state {
  gap_buffer gb;
  viewport_window window;
};

constexpr uint32_t buf_size = 16 * 1024;
char_type buf[buf_size];
constexpr uint32_t offsets_size = 1024;
int32_t offsets[offsets_size];

void main()
{
  video_output::set_mode_vga();

  auto font = reinterpret_cast<const struct font *>(&_binary_ter_u20n_data_start);
  auto glyphs = reinterpret_cast<const struct glyph *>(&font[1]);
  auto texture = reinterpret_cast<const uint32_t *>(&glyphs[font->glyph_count]);

  inflate_font(texture,
               font->texture_stride,
               font->max_z_curve_ix);

  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;
  holly.PALETTE_RAM[0] = 0x8000;
  holly.PALETTE_RAM[1] = 0xffff;

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
    //| ta_alloc_ctrl::t_opb::_16x4byte
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
				       //, .translucent = 16 * 4
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;

  struct editor_state state = { 0 };
  gap_init_from_buf(state.gb, buf, buf_size, 0);
  line_init_from_buf(state.gb, offsets, offsets_size);
  viewport_init_fullscreen(state.window);

  ft6::data_transfer::data_format keyboards[2] = { 0 };

  while (true) {
    keyboard_do_get_condition(keyboards[frame_ix & 1]);
    keyboard_debug(keyboards, frame_ix);
    keyboard_update(keyboards, frame_ix, state.gb);

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    render(parameter, font, glyphs, state.gb, state.window);

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
