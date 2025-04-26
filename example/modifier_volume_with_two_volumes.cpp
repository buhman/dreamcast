#include <cstdint>
#include <bit>

#include "align.hpp"
#include "holly/video_output.hpp"

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

#include "geometry/plane.hpp"
#include "geometry/cube.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "texture/macaw/macaw.data.h"
#include "texture/wolf/wolf.data.h"
#include "twiddle.hpp"

static ft0::data_transfer::data_format data[4];

void do_get_condition()
{
  uint8_t send_buf[1024] __attribute__((aligned(32)));
  uint8_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::controller);

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((std::byteswap(data_fields.function_type) & function_type::controller) == 0) {
      return;
    }

    data[port].analog_coordinate_axis[0] = data_fields.data.analog_coordinate_axis[0];
    data[port].analog_coordinate_axis[1] = data_fields.data.analog_coordinate_axis[1];
    data[port].analog_coordinate_axis[2] = data_fields.data.analog_coordinate_axis[2];
    data[port].analog_coordinate_axis[3] = data_fields.data.analog_coordinate_axis[3];
  }
}

struct rot_pos {
  float theta;
  float x;
  float y;
};

vec3 _transform(const vec3& point,
                const uint32_t scale)
{
  float x = point.x;
  float y = point.y;
  float z = point.z;

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

vec3 _transform(const vec3& point,
                const uint32_t scale,
                const struct rot_pos& rot_pos)
{
  float x = point.x;
  float y = point.y;
  float z = point.z;
  float t;

  // object transform
  t  = z * cos(rot_pos.theta) - x * sin(rot_pos.theta);
  x  = z * sin(rot_pos.theta) + x * cos(rot_pos.theta);
  z  = t;

  x += rot_pos.x;
  z += rot_pos.y;

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

uint32_t argb8888(const vec4& color)
{
  return ((static_cast<int>(255.f * color.a) & 0xff) << 24)
       | ((static_cast<int>(255.f * color.r) & 0xff) << 16)
       | ((static_cast<int>(255.f * color.g) & 0xff) << 8)
       | ((static_cast<int>(255.f * color.b) & 0xff) << 0)
       ;
}

void transform_polygon(ta_parameter_writer& writer,
                       const vec3 * vertices,
		       const vec2 * texture,
                       const face_vtn& face,
                       const float scale,
                       const vec4& color0,
                       const vec4& color1,
                       const struct rot_pos& rot_pos)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::punch_through
                                        | obj_control::col_type::packed_color
                                        | obj_control::shadow
                                        | obj_control::volume::polygon::with_two_volumes
					| obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word_0 = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(128)
                                      | tsp_instruction_word::texture_v_size::from_int(128);

  const uint32_t tsp_instruction_word_1 = tsp_instruction_word::src_alpha_instr::other_color
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(128)
                                      | tsp_instruction_word::texture_v_size::from_int(128);

  const uint32_t texture_address_0 = texture_memory_alloc::texture.start
                                   + 128 * 128 * 2 * 0;
  const uint32_t texture_address_1 = texture_memory_alloc::texture.start
                                   + 128 * 128 * 2 * 1;

  const uint32_t texture_control_word_0 = texture_control_word::pixel_format::_565
					| texture_control_word::scan_order::twiddled
					| texture_control_word::texture_address(texture_address_0 / 8);

  const uint32_t texture_control_word_1 = texture_control_word::pixel_format::_565
					| texture_control_word::scan_order::twiddled
					| texture_control_word::texture_address(texture_address_1 / 8);

  writer.append<ta_global_parameter::polygon_type_3>() =
    ta_global_parameter::polygon_type_3(parameter_control_word,
					isp_tsp_instruction_word,
					tsp_instruction_word_0,   // tsp_instruction_word_0
					texture_control_word_0, // texture_control_word_0
					tsp_instruction_word_1,   // tsp_instruction_word_1
					texture_control_word_1, // texture_control_word_1
					0, // data_size_for_sort_dma
					0  // next_address_for_sort_dma
					);

  constexpr uint32_t strip_length = 3;
  for (uint32_t i = 0; i < strip_length; i++) {
    // world transform
    uint32_t vertex_ix = face[i].vertex;
    auto& vertex = vertices[vertex_ix];
    auto point = _transform(vertex, scale, rot_pos);

    uint32_t texture_ix = face[i].texture;
    auto& uv = texture[texture_ix];

    bool end_of_strip = i == strip_length - 1;
    writer.append<ta_vertex_parameter::polygon_type_11>() =
      ta_vertex_parameter::polygon_type_11(polygon_vertex_parameter_control_word(end_of_strip),
					   point.x,
					   point.y,
					   point.z,
					   uv.u,
					   uv.v,
					   argb8888(color0), // base_color_0
					   0,                // offset_color_0
					   uv.u,
					   uv.v,
					   argb8888(color1), // base_color_1
					   0                 // offset_color_1
					   );
  }
}

void transform_modifier_volume(ta_parameter_writer& writer,
                               const vec3 * vertices,
                               const face_vtn * faces,
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

  writer.append<ta_global_parameter::modifier_volume>() =
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
    auto a = _transform(_a, scale);
    auto b = _transform(_b, scale);
    auto c = _transform(_c, scale);

    if (i == (num_faces - 1)) {
      const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                                 | para_control::list_type::opaque_modifier_volume
                                                 | obj_control::volume::modifier_volume::last_in_volume;

      const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                                   | isp_tsp_instruction_word::culling_mode::no_culling;

      writer.append<ta_global_parameter::modifier_volume>() =
        ta_global_parameter::modifier_volume(last_parameter_control_word,
					     last_isp_tsp_instruction_word);

    }

    writer.append<ta_vertex_parameter::modifier_volume>() =
      ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
					   a.x, a.y, a.z,
					   b.x, b.y, b.z,
					   c.x, c.y, c.z);
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

void
load_texture(const uint8_t * src,
	     const uint32_t size,
	     const uint32_t ix)
{
  auto texture = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);

  uint16_t temp[size / 3];
  for (uint32_t px = 0; px < size / 3; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    temp[px] = rgb565;
  }
  twiddle::texture(&texture[(128 * 128 * 2 * ix) / 2], temp, 128, 128);
}

