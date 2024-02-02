#include <cstdint>

#include "align.hpp"
#include "vga.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
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

#include "geometry/heart.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"

using mat4x4 = mat<4, 4, float>;

constexpr float pi = 3.141592653589793;

struct rotation_weights {
  float drx;
  float dry;
  float drz;
};

// randomly generated numbers
constexpr rotation_weights weights[16] = {
  {-0.8154296875, 0.8583984375, -0.498046875},
  {0.322265625, 0.6796875, 0.3251953125},
  {-0.2626953125, -0.7744140625, 0.37109375},
  {0.5830078125, 0.42578125, 0.5546875},
  {0.9140625, 0.7568359375, -0.037109375},
  {0.8974609375, 0.103515625, -0.2666015625},
  {0.8427734375, -0.4091796875, -0.365234375},
  {0.162109375, -0.603515625, 0.4248046875},
  {-0.47265625, -0.73828125, -0.4912109375},
  {-0.921875, 0.4609375, 0.2216796875},
  {0.400390625, -0.5634765625, -0.3232421875},
  {0.896484375, 0.26953125, -0.951171875},
  {0.541015625, 0.90625, 0.640625},
  {0.5927734375, -0.361328125, 0.21875},
  {-0.9267578125, -0.9423828125, 0.4580078125},
  {0.16796875, 0.3662109375, 0.603515625},
};

// randomly generated numbers
float lighting_weights[16] = {
  0.7314453125,
  0.44921875,
  0.259765625,
  0.3232421875,
  0.1015625,
  0.2529296875,
  0.8662109375,
  0.5439453125,
  0.1337890625,
  0.041015625,
  0.6298828125,
  0.30859375,
  0.517578125,
  0.6259765625,
  0.283203125,
  0.982421875,
};

struct model_transform {
  float x;
  float y;
  float z;
  float rx;
  float ry;
  float rz;

  model_transform()
    : x(0.f)
    , y(0.f)
    , z(0.f)
    , rx(0.f)
    , ry(0.f)
    , rz(0.f)
  { }
};

struct model_transform models[] = {
};

inline mat4x4 rotate_x(float t)
{
  return mat4x4(1.f,    0.f,     0.f, 0.f,
		0.f, cos(t), -sin(t), 0.f,
		0.f, sin(t),  cos(t), 0.f,
		0.f,    0.f,     0.f, 1.f
		);
}

inline mat4x4 rotate_y(float t)
{
  return mat4x4( cos(t), 0.f, sin(t), 0.f,
		    0.f, 1.f,    0.f, 0.f,
		-sin(t), 0.f, cos(t), 0.f,
		    0.f, 0.f,    0.f, 1.f
		);
}

inline mat4x4 rotate_z(float t)
{
  return mat4x4(cos(t), -sin(t), 0.f, 0.f,
		sin(t),  cos(t), 0.f, 0.f,
		   0.f,     0.f, 1.f, 0.f,
		   0.f,     0.f, 0.f, 1.f
		);
}

inline mat4x4 translate(float x, float y, float z)
{
  return mat4x4(1.f, 0.f, 0.f,   x,
		0.f, 1.f, 0.f,   y,
		0.f, 0.f, 1.f,   z,
		0.f, 0.f, 0.f, 1.f
		);
}

vec3 _transform(const vec4& point)
{
  float x = point.x;
  float y = point.y;
  float z = point.z;

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

void transform_model(ta_parameter_writer& parameter,
		     const position__color * vertices,
		     const vec3 * normals,
		     const face_vn * faces,
		     const uint32_t num_faces,
		     const model_transform& mt,
		     const float lighting_weight)
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

  const mat4x4 mat = translate(mt.x, mt.y, mt.z) * rotate_z(mt.rz) * rotate_y(mt.ry) * rotate_x(mt.rx);

  constexpr uint32_t strip_length = 3;
  for (uint32_t face_ix = 0; face_ix < num_faces; face_ix++) {

    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );

    auto& face = faces[face_ix];
    for (uint32_t i = 0; i < strip_length; i++) {
      // world transform
      uint32_t vertex_ix = face[i].vertex;
      auto& vertex = vertices[vertex_ix].position;
      auto& color = vertices[vertex_ix].color;
      auto point = mat * vec4(vertex);

      uint32_t normal_ix = face[i].normal;
      auto& normal = normals[normal_ix];
      auto n = mat * vec4(normal);

      vec4 light = {0.f, 0.f, 40.f, 1.f};
      auto l = light - point;
      auto n_dot_l = dot(n, l);
      vec3 c(0.f, 0.f, 0.f);
      c.r += color.r * 0.1;
      c.g += color.g * 0.1;
      c.b += color.b * 0.1;
      if (n_dot_l > 0) {
	float intensity = n_dot_l / (length(n) * length(l));
        c.r += color.r * intensity * lighting_weight;
        c.g += color.g * intensity * lighting_weight;
        c.b += color.b * intensity * lighting_weight;
      }

      auto screen = _transform(point);

      bool end_of_strip = i == strip_length - 1;
      parameter.append<ta_vertex_parameter::polygon_type_1>() =
	ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					    screen.x,
					    screen.y,
					    screen.z,
					    1.0f,    // alpha
					    c.r, // red
					    c.g, // green
					    c.b  // blue
					    );
    }
  }
}

void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background, 0xff220000);
  holly.VO_BORDER_COL = 0x00220000;

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

  model_transform mt[16] = {};
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      int ix = y * 4 + x;
      mt[ix].x = -8.f + 5.f * static_cast<float>(x);
      mt[ix].y = -7.5f + 5.f * static_cast<float>(y);
      mt[ix].z = 6.f;
      mt[ix].rx = (-8.f + static_cast<float>(ix)) * -pi / 16;
      mt[ix].ry = (-8.f + static_cast<float>(ix)) * ix * -pi / 16;
      mt[ix].rz = (-8.f + static_cast<float>(ix)) * ix * -pi / 16;
    }
  }

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);
    auto parameter = ta_parameter_writer(ta_parameter_buf);
    { // plane
      for (uint32_t i = 0; i < 16; i++) {
	transform_model(parameter,
			heart::vertices,
			heart::normals,
			heart::faces,
			heart::num_faces,
			mt[i],
			(1.f + sin(theta * 2 * lighting_weights[i])) * 0.5f);

	// update model
	auto& weight = weights[i];
	mt[i].rx += weight.drx / 50.f;
	mt[i].ry += weight.dry / 50.f;
	mt[i].rz += weight.drz / 50.f;
      }
    }
    // end of opaque list
    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();

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
