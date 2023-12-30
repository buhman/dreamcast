#include <cstdint>
#include <bit>

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

#include "geometry/border.hpp"
#include "geometry/circle.hpp"
#include "math/vec4.hpp"

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

  maple::init_host_command_all_ports<command_type, response_type>(command_buf, receive_buf,
								  data_fields);
  maple::dma_start(command_buf);

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

    /*
    bool a = ft0::data_transfer::digital_button::a(data_fields.data.digital_button);
    if (a == 0) {
      serial::string("port ");
      serial::integer<uint8_t>(port);
      serial::string("  `a` press ");
      serial::integer<uint8_t>(a);
    }
    */
    data[port].analog_axis_3 = data_fields.data.analog_axis_3;
    data[port].analog_axis_4 = data_fields.data.analog_axis_4;
  }
}

void transform(ta_parameter_writer& parameter,
	       const vec3 * vertices,
	       const face& face,
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

  parameter.append<global_polygon_type_0>() = global_polygon_type_0(parameter_control_word,
                                                                    isp_tsp_instruction_word,
                                                                    tsp_instruction_word,
                                                                    0);

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

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

    parameter.append<global_end_of_list>() = global_end_of_list();
    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();
    core_start_render(frame_ix, num_frames);

    v_sync_out();
    core_wait_end_of_render_video(frame_ix, num_frames);
    frame_ix += 1;
  }
}
