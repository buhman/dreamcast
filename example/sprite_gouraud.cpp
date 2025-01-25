#include <stdint.h>

#include "holly/background.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/texture_memory_alloc2.hpp"
#include "holly/video_output.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "systembus.hpp"

#include "memorymap.hpp"

struct vertex {
  float x;
  float y;
  float z;
  unsigned int base_color;
};

// screen space coordinates
const struct vertex quad_vertices[] = {
  { 100.607f,   50.f, 0.1f, 0xff0000ff },
  { 539.393f,   50.f, 0.1f, 0xff00ff00 },
  { 539.393f,  430.f, 0.1f, 0xff00ff00 },
  { 100.607f,  430.f, 0.1f, 0xff0000ff },
};

void transfer_triangle()
{
  const uint32_t parameter_control_word = para_control::para_type::sprite
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;
    //| obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;

  const uint32_t base_color = 0xffff0000;
  *reinterpret_cast<ta_global_parameter::sprite *>(store_queue) =
    ta_global_parameter::sprite(parameter_control_word,
                                isp_tsp_instruction_word,
                                tsp_instruction_word,
                                texture_control_word,
                                base_color,
                                0,  // offset_color
                                0,  // data_size_for_sort_dma
                                0); // next_address_for_sort_dma
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::sprite_type_0 *>(store_queue) =
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
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void main()
{
  const uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                          | ta_alloc_ctrl::tm_opb::no_list
                          | ta_alloc_ctrl::t_opb::no_list
                          | ta_alloc_ctrl::om_opb::no_list
                          | ta_alloc_ctrl::o_opb::_16x4byte;

  const int render_passes = 1;
  const struct opb_size opb_size[render_passes] = {
    {
      .opaque = 16 * 4,
      .opaque_modifier = 0,
      .translucent = 0,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  video_output::set_mode_vga();

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array[0].start,
			 texture_memory_alloc::object_list[0].start);

  background_parameter2(texture_memory_alloc::background[0].start,
			0xff220033);


  while (1) {
    ta_polygon_converter_init2(texture_memory_alloc::isp_tsp_parameters[0].start,
			       texture_memory_alloc::isp_tsp_parameters[0].end,
			       texture_memory_alloc::object_list[0].start,
			       texture_memory_alloc::object_list[0].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    transfer_triangle();
    ta_wait_opaque_list();

    //0x80400000
    volatile uint32_t * param_base = &texture_memory32[texture_memory_alloc::isp_tsp_parameters[0].start / 4];
    param_base[0] |= isp_tsp_instruction_word::gouraud_shading;
    //param_base[1]; // tsp
    //param_base[2]; // texture
    //param_base[3]; // x
    //param_base[4]; // y
    //param_base[5]; // z
    param_base[6] = quad_vertices[0].base_color; // base_color
    //param_base[7]; // x
    //param_base[8]; // y
    //param_base[9]; // z
    param_base[10] = quad_vertices[1].base_color; // base_color
    //param_base[11]; // x
    //param_base[12]; // y
    //param_base[13]; // z
    //param_base[14] = quad_vertices[2].base_color; // base_color
    //param_base[15]; // x
    //param_base[16]; // y
    //param_base[17]; // z
    //param_base[18] = quad_vertices[3].base_color; // base_color

    core_start_render2(texture_memory_alloc::region_array[0].start,
                       texture_memory_alloc::isp_tsp_parameters[0].start,
                       texture_memory_alloc::background[0].start,
                       texture_memory_alloc::framebuffer[0].start,
                       framebuffer_width);

    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc::framebuffer[0].start;
    while (spg_status::vsync(holly.SPG_STATUS));
    break;
  }
  serial::integer<uint32_t>(system.ISTNRM);
  serial::integer<uint32_t>(system.ISTERR);
  serial::string("return\nreturn\nreturn\n");
}