void update_rot_pos(struct rot_pos& rot_pos)
{
  const float l_pos = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_pos = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_pos = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) * (0.5f / 127.f);
  const float y_pos = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) * (0.5f / 127.f);

  const float rotation = (l_pos > r_pos) ? (l_pos) : (-r_pos);

  constexpr float half_degree = 0.01745329f / 2;

  rot_pos.x += x_pos / 10.f;
  rot_pos.y += y_pos / 10.f;
  rot_pos.theta += rotation * half_degree * 10.f;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[((32 * 8192) + 32) / 4];

void main()
{
  video_output::set_mode_vga();

  auto src0 = reinterpret_cast<const uint8_t *>(&_binary_texture_macaw_macaw_data_start);
  auto size0 = reinterpret_cast<const uint32_t>(&_binary_texture_macaw_macaw_data_size);

  auto src1 = reinterpret_cast<const uint8_t *>(&_binary_texture_wolf_wolf_data_start);
  auto size1 = reinterpret_cast<const uint32_t>(&_binary_texture_wolf_wolf_data_size);

  load_texture(src0, size0, 0);
  load_texture(src1, size1, 1);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::_16x4byte
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::no_list
			      | ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::no_list;

  constexpr struct opb_size opb_size = { .opaque = 0
				       , .opaque_modifier = 16 * 4
				       , .translucent = 0
				       , .translucent_modifier = 0
				       , .punch_through = 16 * 4,
				       };
  /*
  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::no_list
			      | ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 16 * 4
				       , .translucent = 0
				       , .translucent_modifier = 0
				       , .punch_through = 0,
				       };
  */

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;

  struct rot_pos rot_pos = { 0.f, 0.f, 0.f };

  while (true) {
    do_get_condition();

    update_rot_pos(rot_pos);

    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);
    auto writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));
    { // plane
      vec4 color0 = {1.0, 0.9, 0.9, 0.9};
      vec4 color1 = {1.0, 0.9, 0.9, 0.9};
      float scale = 2.f;
      for (uint32_t i = 0; i < plane::num_faces; i++) {
        transform_polygon(writer,
                          plane::vertices,
                          plane::texture,
                          plane::faces[i],
                          scale,
                          color0,
                          color1,
                          rot_pos);
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
    writer.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);


    { // cube
      float scale = 1.f;
      transform_modifier_volume(writer,
                                cube::vertices,
                                cube::faces,
                                cube::num_faces,
                                scale);
    }
    // end of opaque modifier list
    writer.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);


    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_modifier_volume_list();


    core_start_render(frame_ix);
    core_wait_end_of_render_video();


    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));


    frame_ix = (frame_ix + 1) & 1;
  }
}
