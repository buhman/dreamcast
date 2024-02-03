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

#include "geometry/triangle.hpp"
#include "geometry/circle.hpp"
#include "math/vec4.hpp"
#include "math/math.hpp"
#include "math/geometry.hpp"

#include "maple/maple.hpp"
#include "maple/maple_impl.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

uint32_t _command_buf[(1024 + 32) / 4];
uint32_t _receive_buf[(1024 + 32) / 4];

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

    data[port].analog_axis_1 = data_fields.data.analog_axis_1;
    data[port].analog_axis_2 = data_fields.data.analog_axis_2;
    data[port].analog_axis_3 = data_fields.data.analog_axis_3;
    data[port].analog_axis_4 = data_fields.data.analog_axis_4;
  }
}

constexpr vec3 colors[] = {
  {1.f, 0.5f, 0.f},
  {0.f, 1.0f, 0.f},
  {0.f, 0.5f, 1.f},
  {1.f, 0.0f, 1.f},
};

void transform1(ta_parameter_writer& parameter,
		const vec3& v,
		const vec3& c,
		bool end_of_strip)
{
  float x = v.x;
  float y = v.y;
  float z = v.z;

  // camera transform
  z += 1;

  // screen space transform
  x *= 240.f;
  y *= 240.f;
  x += 320.f;
  y += 240.f;
  z = 1 / z;

  parameter.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(end_of_strip),
					x, y, z,
					1.0f, // alpha
					c.r,
					c.g,
					c.b
					);

}

void transform(ta_parameter_writer& parameter,
	       const vec3 * vertices,
	       const face_vtn& face,
	       const vec4& color,
	       const vec3& position,
	       const float theta,
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
  vec3 points[strip_length];

  // object transform and clip
  for (uint32_t i = 0; i < strip_length; i++) {
    uint32_t vertex_ix = face[i].vertex;
    auto vertex = vertices[vertex_ix];

    vertex = (vertex * 0.5f);

    // rotate 90° around the X axis
    //float x = vertex.x;
    //float y = vertex.z;
    //float z = vertex.y;

    float x = vertex.x * cos(theta) - vertex.z * sin(theta);
    float y = vertex.x * sin(theta) + vertex.z * cos(theta);
    float z = vertex.y;

    // object transform
    x += position.x; // object space
    y += position.y; // object space
    z += position.z; // object space

    // clip
    points[i] = vec3(x, y, z);
  }

  const vec3 plane_point = {0.f, 0.f, 0.f};
  const vec3 plane_normal = {-1.f, 0.f, 0.f};
  vec3 output[4];
  int output_length = geometry::clip_polygon<3>(output, plane_point, plane_normal, &points[0]);

  if (output_length >= 3) {
    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
    transform1(parameter, output[0], colors[0], false);
    transform1(parameter, output[1], colors[1], false);
    transform1(parameter, output[2], colors[2], true);
  }
  if (output_length >= 4) {
    parameter.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
    transform1(parameter, output[0], colors[0], false);
    transform1(parameter, output[2], colors[2], false);
    transform1(parameter, output[3], colors[3], true);
  }

  /*

    A B
    D C

   */
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

    const float l_ = static_cast<float>(data[0].analog_axis_1) * (1.f / 255.f);
    const float r_ = static_cast<float>(data[0].analog_axis_2) * (1.f / 255.f);
    const float t_ = ((l_ > r_) ? l_ : -r_) * 3.14f / 2.f;

    if (t_ > theta) theta += (0.04f * ((t_ - theta) * (t_ - theta)));
    else            theta -= (0.04f * ((t_ - theta) * (t_ - theta)));

    const float x_ = static_cast<float>(data[0].analog_axis_3 - 0x80) / 127.f;
    const float y_ = static_cast<float>(data[0].analog_axis_4 - 0x80) / 127.f;
    if (x_ > x_pos) x_pos += (0.02f * ((x_ - x_pos) * (x_ - x_pos)));
    else            x_pos -= (0.02f * ((x_ - x_pos) * (x_ - x_pos)));
    if (y_ > y_pos) y_pos += (0.02f * ((y_ - y_pos) * (y_ - y_pos)));
    else            y_pos -= (0.02f * ((y_ - y_pos) * (y_ - y_pos)));

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    for (uint32_t i = 0; i < circle::num_faces; i++) {
      transform(parameter,
		circle::vertices,
		circle::faces[i],
		{1.0f, 1.0f, 0.0f, 0.0f}, // color
		{x_pos * 2, y_pos * 2, 0.0f}, // position
		theta,
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
  }
}
