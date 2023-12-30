#include <cstdint>

#include "align.hpp"

#include "vga.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "memorymap.hpp"
#include "serial.hpp"

#include "geometry/icosphere.hpp"
#include "geometry/suzanne.hpp"
#include "math/vec4.hpp"

constexpr float half_degree = 0.01745329f / 2;

#define MODEL suzanne

vec3 rotate(const vec3& vertex,
            const float theta)
{
  float x = vertex.x;
  float y = vertex.y;
  float z = vertex.z;
  float t;

  t  = y * cos(theta) - z * sin(theta);
  z  = y * sin(theta) + z * cos(theta);
  y  = t;

  float theta2 = 3.14 * sin(theta / 2);

  t  = x * cos(theta2) - z * sin(theta2);
  z  = x * sin(theta2) + z * cos(theta2);
  x  = t;

  return vec3(x, y, z);
}

void transform(ta_parameter_writer& parameter,
               const uint32_t face_ix,
               const float theta,
               const vec3 lights[3])
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::cull_if_positive;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  parameter.append<global_polygon_type_0>() = global_polygon_type_0(parameter_control_word,
                                                                    isp_tsp_instruction_word,
                                                                    tsp_instruction_word,
                                                                    0);
  auto& face = MODEL::faces[face_ix];

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    // world transform
    uint32_t vertex_ix = face[i].vertex;
    auto& vertex = MODEL::vertices[vertex_ix];
    auto point = rotate(vertex, theta);

    // lighting transform
    uint32_t normal_ix = face[i].normal;
    auto& normal = MODEL::normals[normal_ix];
    auto n = rotate(normal, theta);

    vec4 color = {0.2, 0.2, 0.2, 1.0};

    // intensity calculation
    {
      auto l = lights[0] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
        color.x += 0.6 * n_dot_l / (length(n) * length(l));
      }
    }

    {
      auto l = lights[1] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
        color.y += 0.6 * n_dot_l / (length(n) * length(l));
      }
    }

    {
      auto l = lights[2] - point;
      auto n_dot_l = dot(n, l);
      if (n_dot_l > 0) {
        color.z += 0.6 * n_dot_l / (length(n) * length(l));
      }
    }

    float x = point.x;
    float y = point.y;
    float z = point.z;

    x *= 10;
    y *= 10;
    z *= 10;

    // camera transform
    z += 20;

    // perspective
    x = x / z;
    y = y / z;

    // screen space transform
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;
    z = 1 / z;

    parameter.append<vertex_polygon_type_1>() =
      vertex_polygon_type_1(x, y, z,
			    color.w, // alpha
			    color.x, // r
			    color.y, // g
			    color.z, // b
			    end_of_strip);
  }
}

void transform2(ta_parameter_writer& parameter,
                const vec3& pos,
                const vec4& color)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  parameter.append<global_polygon_type_0>() = global_polygon_type_0(parameter_control_word,
                                                                    isp_tsp_instruction_word,
                                                                    tsp_instruction_word,
                                                                    0);

  constexpr vec3 triangle[] = {
    { 0.f, -1.f, 0.f},
    {-1.f,  1.f, 0.f},
    { 1.f,  1.f, 0.f},
  };

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;
    float x = triangle[i].x;
    float y = triangle[i].y;
    float z = triangle[i].z;

    x *= 0.2;
    y *= 0.2;
    z *= 0.2;

    x += pos.x;
    y += pos.y;
    z += pos.z;

    // camera transform
    z += 20;

    // perspective
    x = x / z;
    y = y / z;

    // screen space transform
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;
    z = 1 / z;

    parameter.append<vertex_polygon_type_1>() =
      vertex_polygon_type_1(x, y, z,
			    color.w, // alpha
			    color.x, // r
			    color.y, // g
			    color.z, // b
			    end_of_strip);
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
  vga();

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
  constexpr uint32_t num_frames = 1;

  float theta = 0;
  vec3 lights[3] = {
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
  };

  while (1) {
    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);

    float theta2 = 3.14 * 2 * sin(theta / 7);

    lights[0].x = cos(theta) * 15;
    lights[0].z = sin(theta) * 15;

    lights[1].x = cos(theta2 + half_degree * 180.f) * 15;
    lights[1].z = sin(theta2 + half_degree * 180.f) * 15;

    lights[2].x = cos(theta + half_degree * 360.f) * 15;
    lights[2].z = sin(theta + half_degree * 360.f) * 15;

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    for (uint32_t i = 0; i < MODEL::num_faces; i++) {
      transform(parameter, i, theta, lights);
    }
    transform2(parameter, lights[0], {1.f, 0.f, 0.f, 1.f});
    transform2(parameter, lights[1], {0.f, 1.f, 0.f, 1.f});
    transform2(parameter, lights[2], {0.f, 0.f, 1.f, 1.f});

    parameter.append<global_end_of_list>() = global_end_of_list();
    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();
    core_start_render(frame_ix, num_frames);

    v_sync_out();
    core_wait_end_of_render_video(frame_ix, num_frames);
    theta += half_degree;
    frame_ix += 1;
  }
}
