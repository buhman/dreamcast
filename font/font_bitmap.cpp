#include <cstdint>

#include "../holly/holly.hpp"
#include "../holly/core_bits.hpp"
#include "../holly/texture_memory_alloc.hpp"
#include "../holly/isp_tsp.hpp"
#include "../holly/ta_parameter.hpp"
#include "../holly/ta_global_parameter.hpp"
#include "../holly/ta_vertex_parameter.hpp"
#include "../memorymap.hpp"
#include "../twiddle.hpp"

#include "../sh7091/serial.hpp"

#include "font_bitmap.hpp"

namespace font_bitmap {

static inline void inflate_character(const uint32_t pitch,
                                     const uint32_t width,
                                     const uint32_t height,
                                     const uint32_t texture_width,
                                     const uint32_t texture_height,
                                     const uint8_t * src,
                                     const uint8_t c)
{
  const uint32_t character_index = c - ' ';
  const uint32_t offset = pitch * height * character_index;
  uint8_t temp[texture_width * texture_height];

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint8_t row = src[offset + (y * pitch) + (x / 8)];
      uint8_t px = (row >> (7 - (x % 8))) & 1;
      //serial::character((px == 1) ? 'X' : '_');
      uint16_t palette_index = px ? 2 : 1;
      temp[y * texture_width + x] = palette_index;
    }
    for (uint32_t x = width; x < texture_width; x++) {
      temp[y * texture_width + x] = 1;
    }
    //serial::character('\n');
  }
  for (uint32_t y = height; y < texture_height; y++) {
    for (uint32_t x = 0; x < texture_width; x++) {
      temp[y * texture_width + x] = 1;
    }
  }

  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory64);
  auto texture = reinterpret_cast<volatile uint32_t *>(mem->texture);

  const uint32_t texture_offset = texture_width * texture_height * character_index / 2;
  twiddle::texture2<4>(&texture[texture_offset / 4], // uint32_t *
                       temp,
		       texture_width,
		       texture_width * texture_height);
}

void inflate(const uint32_t pitch,
             const uint32_t width,
             const uint32_t height,
             const uint32_t texture_width,
             const uint32_t texture_height,
             const uint8_t * src)
{
  for (uint8_t ix = 0x20; ix < 0x7f; ix++) {
    inflate_character(pitch,
                      width,
                      height,
                      texture_width,
                      texture_height,
                      src,
                      ix);
  }
}

void palette_data()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  holly.PALETTE_RAM[1] = 0x0000;
  holly.PALETTE_RAM[2] = 0xffff;
}

struct quad_vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

const struct quad_vertex quad_vertices[4] = {
  // [ position       ]  [ uv coordinates ]
  { -0.5f,   0.5f,  0.f, 0.f, 1.f, },
  { -0.5f,  -0.5f,  0.f, 0.f, 0.f, },
  {  0.5f,   0.5f,  0.f, 1.f, 1.f, },
  {  0.5f,  -0.5f,  0.f, 1.f, 0.f, },
};
constexpr uint32_t quad_length = (sizeof (quad_vertices)) / (sizeof (struct quad_vertex));

void transform_string(ta_parameter_writer& parameter,
                      const uint32_t texture_width,
                      const uint32_t texture_height,
                      const uint32_t glyph_width,
                      const uint32_t glyph_height,
                      const int32_t position_x,
                      const int32_t position_y,
                      const char * s,
                      const uint32_t len
                      )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(texture_width)
                                      | tsp_instruction_word::texture_v_size::from_int(texture_height);

  for (uint32_t string_ix = 0; string_ix < len; string_ix++) {
    const uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));
    const uint32_t glyph_address = texture_address + texture_width * texture_height * (s[string_ix] - ' ') / 2;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
					| texture_control_word::scan_order::twiddled
					| texture_control_word::texture_address(glyph_address / 8);

    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  texture_control_word,
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );

    for (uint32_t i = 0; i < quad_length; i++) {
      bool end_of_strip = i == quad_length - 1;

      float x = quad_vertices[i].x;
      float y = quad_vertices[i].y;
      float z = quad_vertices[i].z;
      float u = quad_vertices[i].u;
      float v = quad_vertices[i].v;

      x *= static_cast<float>(glyph_width * 1);
      y *= static_cast<float>(glyph_height * 1);
      x += static_cast<float>(position_x + glyph_width * 4 * string_ix);
      y += static_cast<float>(position_y);
      z = 1.f / (z + 10.f);

      u *= static_cast<float>(glyph_width - 1) / static_cast<float>(texture_width);
      v *= static_cast<float>(glyph_height - 1) / static_cast<float>(texture_height);

      parameter.append<ta_vertex_parameter::polygon_type_3>() =
	ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
					    x, y, z,
                                            u, v,
					    0, // base_color
					    0  // offset_color
					    );
    }
  }
}

}
