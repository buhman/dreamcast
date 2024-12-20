#include <cstdint>
#include <bit>

#include "align.hpp"

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
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/video_output.hpp"
#include "memorymap.hpp"
#include "twiddle.hpp"
#include "palette.hpp"

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"

#include "sh7091/serial.hpp"

#include "texture/bbb/bbb1024.data.h"
#include "texture/bbb/bbb128.data.h"
#include "texture/bbb/bbb16.data.h"
#include "texture/bbb/bbb1.data.h"
#include "texture/bbb/bbb256.data.h"
#include "texture/bbb/bbb2.data.h"
#include "texture/bbb/bbb32.data.h"
#include "texture/bbb/bbb4.data.h"
#include "texture/bbb/bbb512.data.h"
#include "texture/bbb/bbb64.data.h"
#include "texture/bbb/bbb8.data.h"
#include "texture/bbb/bbb.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t color;
};

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv coordinates       ]  [color   ]
  { -0.5f,   0.5f,  0.f, 0.f        , 127.f/128.f, 0xffffffff}, // the first two base colors in a
  { -0.5f,  -0.5f,  0.f, 0.f        , 0.f        , 0xffffffff}, // non-Gouraud triangle strip are ignored
  {  0.5f,   0.5f,  0.f, 127.f/128.f, 127.f/128.f, 0xffffffff},
  {  0.5f,  -0.5f,  0.f, 127.f/128.f, 0.f        , 0xffffffff},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

constexpr float half_degree = 0.01745329f / 2.f;

namespace defaults {
  constexpr uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                            | obj_control::col_type::packed_color
                                            | obj_control::texture;

  constexpr uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                              | isp_tsp_instruction_word::culling_mode::no_culling;

  constexpr uint32_t tsp_instruction_word = tsp_instruction_word::src_select::primary_accumulation_buffer
                                          | tsp_instruction_word::dst_select::primary_accumulation_buffer
                                          | tsp_instruction_word::fog_control::no_fog;

  constexpr uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                          | texture_control_word::scan_order::twiddled;
}

void append_point_sampled(ta_parameter_writer& parameter, uint32_t texture_control_word, uint32_t size)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::point_sampled
				      | tsp_instruction_word::texture_u_size::from_int(size)
				      | tsp_instruction_word::texture_v_size::from_int(size);

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

void append_bilinear(ta_parameter_writer& parameter, uint32_t texture_control_word, uint32_t size)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::bilinear_filter
    				      | tsp_instruction_word::texture_u_size::from_int(size)
				      | tsp_instruction_word::texture_v_size::from_int(size);

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

void append_trilinear_pass_a(ta_parameter_writer& parameter, uint32_t texture_control_word, uint32_t size)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::trilinear_pass_a
    				      | tsp_instruction_word::texture_u_size::from_int(size)
				      | tsp_instruction_word::texture_v_size::from_int(size);

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

void append_trilinear_pass_b(ta_parameter_writer& parameter, uint32_t texture_control_word, uint32_t size)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::translucent;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::one
                                      | tsp_instruction_word::filter_mode::trilinear_pass_b
    				      | tsp_instruction_word::texture_u_size::from_int(size)
				      | tsp_instruction_word::texture_v_size::from_int(size);

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

uint8_t const * const mips[] = {
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb1_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb2_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb4_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb8_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb16_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb32_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb64_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb128_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb256_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb512_data_start),
  reinterpret_cast<uint8_t *>(&_binary_texture_bbb_bbb1024_data_start)
};

void _copy_bbb_texture(uint32_t dst_offset, uint8_t const * const src, uint32_t mip)
{
  auto area  = mip * mip;
  uint16_t temp[area];
  for (uint32_t px = 0; px < area; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    temp[px] = rgb565;
  }

  auto texture = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);
  auto dst = &texture[dst_offset / 2];
  twiddle::texture(dst, temp, mip, mip);
}

