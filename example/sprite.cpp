#include <cstdint>

#include "align.hpp"
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

struct vertex {
  float x;
  float y;
  float z;
};

// screen space coordinates
const struct vertex quad_vertices[4] = {
  { 200.f,  360.f, 0.1f },
  { 200.f,  120.f, 0.1f },
  { 440.f,  120.f, 0.1f },
  { 440.f,  360.f, 0.1f },
};

uint32_t transform(uint32_t * ta_parameter_buf)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);

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
                                0,  // offset_color
                                0,  // data_size_for_sort_dma
                                0); // next_address_for_sort_dma
  parameter.append<ta_vertex_parameter::sprite_type_0>() =
    ta_vertex_parameter::sprite_type_0(para_control::para_type::vertex_parameter,
				       quad_vertices[0].x,
				       quad_vertices[0].y,
				       quad_vertices[0].z,
				       quad_vertices[1].x,
				       quad_vertices[1].y,
				       quad_vertices[1].z,
				       quad_vertices[2].x,
				       quad_vertices[2].y,
				       quad_vertices[2].z,
				       quad_vertices[3].x,
				       quad_vertices[3].y);
  // curiously, there is no quad_vertices[3].z in vertex_sprite_type_0

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff220000);
}

void main()
{
  serial::init(0);
  serial::string("main\n");
  video_output::set_mode_vga();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t ta_parameter_buf[32 + 64 + 32 / 4] __attribute__((aligned(32)));

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

  serial::string("while true\n");

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
			      ta_alloc,
			      640 / 32,
			      480 / 32);
    uint32_t ta_parameter_size = transform(ta_parameter_buf);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
