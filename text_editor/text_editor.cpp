#include <cstdint>
#include <bit>

#include "align.hpp"
#include "holly/video_output.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "memorymap.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"
#include "twiddle.hpp"
#include "palette.hpp"

#include "sh7091/serial.hpp"

#include "maple/maple.hpp"
#include "maple/maple_impl.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "font/font.hpp"
#include "ter_u20n.hpp"

#include "gap_buffer.hpp"

struct vertex {
  float x;
  float y;
  float u;
  float v;
};

const struct vertex strip_vertices[4] = {
  // [ position ]  [ uv coordinates ]
  {  0.f,  0.f,    0.f, 0.f, },
  {  1.f,  0.f,    1.f, 0.f, },
  {  1.f,  1.f,    1.f, 1.f, },
  {  0.f,  1.f,    0.f, 1.f, },
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

constexpr inline float float_26_6(int32_t n)
{
  float v = n >> 6;
//float d = n & 63;
//return v + (d / 64.f);
  return v;
}

void transform(ta_parameter_writer& parameter,
	       const uint32_t texture_width, uint32_t texture_height,
	       const glyph& glyph,
	       const float origin_x,
	       const float origin_y)
{
  const uint32_t parameter_control_word = para_control::para_type::sprite
					| para_control::list_type::opaque
					| obj_control::col_type::packed_color
					| obj_control::texture
					| obj_control::_16bit_uv;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
					  | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
				      | tsp_instruction_word::dst_alpha_instr::one
				      | tsp_instruction_word::fog_control::no_fog
				      | tsp_instruction_word::use_alpha
				      | tsp_instruction_word::texture_u_size::from_int(texture_width)
				      | tsp_instruction_word::texture_v_size::from_int(texture_height);

  const uint32_t texture_address = texture_memory_alloc::texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
				      | texture_control_word::scan_order::twiddled
				      | texture_control_word::texture_address(texture_address / 8);

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
					isp_tsp_instruction_word,
					tsp_instruction_word,
					texture_control_word,
					0, // data_size_for_sort_dma
					0  // next_address_for_sort_dma
					);

  struct vertex out[strip_length];
  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;

    x *= glyph.bitmap.width;
    y *= glyph.bitmap.height;
    x += origin_x + float_26_6(glyph.metrics.horiBearingX);
    y += origin_y - float_26_6(glyph.metrics.horiBearingY);

    float u = strip_vertices[i].u;
    float v = strip_vertices[i].v;
    u *= glyph.bitmap.width;
    v *= glyph.bitmap.height;
    u += glyph.bitmap.x;
    v += glyph.bitmap.y;
    u = u / static_cast<float>(texture_width);
    v = v / static_cast<float>(texture_height);

    out[i].x = x;
    out[i].y = y;
    out[i].u = u;
    out[i].v = v;
  }

  const float z = 0.1f;
  parameter.append<ta_vertex_parameter::sprite_type_1>() =
    ta_vertex_parameter::sprite_type_1(para_control::para_type::vertex_parameter,
				       out[0].x,
				       out[0].y,
				       z,
				       out[1].x,
				       out[1].y,
				       z,
				       out[2].x,
				       out[2].y,
				       z,
				       out[3].x,
				       out[3].y,
				       uv_16bit(out[0].u, out[0].v),
				       uv_16bit(out[1].u, out[1].v),
				       uv_16bit(out[2].u, out[2].v)
				       );
}

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

uint32_t _command_buf[(1024 + 32) / 4];
uint32_t _receive_buf[(1024 + 32) / 4];

struct button_state {
  bool ra;
  bool la;
  bool da;
  bool ua;
  bool a;
  bool b;
  bool x;
  bool y;
  bool start;

  void reset()
  {
    ra = la = da = ua = 0;
    a = b = x = y = 0;
    start = 0;
  }
};

void do_get_condition(uint32_t * command_buf,
		      uint32_t * receive_buf,
		      button_state& buttons)
{
  using command_type = get_condition;
  using response_type = data_transfer<ft0::data_transfer::data_format>;

  get_condition::data_fields data_fields = {
    .function_type = std::byteswap(function_type::controller)
  };

  const uint32_t command_size = maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf,
                                                                                        data_fields);
  using host_response_type = struct maple::command_response<response_type::data_fields>;
  auto host_response = reinterpret_cast<host_response_type *>(receive_buf);

  maple::dma_start(command_buf, command_size,
                   receive_buf, maple::sizeof_command(host_response));

  buttons.reset();

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((data_fields.function_type & std::byteswap(function_type::controller)) == 0) {
      return;
    }

    buttons.ra |= ft0::data_transfer::digital_button::ra(data_fields.data.digital_button) == 0;
    buttons.la |= ft0::data_transfer::digital_button::la(data_fields.data.digital_button) == 0;
    buttons.da |= ft0::data_transfer::digital_button::da(data_fields.data.digital_button) == 0;
    buttons.ua |= ft0::data_transfer::digital_button::ua(data_fields.data.digital_button) == 0;

    buttons.a |= ft0::data_transfer::digital_button::a(data_fields.data.digital_button) == 0;
    buttons.b |= ft0::data_transfer::digital_button::b(data_fields.data.digital_button) == 0;
    buttons.x |= ft0::data_transfer::digital_button::x(data_fields.data.digital_button) == 0;
    buttons.y |= ft0::data_transfer::digital_button::y(data_fields.data.digital_button) == 0;

    buttons.start |= ft0::data_transfer::digital_button::start(data_fields.data.digital_button) == 0;
  }
}