uint32_t copy_bbb_texture(uint32_t top)
{
  uint32_t dst_offset = top + 6;
  uint32_t ix = 0;
  for (uint32_t mip = 1; mip <= 1024; mip *= 2) {
    _copy_bbb_texture(dst_offset, mips[ix], mip);
    ix += 1;
    dst_offset += mip * mip * 2;
  }
  return dst_offset;
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
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::controller);

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  buttons.reset();

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((std::byteswap(data_fields.function_type) & function_type::controller) == 0) {
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

uint32_t _ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4];

struct mipmap_state {
  enum type : int {
    disabled,
    enabled
  };
};

static char const * const mipmap_state_names[] = {
  [mipmap_state::disabled] = "mipmap disabled",
  [mipmap_state::enabled] = "mipmap enabled"
};

struct filter_state {
  enum type : int {
    point_sampled,
    bilinear,
    trilinear_a_b,
    trilinear_a,
    trilinear_b
  };
};

static char const * const filter_state_names[] = {
  [filter_state::point_sampled] = "point sampled",
  [filter_state::bilinear] = "bilinear",
  [filter_state::trilinear_a_b] = "trilinear pass A and B",
  [filter_state::trilinear_a] = "trilinear pass A only",
  [filter_state::trilinear_b] = "trilinear pass B only"
};

struct demo_state {
  mipmap_state::type mipmap;
  filter_state::type filter;
  float y_axis_rotate;
  float scale;
  int32_t size;
};

bool has_translucent(filter_state::type filter)
{
  switch (filter) {
  default: [[fallthrough]];
  case filter_state::point_sampled: return false;
  case filter_state::bilinear: return false;
  case filter_state::trilinear_a_b: return true;
  case filter_state::trilinear_a: return false;
  case filter_state::trilinear_b: return true;
  }
}

bool has_opaque(filter_state::type filter)
{
  switch (filter) {
  default: [[fallthrough]];
  case filter_state::point_sampled: return true;
  case filter_state::bilinear: return true;
  case filter_state::trilinear_a_b: return true;
  case filter_state::trilinear_a: return true;
  case filter_state::trilinear_b: return false;
  }
}

uint32_t get_ta_alloc(filter_state::type filter)
{
  return
      ta_alloc_ctrl::pt_opb::_16x4byte
    | ta_alloc_ctrl::tm_opb::no_list
    | (has_translucent(filter) ? ta_alloc_ctrl::t_opb::_16x4byte : ta_alloc_ctrl::t_opb::no_list)
    | ta_alloc_ctrl::om_opb::no_list
    | (has_opaque(filter)      ? ta_alloc_ctrl::o_opb::_16x4byte : ta_alloc_ctrl::o_opb::no_list)
    ;
}

struct opb_size get_opb_size(filter_state::type filter)
{
  return
    { .opaque = has_opaque(filter) ? 16u * 4 : 0
    , .opaque_modifier = 0
    , .translucent = has_translucent(filter) ? 16u * 4 : 0
    , .translucent_modifier = 0
    , .punch_through = 16 * 4
    };
}

enum struct filter_type {
  point_sampled,
  bilinear,
  trilinear_pass_a,
  trilinear_pass_b,
};

const uint32_t mip_offset[] = {
  0x00006, // 1
  0x00008, // 2
  0x00010, // 4
  0x00030, // 8
  0x000b0, // 16
  0x002b0, // 32
  0x00ab0, // 64
  0x02ab0, // 128
  0x0aab0, // 256
  0x2aab0, // 512
  0xaaab0, // 1024
};

const char * texture_size_names[] = {
  "1x1",
  "2x2",
  "4x4",
  "8x8",
  "16x16",
  "32x32",
  "64x64",
  "128x128",
  "256x256",
  "512x512",
  "1024x1024"
};

