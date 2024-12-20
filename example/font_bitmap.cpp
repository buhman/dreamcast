#include <cstdint>

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

#include "font/sperrypc/sperrypc_8x8.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

/*
// screen space coordinates
const struct vertex quad_vertices[4] = {
  { 0.f,  64.f,  0.01f, 0.f, 1.f  },
  { 0.f,  0.f,   0.01f, 0.f, 0.f  },
  { 64.f, 0.f,   0.01f, 1.f, 0.f  },
  { 64.f, 64.f,  0.01f, 1.f, 1.f, },
};

uint32_t transform(uint32_t * ta_parameter_buf)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));
  constexpr uint32_t base_color = 0xffffffff;
  auto sprite = global_sprite(base_color);
  sprite.parameter_control_word = para_control::para_type::sprite
                                | para_control::list_type::opaque
                                | obj_control::col_type::packed_color
                                | obj_control::texture
                                | obj_control::_16bit_uv;
  sprite.tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			      | tsp_instruction_word::dst_alpha_instr::zero
			      | tsp_instruction_word::fog_control::no_fog
			      | tsp_instruction_word::texture_u_size::_8   // 8px
			      | tsp_instruction_word::texture_v_size::_8;  // 8px
  sprite.texture_control_word = texture_control_word::pixel_format::_565
			      | texture_control_word::scan_order::twiddled
			      | texture_control_word::texture_address(texture_address / 8);
  parameter.append<global_sprite>() = sprite;

  parameter.append<vertex_sprite_type_1>() =
    vertex_sprite_type_1(quad_vertices[0].x,
			 quad_vertices[0].y,
			 quad_vertices[0].z,
			 quad_vertices[1].x,
			 quad_vertices[1].y,
			 quad_vertices[1].z,
			 quad_vertices[2].x,
			 quad_vertices[2].y,
			 quad_vertices[2].z,
			 quad_vertices[3].x,
			 quad_vertices[3].y,
			 uv_16bit(quad_vertices[0].u, quad_vertices[0].v),
			 uv_16bit(quad_vertices[1].u, quad_vertices[1].v),
			 uv_16bit(quad_vertices[2].u, quad_vertices[2].v));
  // curiously, there is no `dz` in vertex_sprite_type_1
  // curiously, there is no `du_dv` in vertex_sprite_type_1

  parameter.append<global_end_of_list>() = global_end_of_list();

  return parameter.offset;
}
*/

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv coordinates ]
  { -0.5f,   0.5f,  0.f, 0.f, 1.f, },
  { -0.5f,  -0.5f,  0.f, 0.f, 0.f, },
  {  0.5f,   0.5f,  0.f, 1.f, 1.f, },
  {  0.5f,  -0.5f,  0.f, 1.f, 0.f, },
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

uint32_t transform(uint32_t * ta_parameter_buf, const char * s, const uint32_t len)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);

  for (uint32_t string_ix = 0; string_ix < len; string_ix++) {
    const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
					  | para_control::list_type::opaque
					  | obj_control::col_type::packed_color
					  | obj_control::texture;

    const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
					    | isp_tsp_instruction_word::culling_mode::no_culling;

    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
					| tsp_instruction_word::dst_alpha_instr::zero
					| tsp_instruction_word::fog_control::no_fog
					| tsp_instruction_word::texture_u_size::from_int(8)
					| tsp_instruction_word::texture_v_size::from_int(8);

    const uint32_t character_offset = ((8 * 8) / 2) * (s[string_ix] - ' ');
    const uint32_t texture_address = texture_memory_alloc::texture.start;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
					| texture_control_word::scan_order::twiddled
                                        | texture_control_word::texture_address((texture_address + character_offset) / 8);

    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  texture_control_word,
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );

    for (uint32_t i = 0; i < strip_length; i++) {
      bool end_of_strip = i == strip_length - 1;

      float x = strip_vertices[i].x;
      float y = strip_vertices[i].y;
      float z = strip_vertices[i].z;

      x *= 8.f;
      y *= 8.f;
      x += 64.f + 8 * string_ix;
      y += 240.f;
      z = 1.f / (z + 10.f);

      parameter.append<ta_vertex_parameter::polygon_type_3>() =
	ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
					    x, y, z,
					    strip_vertices[i].u,
					    strip_vertices[i].v,
					    0, // base_color
					    0  // offset_color
					    );
    }
  }

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  return parameter.offset;
}


void init_texture_memory(const struct opb_size& opb_size)
{
  background_parameter(0xff0000ff);

  region_array2(640 / 32, 480 / 32, opb_size);
}

inline void inflate_character(const uint8_t * src, const uint8_t c)
{
  uint8_t character_index = c - ' ';

  uint8_t temp[8 * 8];
  for (uint32_t y = 0; y < 8; y++) {
    uint8_t row = src[y + 8 * character_index];
    for (uint32_t x = 0; x < 8; x++) {
      uint8_t px = (row >> (7 - x)) & 1;
      //serial::character((px == 1) ? 'X' : '_');
      //uint16_t rgb565 = px ? 0xffff : 0;
      uint16_t palette_index = px ? 2 : 1;

      temp[y * 8 + x] = palette_index;
    }
    //serial::character('\n');
  }

  auto texture = reinterpret_cast<volatile uint32_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);

  uint32_t offset = ((8 * 8) / 2) * character_index;

  union {
    uint8_t  u8[8 * 8];
    uint32_t u32[8 * 8 / 4];
  } temp2;

  twiddle::texture_4bpp(temp2.u8, temp, 8, 8);
  for (uint32_t i = 0; i < 8 * 8 / 4; i++) {
    texture[(offset / 4) + i] = temp2.u32[i];
  }
}

void inflate_font(const uint8_t * src)
{
  for (uint8_t ix = 0x20; ix < 0x7f; ix++) {
    inflate_character(src, ix);
  }
}

void palette_data()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  holly.PALETTE_RAM[1] = (15) << 11;
  holly.PALETTE_RAM[2] = (15 << 11) | (30 << 5);
}

uint32_t _ta_parameter_buf[((32 * 10 * 17) + 32) / 4];

void main()
{
  video_output::set_mode_vga();

  auto src = reinterpret_cast<const uint8_t *>(&_binary_font_sperrypc_sperrypc_8x8_data_start);
  inflate_font(src);
  palette_data();

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

  const char ana[18] = "A from ana i know";

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);
    uint32_t ta_parameter_size = transform(ta_parameter_buf, ana, 17);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
