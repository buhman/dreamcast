#include "transform.hpp"

#include "holly/isp_tsp.hpp"
#include "holly/texture_memory_alloc3.hpp"
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

static inline void transfer_quad_type_0(ta_parameter_writer& writer,
                                        float ax, float ay,
                                        float bx, float by,
                                        float cx, float cy,
                                        float dx, float dy,
                                        float z)
{
  uint32_t base_color = 0xffffffff;

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        ax, ay, z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        bx, by, z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        dx, dy, z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        cx, cy, z,
                                        base_color);
}

void transform_cursor(ta_parameter_writer& writer,
		      const float origin_x,
		      const float origin_y,
		      const float width,
		      const float height)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
					| obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
					  | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::inverse_other_color
				      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;


  const uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  const float z = 0.1f;

  transfer_quad_type_0(writer,
                       strip_vertices[0].x * width  + origin_x,
                       strip_vertices[0].y * height + origin_y,
                       strip_vertices[1].x * width  + origin_x,
                       strip_vertices[1].y * height + origin_y,
                       strip_vertices[2].x * width  + origin_x,
                       strip_vertices[2].y * height + origin_y,
                       strip_vertices[3].x * width  + origin_x,
                       strip_vertices[3].y * height + origin_y,
                       z
                       );
}

void glyph_begin(ta_parameter_writer& writer,
                 const uint32_t texture_width, uint32_t texture_height)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
					| obj_control::col_type::packed_color
					| obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::always
					  | isp_tsp_instruction_word::culling_mode::no_culling;


  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
				      | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
				      | tsp_instruction_word::fog_control::no_fog
				      | tsp_instruction_word::texture_u_size::from_int(texture_width)
				      | tsp_instruction_word::texture_v_size::from_int(texture_height);

  const uint32_t texture_address = texture_memory_alloc.texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
				      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::palette_selector8(0)
				      | texture_control_word::texture_address(texture_address / 8);

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

static inline void transfer_quad_type_1(ta_parameter_writer& writer,
                                        struct vertex va, struct vertex vb, struct vertex vc, struct vertex vd, float z)
{
  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        va.x, va.y, z,
                                        va.u, va.v,
                                        0,  // base_color
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        vb.x, vb.y, z,
                                        vb.u, vb.v,
                                        0,  // base_color
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        vd.x, vd.y, z,
                                        vd.u, vd.v,
                                        0,  // base_color
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(true),
                                        vc.x, vc.y, z,
                                        vc.u, vc.v,
                                        0,  // base_color
                                        0); // offset_color
}

void transform_glyph(ta_parameter_writer& writer,
		     const float r_texture_width,
                     const float r_texture_height,
		     const glyph& glyph,
		     const float origin_x,
		     const float origin_y)
{
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
    u = u * r_texture_width;
    v = v * r_texture_height;

    out[i].x = x;
    out[i].y = y;
    out[i].u = u;
    out[i].v = v;
  }

  const float z = 0.1f;
  transfer_quad_type_1(writer, out[0], out[1], out[2], out[3], z);
}
