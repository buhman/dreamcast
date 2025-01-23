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

#include "memorymap.hpp"

struct vertex {
  float x;
  float y;
  float z;
  unsigned int base_color;
};

// screen space coordinates
const struct vertex triangle_vertices[] = {
  { 320.000f,   50.f, 0.1f, 0xffff0000 },
  { 539.393f,  430.f, 0.1f, 0xff00ff00 },
  { 100.607f,  430.f, 0.1f, 0xff0000ff },
};

void transfer_triangle()
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        0,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  for (int i = 0; i < 3; i++) {
    float x = triangle_vertices[i].x;
    float y = triangle_vertices[i].y;
    float z = triangle_vertices[i].z;
    int base_color = triangle_vertices[i].base_color;

    bool end_of_strip = i == 2;

    *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
      ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          base_color);
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }

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
  serial::string("return\nreturn\nreturn\n");
}
