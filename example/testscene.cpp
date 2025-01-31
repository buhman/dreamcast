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
#include "holly/texture_memory_alloc3.hpp"
#include "holly/video_output.hpp"

#include "sh7091/serial.hpp"
#include "sh7091/store_queue.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "model/model.h"
#include "model/testscene/model.h"

using vec3 = vec<3, float>;

static float theta = 0;

static inline vec3 transform_vertex(vec3 vec)
{
  float x9 = vec.x;
  float y9 = vec.y;
  float z9 = vec.z;

  float x0 = x9 * cos(theta) - z9 * sin(theta);
  float y0 = y9;
  float z0 = x9 * sin(theta) + z9 * cos(theta);

  float x1 = x0;
  float y1 = y0 * cos(theta) - z0 * sin(theta);
  float z1 = y0 * sin(theta) + z0 * cos(theta);

  float x2 = x1;
  float y2 = y1;
  float z2 = z1 + 4.5;

  float x3 = x2 / z2;
  float y3 = y2 / z2;
  float z3 = 1.0 / z2;

  float x = x3 * 240 + 320;
  float y = y3 * 240 + 320;
  float z = z3;

  return {x, y, z};
}

static uint32_t base_color = 0xffc0c000;

static inline void transfer_triangle(vertex_position * position, union triangle * triangle)
{
  base_color ^= base_color << 13;
  base_color ^= base_color >> 17;
  base_color ^= base_color << 5;

  vec3 v1 = transform_vertex(position[triangle->a.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[triangle->b.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        0xffc0c000);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[triangle->c.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

static inline void transfer_quadrilateral(vertex_position * position, union quadrilateral * quadrilateral)
{
  base_color ^= base_color << 13;
  base_color ^= base_color >> 17;
  base_color ^= base_color << 5;

  vec3 v1 = transform_vertex(position[quadrilateral->a.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[quadrilateral->b.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[quadrilateral->c.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v3.x, v3.y, v3.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v4 = transform_vertex(position[quadrilateral->d.position]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        v4.x, v4.y, v4.z,
                                        base_color);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

static inline void transfer_triangles(struct model * model, struct object * object)
{
  if (object->triangle_count == 0)
    return;

  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);

  for (int i = 0; i < object->triangle_count; i++) {
    transfer_triangle(model->position, &object->triangle[i]);
  }
  for (int i = 0; i < object->quadrilateral_count; i++) {
    transfer_quadrilateral(model->position, &object->quadrilateral[i]);
  }
}

void transfer_scene()
{
  struct model * model = &testscene_model;
  struct object * object = &testscene_Waterfall;
  transfer_triangles(model, object);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void main()
{
  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
			      | ta_alloc_ctrl::t_opb::no_list
			      | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
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
			 texture_memory_alloc.region_array[0].start,
			 texture_memory_alloc.object_list[0].start);

  background_parameter2(texture_memory_alloc.background[0].start,
			0xff220033);


  const float degree = 0.017453292519943295;
  int frame = 0;

  while (1) {
    base_color = 0xffc0c000;

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[0].start,
			       texture_memory_alloc.isp_tsp_parameters[0].end,
			       texture_memory_alloc.object_list[0].start,
			       texture_memory_alloc.object_list[0].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    transfer_scene();
    ta_wait_opaque_list();

    core_start_render2(texture_memory_alloc.region_array[0].start,
                       texture_memory_alloc.isp_tsp_parameters[0].start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[0].start,
                       framebuffer_width);

    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[0].start;
    while (spg_status::vsync(holly.SPG_STATUS));

    frame += 1;
    theta += degree;
    if (frame > 300)
      break;
  }
  serial::string("return\nreturn\nreturn\n");
}
