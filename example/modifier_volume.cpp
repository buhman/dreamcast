#include <cstdint>

#include "align.hpp"
#include "holly/video_output.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/isp_tsp.hpp"
#include "memorymap.hpp"

#include "geometry/plane.hpp"
#include "geometry/cube.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

vec3 _transform(const vec3& point,
                const uint32_t scale,
                const float theta)
{
  float x = point.x;
  float y = point.y;
  float z = point.z;
  float t;

  // object transform
  t  = z * cos(theta) - x * sin(theta);
  x  = z * sin(theta) + x * cos(theta);
  z  = t;

  x *= scale;
  y *= scale;
  z *= scale;

  // world transform
  y += 2.0f;
  x *= 0.8;
  y *= 0.8;
  z *= 0.8;

  // camera transform
  z += 4;

  // perspective
  x = x / z;
  y = y / z;

  // screen space transform
  x *= 240.f;
  y *= 240.f;
  x += 320.f;
  y += 240.f;
  z = 1 / z;

  return {x, y, z};
}

void transform_polygon(ta_parameter_writer& parameter,
                       const vec3 * vertices,
                       const face& face,
                       const float scale,
                       const vec4& color,
                       const float theta)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::floating_color
                                        | obj_control::shadow;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
					isp_tsp_instruction_word,
					tsp_instruction_word,
					0, // texture_control_word
					0, // data_size_for_sort_dma
					0  // next_address_for_sort_dma
					);

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    // world transform
    uint32_t vertex_ix = face[i].vertex;
    auto& vertex = vertices[vertex_ix];
    auto point = _transform(vertex, scale, theta);

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_1>() =
      ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					  point.x,
					  point.y,
					  point.z,
					  color.a, // alpha
					  color.r, // red
					  color.g, // green
					  color.b  // blue
					  );
  }
}

void transform_modifier_volume(ta_parameter_writer& parameter,
                               const vec3 * vertices,
                               const face * faces,
                               const uint32_t num_faces,
                               const float scale)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque_modifier_volume
  //                                    | group_control::group_en
  //                                    | group_control::user_clip::inside_enable
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  parameter.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(parameter_control_word,
					 isp_tsp_instruction_word
					 );

  for (uint32_t i = 0; i < num_faces; i++) {
    // world transform
    uint32_t ix_a = faces[i][0].vertex;
    uint32_t ix_b = faces[i][1].vertex;
    uint32_t ix_c = faces[i][2].vertex;
    auto& _a = vertices[ix_a];
    auto& _b = vertices[ix_b];
    auto& _c = vertices[ix_c];
    auto a = _transform(_a, scale, 0.f);
    auto b = _transform(_b, scale, 0.f);
    auto c = _transform(_c, scale, 0.f);

    if (i == (num_faces - 1)) {
      const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                                 | para_control::list_type::opaque_modifier_volume
                                                 | obj_control::volume::modifier_volume::last_in_volume;

      const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                                   | isp_tsp_instruction_word::culling_mode::no_culling;

      parameter.append<ta_global_parameter::modifier_volume>() =
        ta_global_parameter::modifier_volume(last_parameter_control_word,
					     last_isp_tsp_instruction_word);

    }

    parameter.append<ta_vertex_parameter::modifier_volume>() =
      ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					   a.x, a.y, a.z,
					   b.x, b.y, b.z,
					   c.x, c.y, c.z);
  }
}

void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background, 0xff220000);

  region_array2(mem->region_array,
	        (offsetof (struct texture_memory_alloc, object_list)),
		640 / 32, // width
		480 / 32, // height
		opb_size
		);
}

uint32_t _ta_parameter_buf[((32 * 8192) + 32) / 4];

void main()
{
  video_output::set_mode_vga();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::no_list
			      | ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
				       , .opaque_modifier = 16 * 4
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
  constexpr uint32_t num_frames = 1;

  float theta = 0;

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);
    auto parameter = ta_parameter_writer(ta_parameter_buf);
    { // plane
      vec4 color = {1.0, 0.9, 0.4, 0.2};
      float scale = 2.f;
      for (uint32_t i = 0; i < plane::num_faces; i++) {
        transform_polygon(parameter,
                          plane::vertices,
                          plane::faces[i],
                          scale,
                          color,
                          theta);
      }

      /*
      for (uint32_t i = 0; i < cube::num_faces; i++) {
        transform_polygon(parameter,
                          cube::vertices,
                          cube::faces[i],
                          1.f,
                          {1.0f, 0.0f, 1.0f, 0.0f});
      }
      */
    }
    // end of opaque list
    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    { // cube
      float scale = 1.f;
      transform_modifier_volume(parameter,
                                cube::vertices,
                                cube::faces,
                                cube::num_faces,
                                scale);
    }
    // end of opaque modifier list
    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_modifier_volume_list();

    core_start_render(frame_ix, num_frames);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix, num_frames);
    while (spg_status::vsync(holly.SPG_STATUS));

    constexpr float half_degree = 0.01745329f / 2;
    theta += half_degree;
    frame_ix += 1;
  }
}
