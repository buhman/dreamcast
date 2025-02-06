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
#include "model/elizabeth/elizabeth_mat_emissive.data.h"
#include "model/elizabeth/elizabeth_sword_mat_emissive.data.h"
#include "model/elizabeth/material.h"
#include "model/elizabeth/model.h"

using vec3 = vec<3, float>;
using vec2 = vec<2, float>;

const float degree = 0.017453292519943295;
static float theta = 0;
static int frame = 0;

static inline vec3 transform_vertex(const vec3 vec, const vec3 translate)
{
  float xm = -vec.x;
  float ym = -vec.y;
  float zm =  vec.z;

  float x0 = xm * cos(theta) - zm * sin(theta);
  float y0 = ym;
  float z0 = xm * sin(theta) + zm * cos(theta);

  float x1 = x0 + translate.x;
  float y1 = y0 + translate.y;
  float z1 = z0 + translate.z;

  float x2 = x1;
  float y2 = y1 + 1;
  float z2 = z1 + 1.2;

  float x3 = x2 / z2;
  float y3 = y2 / z2;
  float z3 = 1.0 / z2;

  float x = x3 * 240 + 320;
  float y = y3 * 240 + 320 - 50;
  float z = z3;

  return {x, y, z};
}

static inline vec2 transform_uv(vec2 uv)
{

  float x = uv.x;
  float y = uv.y;

  return {x, y};
}

const uint32_t base_color = 0xa0000000;

static inline void transfer_triangle(const vertex_position * position,
                                     const vertex_texture * texture,
                                     const union triangle * triangle,
                                     const vec3 translate)
{
  vec3 v1 = transform_vertex(position[triangle->a.position], translate);
  vec2 uv1 = transform_uv(texture[triangle->a.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[triangle->b.position], translate);
  vec2 uv2 = transform_uv(texture[triangle->b.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[triangle->c.position], translate);
  vec2 uv3 = transform_uv(texture[triangle->c.texture]);
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
                                          const union quadrilateral * quadrilateral,
                                          const vec3 translate)
{
  vec3 v1 = transform_vertex(position[quadrilateral->a.position], translate);
  vec2 uv1 = transform_uv(texture[quadrilateral->a.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v2 = transform_vertex(position[quadrilateral->b.position], translate);
  vec2 uv2 = transform_uv(texture[quadrilateral->b.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v4 = transform_vertex(position[quadrilateral->d.position], translate);
  vec2 uv4 = transform_uv(texture[quadrilateral->d.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v4.x, v4.y, v4.z,
                                        uv4.x, uv4.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

  vec3 v3 = transform_vertex(position[quadrilateral->c.position], translate);
  vec2 uv3 = transform_uv(texture[quadrilateral->c.texture]);
  *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.x, uv3.y,
                                        base_color,
                                        0); // offset_color
  sq_transfer_32byte(ta_fifo_polygon_converter);

}

static inline void transfer_triangles(const struct model * model,
                                      const struct material_descriptor * material,
                                      const struct object * object,
                                      const uint32_t list_type,
                                      const uint32_t blending,
                                      const uint32_t pixel_format,
                                      const vec3 translate)
{
  if (object->triangle_count == 0 && object->quadrilateral_count == 0)
    return;

  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list_type
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = blending
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_address = texture_memory_alloc.texture.start + material[object->material].pixel.vram_offset;
  const uint32_t texture_control_word = pixel_format
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
    transfer_triangle(model->position, model->texture, &object->triangle[i], translate);
  }
  for (int i = 0; i < object->quadrilateral_count; i++) {
    transfer_quadrilateral(model->position, model->texture, &object->quadrilateral[i], translate);
  }
}

void transfer_scene()
{
  const struct model * model = &elizabeth_model;
  const struct material_descriptor * material = elizabeth_material;

  // opaque
  {
    const uint32_t list_type = para_control::list_type::opaque;
    const uint32_t blending = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::zero
                            | tsp_instruction_word::texture_u_size::from_int(128)
                            | tsp_instruction_word::texture_v_size::from_int(128);
    const uint32_t pixel_format = texture_control_word::pixel_format::_1555;

    const vec3 translate = {-0.3, -0.2, 0};

    transfer_triangles(model, material,
                       &elizabeth_elizabeth_opaque,
                       list_type,
                       blending,
                       pixel_format, translate);

    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }

  // punch through
  {
    const uint32_t list_type = para_control::list_type::punch_through;
    const uint32_t blending = tsp_instruction_word::src_alpha_instr::src_alpha
                            | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                            | tsp_instruction_word::texture_u_size::from_int(128)
                            | tsp_instruction_word::texture_v_size::from_int(128);
    const uint32_t pixel_format = texture_control_word::pixel_format::_1555;

    const vec3 translate = {-0.3, -0.2, 0};

    transfer_triangles(model, material,
                       &elizabeth_elizabeth_punchthrough,
                       list_type,
                       blending,
                       pixel_format,
                       translate);

    const uint32_t blending_sword = tsp_instruction_word::src_alpha_instr::src_alpha
                                  | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                  | tsp_instruction_word::texture_u_size::from_int(32)
                                  | tsp_instruction_word::texture_v_size::from_int(64);

    const vec3 translate_sword = {1, -0.8, 0};

    transfer_triangles(model, material,
                       &elizabeth_elizabeth_sword,
                       list_type,
                       blending_sword,
                       pixel_format,
                       translate_sword);

    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffc0) / 4];
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
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  for (uint32_t i = 0; i < (sizeof (elizabeth_material)) / (sizeof (elizabeth_material[0])); i++) {
    const struct pixel_descriptor * pixel = &elizabeth_material[i].pixel;

    uint32_t offset = texture_memory_alloc.texture.start + pixel->vram_offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(pixel->start);
    transfer_ta_fifo_texture_memory_32byte(dst, src, pixel->width * pixel->height * 2);
  }
}

void main()
{
  serial::init(0);
  transfer_textures();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::_16x4byte
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
      .punch_through = 16 * 4
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
			0xff9090c0);

  frame = 0;

  while (1) {
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[0].start,
			       texture_memory_alloc.isp_tsp_parameters[0].end,
			       texture_memory_alloc.object_list[0].start,
			       texture_memory_alloc.object_list[0].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    transfer_scene();
    ta_wait_punch_through_list();

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
    theta += degree / 2;
  }
  serial::string("return\nreturn\nreturn\n");
}
