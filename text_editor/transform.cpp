#include "transform.hpp"

#include "holly/isp_tsp.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"

constexpr inline float float_26_6(int32_t n)
{
  float v = n >> 6;
//float d = n & 63;
//return v + (d / 64.f);
  return v;
}

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

void transform_sprite(ta_parameter_writer& parameter,
		      const float origin_x,
		      const float origin_y,
		      const float width,
		      const float height)
{
  const uint32_t parameter_control_word = para_control::para_type::sprite
                                        | para_control::list_type::translucent
					| obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
					  | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::inverse_other_color
				      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  constexpr uint32_t base_color = 0xffffffff;
  parameter.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
				isp_tsp_instruction_word,
				tsp_instruction_word,
				0, // texture_control_word
				base_color,
				0, // offset_color
				0, // data_size_for_sort_dma
				0  // next_address_for_sort_dma
				);

  const float z = 0.1f;
  parameter.append<ta_vertex_parameter::sprite_type_0>() =
    ta_vertex_parameter::sprite_type_0(para_control::para_type::vertex_parameter,
				       strip_vertices[0].x * width  + origin_x,
				       strip_vertices[0].y * height + origin_y,
				       z,
				       strip_vertices[1].x * width  + origin_x,
				       strip_vertices[1].y * height + origin_y,
				       z,
				       strip_vertices[2].x * width  + origin_x,
				       strip_vertices[2].y * height + origin_y,
				       z,
				       strip_vertices[3].x * width  + origin_x,
				       strip_vertices[3].y * height + origin_y
				       );
}

void transform_glyph(ta_parameter_writer& parameter,
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

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
					  | isp_tsp_instruction_word::culling_mode::no_culling;


  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
				      | tsp_instruction_word::dst_alpha_instr::zero
				      | tsp_instruction_word::fog_control::no_fog
				      | tsp_instruction_word::texture_u_size::from_int(texture_width)
				      | tsp_instruction_word::texture_v_size::from_int(texture_height);

  const uint32_t texture_address = texture_memory_alloc::texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
				      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::palette_selector(0)
				      | texture_control_word::texture_address(texture_address / 8);

  parameter.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
				isp_tsp_instruction_word,
				tsp_instruction_word,
				texture_control_word,
				0, // base_color
				0, // offset_color
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