uint32_t _ta_parameter_buf[((32 * 10 * 17) + 32) / 4];

struct editor_state {
  struct gap_buffer gb;
};

void render_gap_buffer(ta_parameter_writer& parameter,
		       const struct font * font,
		       const struct glyph * glyphs,
		       struct gap_buffer& gb)
{
  int32_t advance = 0;
  int32_t cursor = 0;
  for (int32_t i = 0; i < gb.size; i++) {
    char_type c;

    if (i < gb.gap_start || i >= gb.gap_end) {
      c = gb.buf[i];
    } else {
      c = '_';
    }
    if (i == gb.gap_start)
      cursor = advance;

    uint32_t ix;
    if (c >= font->first_char_code && c <= font->last_char_code) {
      ix = c - font->first_char_code;
    } else {
      ix = '#' - font->first_char_code;
    }
    auto& glyph = glyphs[ix];

    transform(parameter,
	      font->texture_width,
	      font->texture_height,
	      glyph,
	      50.f + float_26_6(advance), // x
	      100.f  // y
	      );

    advance += glyph.metrics.horiAdvance;
  }

  for (int32_t i = 0; i < gb.line.length; i++) {
    int32_t j = gb.line.offsets[i];

    auto& glyph = glyphs['X' - font->first_char_code];
    transform(parameter,
	      font->texture_width,
	      font->texture_height,
	      glyph,
	      50.f + 10.f * j, // x
	      100.f - float_26_6(font->glyph_height) // y
	      );
  }

  auto& glyph = glyphs['^' - font->first_char_code];
  transform(parameter,
	    font->texture_width, font->texture_height,
	    glyph,
	    50.f + float_26_6(cursor), // x
	    100.f + float_26_6(font->glyph_height) // y
	    );

  {
    auto& glyph = glyphs['`' - font->first_char_code];
    int32_t j;
    if (gb.line.gap < gb.line.length) {
      j = gb.line.offsets[gb.line.gap];
    } else {
      j = gb.size;
    }
    transform(parameter,
	      font->texture_width, font->texture_height,
	      glyph,
	      50.f + 10.f * j, // x
	      100.f + float_26_6(font->glyph_height) * 2 // y
	      );
  }

  int32_t h_advance = 0;
  int32_t v_advance = 0;
  for (int32_t i = 0; i < gb.size; i++) {
    char_type c;

    if (i < gb.gap_start || i >= gb.gap_end) {
      c = gb.buf[i];
    } else {
      continue;
    }

    uint32_t ix;
    if (c >= font->first_char_code && c <= font->last_char_code) {
      ix = c - font->first_char_code;
    } else {
      ix = '#' - font->first_char_code;
    }
    auto& glyph = glyphs[ix];

    transform(parameter,
	      font->texture_width,
	      font->texture_height,
	      glyph,
	      350.f + float_26_6(h_advance), // x
	      100.f + float_26_6(v_advance) // y
	      );

    if (c == '\n') {
      h_advance = 0;
      v_advance += font->glyph_height;
    } else {
      h_advance += glyph.metrics.horiAdvance;
    }
  }
}

void update_state(button_state& prev, button_state& next, editor_state& state)
{
  if ((next.la == true) && (next.la != prev.la)) {
    gap_cursor_pos(state.gb, -1);
  }
  if ((next.ra == true) && (next.ra != prev.ra)) {
    gap_cursor_pos(state.gb, +1);
  }
}

void main()
{
  video_output::set_mode_vga();

  auto font = reinterpret_cast<const struct font *>(&_binary_ter_u20n_data_start);
  auto glyphs = reinterpret_cast<const struct glyph *>(&font[1]);
  auto texture = reinterpret_cast<const uint32_t *>(&glyphs[font->glyph_count]);

  inflate_font(texture,
               font->texture_stride,
               font->max_z_curve_ix);
  palette_data<2>();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
                                       , .translucent = 0
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;

  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);
  struct button_state buttons[2] = { 0 };
  struct editor_state state = { 0 };
  //                  01234567
  char_type buf[64] = "abqb\neggs1\nbarley";
  int32_t offsets[64];
  gap_init_from_buf(state.gb, buf, 24, 17);
  line_init_from_buf(state.gb, offsets, 64);
  gap_cursor_pos(state.gb, (-state.gb.gap_start) + 6);
  gap_append(state.gb, '\n');

  while (true) {
    do_get_condition(command_buf, receive_buf, buttons[frame_ix & 1]);
    update_state(buttons[(frame_ix + 1) & 1], buttons[frame_ix & 1], state);

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    render_gap_buffer(parameter, font, glyphs, state.gb);
    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
