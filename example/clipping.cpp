#include <cstdint>
#include <bit>

#include "align.hpp"

#include "vga.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "memorymap.hpp"
#include "sh7091/serial.hpp"

#include "geometry/circle.hpp"
#include "math/vec4.hpp"
#include "math/math.hpp"

#include "maple/maple.hpp"
#include "maple/maple_impl.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

uint32_t _command_buf[1024 / 4 + 32];
uint32_t _receive_buf[1024 / 4 + 32];

static ft0::data_transfer::data_format data[4];

void do_get_condition(uint32_t * command_buf,
		      uint32_t * receive_buf)
{
  using command_type = get_condition;
  using response_type = data_transfer<ft0::data_transfer::data_format>;

  get_condition::data_fields data_fields = {
    .function_type = std::byteswap(function_type::controller)
  };

  const uint32_t size = maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf,
                                                                                        data_fields);
  maple::dma_start(command_buf, size);

  using command_response_type = struct maple::command_response<response_type::data_fields>;
  for (uint8_t port = 0; port < 4; port++) {
    auto response = reinterpret_cast<command_response_type *>(receive_buf);
    auto& bus_data = response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((data_fields.function_type & std::byteswap(function_type::controller)) == 0) {
      return;
    }

    data[port].analog_axis_3 = data_fields.data.analog_axis_3;
    data[port].analog_axis_4 = data_fields.data.analog_axis_4;
  }
}

vec3 intersection(vec3& a, vec3& b, vec3& n)
{
  const float t = (-dot(n, a)) / dot(n, b - a);
  return a + t * (b - a);
}

void transform(ta_parameter_writer& parameter,
	       const vec3 * vertices,
	       const face_vtn& face,
	       const vec4& color,
	       const vec3& position,
	       const bool enable_clipping
	       )
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

  constexpr uint32_t strip_length = 3;
  vec3 points[strip_length * 2];
  uint32_t positive = 0;
  uint32_t negative = 0;

  vec3 plane_normal = {-1.f, 0.f, 0.f};

  // object transform and clip
  for (uint32_t i = 0; i < strip_length; i++) {
    uint32_t vertex_ix = face[i].vertex;
    auto vertex = vertices[vertex_ix];

    vertex = (vertex * 0.5f);

    // rotate 90° around the X axis
    float x = vertex.x;
    float y = vertex.z;
    float z = vertex.y;

    // object transform
    x += position.x; // object space
    y += position.y; // object space
    z += position.z; // object space

    // clip
    auto point = vec3(x, y, z);
    float distance = dot(plane_normal, point);
    if ((!enable_clipping) || distance > 0.0f) {
      points[0 + positive] = point;
      positive += 1;
    } else { // is negative (or intersects)
      points[(strip_length - 1) - negative] = point;
      negative += 1;
    }
  }

  uint32_t num_tris = 0;
  if ((!enable_clipping) || positive == 3) {
    num_tris = 1;
    // nothing to clip
  } else if (positive == 0) {
    num_tris = 0;
    // clip everything
  } else if (positive == 1) {
    num_tris = 1;
    auto& A = points[0]; // positive
    auto& B = points[1]; // negative
    auto& C = points[2]; // negative

    /*
    //     A
    //     /\
    //    /  \
    // -AB----AC--
    //  /      \
    // B________C
    */

    auto AB_ = intersection(A, B, plane_normal);
    auto AC_ = intersection(A, C, plane_normal);

    points[0] = A;
    points[1] = AC_;
    points[2] = AB_;

  } else if (positive == 2) {
    num_tris = 2;
    auto& A = points[0]; // positive
    auto& B = points[1]; // positive
    auto& C = points[2]; // negative

    // A _____ B
    //  \     /
    //--AC---BC--
    //   \  /
    //    \/
    //    C
    auto AC_ = intersection(A, C, plane_normal);
    auto BC_ = intersection(B, C, plane_normal);

    points[0] = A;
    points[1] = B;
    points[2] = AC_;

    points[3] = B;
    points[4] = BC_;
    points[5] = AC_;
  }

  for (uint32_t j = 0; j < num_tris; j++) {
    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
    for (uint32_t i = 0; i < strip_length; i++) {
      float x = points[3 * j + i].x;
      float y = points[3 * j + i].y;
      float z = points[3 * j + i].z;

      // camera transform
      z += 1;

      // screen space transform
      x *= 240.f;
      y *= 240.f;
      x += 320.f;
      y += 240.f;
      z = 1 / z;

      bool end_of_strip = i == strip_length - 1;
      parameter.append<ta_vertex_parameter::polygon_type_1>() =
	ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					    x, y, z,
					    1.0f, // alpha
					    (i == 0) ? 1.0f : 0.0f, // r
					    (i == 1) ? 1.0f : 0.0f, // g
					    (i == 2) ? 1.0f : 0.0f  // b
					    );
    }
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
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

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
  float x_pos = 0;
  float y_pos = 0;

  while (1) {
    do_get_condition(command_buf, receive_buf);

    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);

    float x_ = static_cast<float>(data[0].analog_axis_3 - 0x80) / 127.f;
    float y_ = static_cast<float>(data[0].analog_axis_4 - 0x80) / 127.f;
    if (x_ > x_pos) x_pos += (0.09f * ((x_ - x_pos) * (x_ - x_pos)));
    if (x_ < x_pos) x_pos -= (0.09f * ((x_ - x_pos) * (x_ - x_pos)));
    if (y_ > y_pos) y_pos += (0.09f * ((y_ - y_pos) * (y_ - y_pos)));
    if (y_ < y_pos) y_pos -= (0.09f * ((y_ - y_pos) * (y_ - y_pos)));

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    for (uint32_t i = 0; i < circle::num_faces; i++) {
      /*
      transform(parameter,
		circle::vertices,
		circle::faces[i],
		vec4{1.0f, 0.5f, 0.5f, 0.0f} * (((i/1.2f) * (1.f / circle::num_faces)) + (1.f/1.2f)), // color
		{x_pos * 2, y_pos * 2, 1.0f}, // position
		false // clipping
		);
      */

      transform(parameter,
		circle::vertices,
		circle::faces[i],
		{1.0f, 1.0f, 0.0f, 0.0f}, // color
		{x_pos * 2, y_pos * 2, 0.0f}, // position
		true // clipping
		);
    }

    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();
    core_start_render(frame_ix, num_frames);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix, num_frames);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix += 1;
    theta += (2.f * pi) / 720.f;
  }
}
