#include "align.hpp"
#include "dve.hpp"
#include "memorymap.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/video_output.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"

#include "sh7091/serial.hpp"

void print_cable_type_resolution(const uint32_t cable_type, const struct video_output::framebuffer_resolution& framebuffer_resolution)
{
  serial::string("cable type: ");
  switch (cable_type) {
  case pdtra::cable_type::vga:     serial::string("vga\n");                break;
  case pdtra::cable_type::rgb:     serial::string("rgb\n");                break;
  case pdtra::cable_type::cvbs_yc: serial::string("cvbs_yc\n");            break;
  default:                         serial::string("undefined/reserved\n"); break;
  }
  serial::string("framebuffer resolution: ");
  serial::integer<uint16_t, serial::dec>(framebuffer_resolution.width, ' ', 3);
  serial::integer<uint16_t, serial::dec>(framebuffer_resolution.height, '\n', 3);
}

struct vertex {
  float x;
  float y;
  float z;
};

// screen space coordinates
const struct vertex quad_vertices[4] = {
  { -0.5f,   0.5f, 0.1f },
  { -0.5f,  -0.5f, 0.1f },
  {  0.5f,  -0.5f, 0.1f },
  {  0.5f,   0.5f, 0.1f },
};

struct vertex transform_vertex(const struct vertex& v,
			       const struct video_output::framebuffer_resolution& framebuffer_resolution,
			       const float theta)
{
  float x = v.x * __builtin_cosf(theta) - v.y * __builtin_sinf(theta);
  float y = v.x * __builtin_sinf(theta) + v.y * __builtin_cosf(theta);

  return {
    x * (framebuffer_resolution.height / 2) + (framebuffer_resolution.width / 2),
    y * (framebuffer_resolution.height / 2) + (framebuffer_resolution.height / 2),
    v.z,
  };
}

uint32_t transform(uint32_t * ta_parameter_buf,
		   const struct video_output::framebuffer_resolution& framebuffer_resolution,
		   const float theta)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);

  auto a = transform_vertex(quad_vertices[0], framebuffer_resolution, theta);
  auto b = transform_vertex(quad_vertices[1], framebuffer_resolution, theta);
  auto c = transform_vertex(quad_vertices[2], framebuffer_resolution, theta);
  auto d = transform_vertex(quad_vertices[3], framebuffer_resolution, theta);

  const uint32_t parameter_control_word = para_control::para_type::sprite
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;


  constexpr uint32_t base_color = 0xffff0000;
  parameter.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
                                isp_tsp_instruction_word,
                                tsp_instruction_word,
                                texture_control_word,
                                base_color,
                                0, // offset_color
                                0, // data_size_for_sort_dma
                                0); // next_address_for_sort_dma

  parameter.append<ta_vertex_parameter::sprite_type_0>() =
    ta_vertex_parameter::sprite_type_0(para_control::para_type::vertex_parameter,
				       a.x, a.y, a.z,
				       b.x, b.y, b.z,
				       c.x, c.y, c.z,
				       d.x, d.y);
  // curiously, there is no quad_veritices[3].z in vertex_sprite_type_0

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size, const struct video_output::framebuffer_resolution& framebuffer_resolution)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background, 0xff220000);

  region_array2(mem->region_array,
	        (offsetof (struct texture_memory_alloc, object_list)),
		framebuffer_resolution.width  / 32, // width
		framebuffer_resolution.height / 32, // height
		opb_size
		);
}

uint32_t _ta_parameter_buf[((32 + 64 + 32) + 32) / 4];

void main()
{
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

  uint32_t cable_type = video_output::get_cable_type();
  auto framebuffer_resolution = video_output::set_mode_by_cable_type(cable_type);
  print_cable_type_resolution(cable_type, framebuffer_resolution);
  init_texture_memory(opb_size, framebuffer_resolution);
  core_init();

  float theta = 0;
  uint32_t frame_ix = 0;
  constexpr uint32_t num_frames = 1;

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      framebuffer_resolution.width / 32,
			      framebuffer_resolution.height / 32);
    uint32_t ta_parameter_size = transform(ta_parameter_buf, framebuffer_resolution, theta);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render((offsetof (struct texture_memory_alloc, framebuffer)),
		      framebuffer_resolution.width, // frame_width
		      0x00096000, // frame_size
		      frame_ix, num_frames);;
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix, num_frames);
    if (cable_type != video_output::get_cable_type()) {
      cable_type = video_output::get_cable_type();
      framebuffer_resolution = video_output::set_mode_by_cable_type(cable_type);
      print_cable_type_resolution(cable_type, framebuffer_resolution);
      init_texture_memory(opb_size, framebuffer_resolution);
    }
    while (spg_status::vsync(holly.SPG_STATUS));

    constexpr float half_degree = 0.01745329f / 2.f;
    theta += half_degree;
    frame_ix += 1;
  }
}
