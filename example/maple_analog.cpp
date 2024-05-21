#include <cstdint>
#include <bit>

#include "align.hpp"

#include "holly/video_output.hpp"
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

#include "geometry/border.hpp"
#include "geometry/circle.hpp"
#include "math/vec4.hpp"

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

  const uint32_t command_size = maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf,
                                                                                        data_fields);
  using host_response_type = struct maple::host_response<response_type::data_fields>;
  auto host_response = reinterpret_cast<host_response_type *>(receive_buf);

  maple::dma_start(command_buf, command_size,
                   receive_buf, maple::sizeof_command(host_response));

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
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

void transform(ta_parameter_writer& parameter,
	       const vec3 * vertices,
	       const face_vtn& face,
	       const vec4& color,
	       const vec3& position,
	       const float scale
	       )
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
    auto point = vertex;

    // rotate 90° around the X axis
    float x = point.x;
    float y = point.z;
    float z = point.y;

    // world transform
    x *= scale; // world space
    y *= scale; // world space
    z *= 10;

    // object transform
    x += position.x; // object space
    y += position.y; // object space
    z += position.z; // object space

    // camera transform
    z += 1;
    //y -= 10;

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
					  color.w, // alpha
					  color.x, // r
					  color.y, // g
					  color.z  // b
					  );
  }
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff220000);
}

uint32_t _ta_parameter_buf[((32 * 8192) + 32) / 4];

void main()
{
  uint32_t * command_buf = align_32byte(_command_buf);
  uint32_t * receive_buf = align_32byte(_receive_buf);

  video_output::set_mode_vga();

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

  while (1) {
    do_get_condition(command_buf, receive_buf);

    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);

    float x_pos = static_cast<float>(data[0].analog_axis_3 - 0x80) * (0.5 / 127);
    float y_pos = static_cast<float>(data[0].analog_axis_4 - 0x80) * (0.5 / 127);

    auto parameter = ta_parameter_writer(ta_parameter_buf);
    for (uint32_t i = 0; i < border::num_faces; i++) {
      transform(parameter,
		border::vertices,
		border::faces[i],
		{1.0, 0.0, 0.0, 1.0}, // color
		{0.0, 0.0, 0.0}, // position
		0.5f * (1.f / 0.95f) // scale
		);
    }

    for (uint32_t i = 0; i < circle::num_faces; i++) {
      transform(parameter,
		circle::vertices,
		circle::faces[i],
		{0.0, 1.0, 1.0, 1.0}, // color
		{x_pos, y_pos, 0.0}, // position
		0.05f // scale
		);
    }

    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();
    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