uint32_t get_texture_control_word(const uint32_t bbb_offset, mipmap_state::type mipmap, int32_t size)
{
  const uint32_t texture_address = texture_memory_alloc::texture.start + bbb_offset;

  switch (mipmap) {
  default: [[fallthrough]];
  case mipmap_state::enabled:
    return defaults::texture_control_word
         | texture_control_word::texture_address(texture_address / 8)
         | texture_control_word::mip_mapped;

  case mipmap_state::disabled:
    return defaults::texture_control_word
         | texture_control_word::texture_address((texture_address + mip_offset[size]) / 8);
  }
}

void transform(ta_parameter_writer& parameter,
               const vertex * strip_vertices,
               const uint32_t strip_length,
               const enum filter_type filter,
	       const demo_state& state,
	       uint32_t bbb_offset
               )
{
  const uint32_t texture_control_word = get_texture_control_word(bbb_offset, state.mipmap, state.size);

  switch (filter) {
  case filter_type::point_sampled:    append_point_sampled(   parameter, texture_control_word, (1 << state.size)); break;
  case filter_type::bilinear:         append_bilinear(        parameter, texture_control_word, (1 << state.size)); break;
  case filter_type::trilinear_pass_a: append_trilinear_pass_a(parameter, texture_control_word, (1 << state.size)); break;
  case filter_type::trilinear_pass_b: append_trilinear_pass_b(parameter, texture_control_word, (1 << state.size)); break;
  }

  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float x1;

    x1 = x * __builtin_cosf(state.y_axis_rotate) - z * __builtin_sinf(state.y_axis_rotate);
    z  = x * __builtin_sinf(state.y_axis_rotate) + z * __builtin_cosf(state.y_axis_rotate);
    x  = x1;

    x *= state.scale;
    y *= state.scale;
    x += 320.f;
    y += 240.f;
    z = 1.f / (z + 10.f);

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          strip_vertices[i].u,
                                          strip_vertices[i].v,
                                          strip_vertices[i].color,
                                          0 // offset_color
                                          );
  }

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void update_state(button_state& prev, button_state& next, demo_state& state)
{
  /* key bindings:

    x: previous texture size
    y: next texture size

    a: previous filter state
    b: next filter state

    up: enlarge scale
    down: reduce scale
    left: rotate Y axis -
    right: rotate Y axis +

    start: toggle mipmap
 */

  if ((next.start == true) && (next.start != prev.start)) {
    if (state.mipmap == mipmap_state::disabled) state.mipmap = mipmap_state::enabled;
    else state.mipmap = mipmap_state::disabled;
  }
  if ((next.x == true) && (next.x != prev.x)) {
    state.size -= 1;
    if (state.size < 3) state.size = 10;
  }
  if ((next.y == true) && (next.y != prev.y)) {
    state.size += 1;
    if (state.size > 10) state.size = 3;
  }
  if ((next.a == true) && (next.a != prev.a)) {
    state.filter = static_cast<filter_state::type>(state.filter - 1);
    if (state.filter < filter_state::point_sampled) state.filter = filter_state::trilinear_b;
  }
  if ((next.b == true) && (next.b != prev.b)) {
    state.filter = static_cast<filter_state::type>(state.filter + 1);
    if (state.filter > filter_state::trilinear_b) state.filter = filter_state::point_sampled;
  }

  if (next.la == true) {
    state.y_axis_rotate -= half_degree;
  }
  if (next.ra == true) {
    state.y_axis_rotate += half_degree;
  }
  if (next.ua == true) {
    state.scale += 1.0f;
  }
  if (next.da == true) {
    state.scale -= 1.0f;
  }
}

