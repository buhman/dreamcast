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

#include "memory.hpp"

#include "model/model.h"
#include "model/material.h"
#include "model/testscene/texture/texBrick.data.h"
#include "model/testscene/texture/texFoliage.data.h"
#include "model/testscene/texture/texGrass.data.h"
#include "model/testscene/texture/texGrassClump.data.h"
#include "model/testscene/texture/texRock.data.h"
#include "model/testscene/texture/texWater.data.h"
#include "model/testscene/material.h"
#include "model/testscene/model.h"

using vec3 = vec<3, float>;
using vec2 = vec<2, float>;

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

static inline void transfer_triangle(const vertex_position * position,
                                     const vertex_texture * texture,
                                     const union triangle * triangle)
{
  base_color ^= base_color << 13;
  base_color ^= base_color >> 17;
  base_color ^= base_color << 5;

  vec3 v1 = transform_vertex(position[triangle->a.position]);
  vec2 uv1 = texture[triangle->a.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[triangle->b.position]);
  vec2 uv2 = texture[triangle->a.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[triangle->c.position]);
  vec2 uv3 = texture[triangle->c.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.x, uv3.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

static inline void transfer_quadrilateral(const vertex_position * position,
                                          const vertex_texture * texture,
                                          const union quadrilateral * quadrilateral)
{
  base_color ^= base_color << 13;
  base_color ^= base_color >> 17;
  base_color ^= base_color << 5;

  vec3 v1 = transform_vertex(position[quadrilateral->a.position]);
  vec2 uv1 = texture[quadrilateral->a.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[quadrilateral->b.position]);
  vec2 uv2 = texture[quadrilateral->b.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v4 = transform_vertex(position[quadrilateral->d.position]);
  vec2 uv4 = texture[quadrilateral->d.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v4.x, v4.y, v4.z,
                                        uv4.x, uv4.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[quadrilateral->c.position]);
  vec2 uv3 = texture[quadrilateral->c.texture];
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.x, uv3.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

}

static inline void transfer_triangles(const struct model * model, const struct object * object)
{
  if (object->triangle_count == 0)
    return;

  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(128)
                                      | tsp_instruction_word::texture_v_size::from_int(128);

  const uint32_t texture_address = texture_memory_alloc.texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

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
    transfer_triangle(model->position, model->texture, &object->triangle[i]);
  }
  for (int i = 0; i < object->quadrilateral_count; i++) {
    transfer_quadrilateral(model->position, model->texture, &object->quadrilateral[i]);
  }
}

void transfer_scene()
{
  const struct model * model = &testscene_model;
  const struct object * object = &testscene_Waterfall;
  transfer_triangles(model, object);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(dst) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(dst) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[texture_memory_alloc.texture.start / 4];
  uint32_t * src32 = reinterpret_cast<uint32_t *>(src);

  length = (length + 31) & ~31; // round up to nearest multiple of 32
  while (length > 0) {
    base[0] = src32[0];
    base[1] = src32[1];
    base[2] = src32[2];
    base[3] = src32[3];
    base[4] = src32[4];
    base[5] = src32[5];
    base[6] = src32[6];
    base[7] = src32[7];
    asm volatile ("pref @%0"
                  :                // output
                  : "r" (&base[0]) // input
                  : "memory");
    serial::integer<uint32_t>((uint32_t)base, ' ');
    serial::integer<uint32_t>((uint32_t)src32, ' ');
    serial::integer<uint32_t>(length);
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  void * dst = reinterpret_cast<void *>(ta_fifo_texture_memory);
  void * src = reinterpret_cast<void *>(&_binary_model_testscene_texture_texBrick_data_start);
  transfer_ta_fifo_texture_memory_32byte(dst, src, 128 * 128 * 2);

  //memory::copy<volatile uint32_t>(&texture_memory64[texture_memory_alloc.texture.start / 4], reinterpret_cast<uint32_t *>(src), 128 * 128 * 2);
}

void main()
{
  transfer_textures();

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
