#include <cstdint>

#include "align.hpp"

#include "vga.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "memorymap.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "twiddle.hpp"
#include "serial.hpp"

#include "sperrypc.hpp"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

/*
// screen space coordinates
const struct vertex quad_verticies[4] = {
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
    vertex_sprite_type_1(quad_verticies[0].x,
			 quad_verticies[0].y,
			 quad_verticies[0].z,
			 quad_verticies[1].x,
			 quad_verticies[1].y,
			 quad_verticies[1].z,
			 quad_verticies[2].x,
			 quad_verticies[2].y,
			 quad_verticies[2].z,
			 quad_verticies[3].x,
			 quad_verticies[3].y,
			 uv_16bit(quad_verticies[0].u, quad_verticies[0].v),
			 uv_16bit(quad_verticies[1].u, quad_verticies[1].v),
			 uv_16bit(quad_verticies[2].u, quad_verticies[2].v));
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
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));

  for (uint32_t string_ix = 0; string_ix < len; string_ix++) {
    auto polygon = global_polygon_type_0(texture_address);
    polygon.parameter_control_word = para_control::para_type::polygon_or_modifier_volume
      | para_control::list_type::opaque
      | obj_control::col_type::packed_color
      | obj_control::texture;

    polygon.tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
      | tsp_instruction_word::dst_alpha_instr::zero
      | tsp_instruction_word::fog_control::no_fog
      | tsp_instruction_word::texture_u_size::_8   // 8px
      | tsp_instruction_word::texture_v_size::_8;  // 8px

    polygon.texture_control_word = texture_control_word::pixel_format::_8bpp_palette
      | texture_control_word::scan_order::twiddled
      | texture_control_word::texture_address((texture_address + 8 * 8 * (s[string_ix] - ' ')) / 8);
    parameter.append<global_polygon_type_0>() = polygon;

    for (uint32_t i = 0; i < strip_length; i++) {
      bool end_of_strip = i == strip_length - 1;

      float x = strip_vertices[i].x;
      float y = strip_vertices[i].y;
      float z = strip_vertices[i].z;

      x *= 32.f;
      y *= 32.f;
      x += 64.f + 32 * string_ix;
      y += 240.f;
      z = 1.f / (z + 10.f);

      parameter.append<vertex_polygon_type_3>() =
	vertex_polygon_type_3(x, y, z,
			      strip_vertices[i].u,
			      strip_vertices[i].v,
			      0x00000000, // base_color
			      end_of_strip);
    }
  }

  parameter.append<global_end_of_list>() = global_end_of_list();

  return parameter.offset;
}


void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background);

  region_array2(mem->region_array,
	        (offsetof (struct texture_memory_alloc, object_list)),
		640 / 32, // width
		480 / 32, // height
		opb_size
		);
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

  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory64);
  auto texture = reinterpret_cast<volatile uint32_t *>(mem->texture);

  uint32_t offset = 8 * 8 * character_index;
  union {
    uint8_t  u8[8 * 8];
    uint32_t u32[8 * 8 / 4];
  } temp2;

  //twiddle::texture(&texture[offset], temp, 8, 8);
  twiddle::texture(temp2.u8, temp, 8, 8);
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
  vga();

  auto src = reinterpret_cast<const uint8_t *>(&_binary_sperrypc_data_start);
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

  constexpr uint32_t tiles = (640 / 32) * (320 / 32);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;
  constexpr uint32_t num_frames = 1;

  const char ana[18] = "A from ana i know";

  while (true) {
    ta_polygon_converter_init(opb_size.total() * tiles, ta_alloc);
    uint32_t ta_parameter_size = transform(ta_parameter_buf, ana, 17);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render(frame_ix, num_frames);

    v_sync_out();
    v_sync_in();
    core_wait_end_of_render_video(frame_ix, num_frames);

    frame_ix++;
  }
}