void transform_hud(ta_parameter_writer& parameter, const demo_state& state)
{
  int row = 0;
  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 2 * 8, // position x
                                16 + row * 16, // position y
                                "(X/Y)", 5,
				para_control::list_type::punch_through);

  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 8 * 8, // position x
                                16 + row * 16, // position y
                                texture_size_names[state.size], -1,
				para_control::list_type::punch_through);
  row++;

  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 2 * 8, // position x
                                16 + row * 16, // position y
                                "(A/B)", 5,
				para_control::list_type::punch_through);

  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 8 * 8, // position x
                                16 + row * 16, // position y
                                filter_state_names[state.filter], -1,
				para_control::list_type::punch_through);
  row++;

  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 0 * 8, // position x
                                16 + row * 16, // position y
                                "(start)", 7,
				para_control::list_type::punch_through);

  font_bitmap::transform_string(parameter,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 8 * 8, // position x
                                16 + row * 16, // position y
                                mipmap_state_names[state.mipmap], -1,
				para_control::list_type::punch_through);
  row++;

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void main()
{
  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  holly.PT_ALPHA_REF = 0x1;
  background_parameter(0xff00ff00);

  auto src = reinterpret_cast<const uint8_t *>(&_binary_font_verite_8x16_verite_8x16_data_start);
  uint32_t bbb_offset = font_bitmap::inflate(1,  // pitch
					     8,  // width
					     16, // height
					     8,  // texture_width
					     16, // texture_height
					     src);
  palette_data<3>();

  copy_bbb_texture(bbb_offset);

  video_output::set_mode_vga();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  uint32_t frame_ix = 0;

  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);
  struct button_state buttons[2] = { 0 };

  struct demo_state state = {
    .mipmap = mipmap_state::disabled,
    .filter = filter_state::point_sampled,
    .y_axis_rotate = 0.f,
    .scale = 512.f,
    .size = 10, // 1024
  };
  filter_state::type last_filter_state = static_cast<filter_state::type>(state.filter + 1);
  uint32_t ta_alloc = 0;
  struct opb_size opb_size;

  while (true) {
    do_get_condition(command_buf, receive_buf, buttons[frame_ix & 1]);
    update_state(buttons[(frame_ix + 1) & 1], buttons[frame_ix & 1], state);

    if (state.filter != last_filter_state) {
      ta_alloc = get_ta_alloc(state.filter);
      opb_size = get_opb_size(state.filter);
      region_array2(640 / 32, 480 / 32, opb_size);
      last_filter_state = state.filter;
    }

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    switch (state.filter) {
    case filter_state::point_sampled:
      {
	auto parameter_o = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_o, strip_vertices, strip_length, filter_type::point_sampled, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_o.offset);
	ta_wait_opaque_list();

	break;
      }
    case filter_state::bilinear:
      {
	auto parameter_o = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_o, strip_vertices, strip_length, filter_type::bilinear, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_o.offset);
	ta_wait_opaque_list();

	break;
      }
    case filter_state::trilinear_a_b:
      {
	auto parameter_o = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_o, strip_vertices, strip_length, filter_type::trilinear_pass_a, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_o.offset);
	ta_wait_opaque_list();

	auto parameter_t = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_t, strip_vertices, strip_length, filter_type::trilinear_pass_b, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_t.offset);
	ta_wait_translucent_list();

	break;
      }
    case filter_state::trilinear_a:
      {
	auto parameter_o = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_o, strip_vertices, strip_length, filter_type::trilinear_pass_a, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_o.offset);
	ta_wait_opaque_list();

	break;
      }
    case filter_state::trilinear_b:
      {
	auto parameter_t = ta_parameter_writer(ta_parameter_buf);
	transform(parameter_t, strip_vertices, strip_length, filter_type::trilinear_pass_b, state, bbb_offset);
	ta_polygon_converter_transfer(ta_parameter_buf, parameter_t.offset);
	ta_wait_translucent_list();

	break;
      }
    }

    auto parameter_pt = ta_parameter_writer(ta_parameter_buf);
    transform_hud(parameter_pt, state);
    ta_polygon_converter_transfer(ta_parameter_buf, parameter_pt.offset);
    ta_wait_punch_through_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
