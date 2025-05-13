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
#include "model/speck/speck.data.h"
#include "model/speck/white.data.h"
#include "model/speck/material.h"
#include "model/speck/model.h"

using vec3 = vec<3, float>;
using vec2 = vec<2, float>;

const float degree = 0.017453292519943295;
static float theta = 0;
static int frame = 0;

static inline vec3 transform_vertex(const vec3 vec, const vec3 translate, const float scale)
{
  float xm = -vec.x * scale;
  float ym = -vec.y * scale;
  float zm =  vec.z * scale;

  float x0 = xm * cos(theta) - zm * sin(theta);
  float y0 = ym;
  float z0 = xm * sin(theta) + zm * cos(theta);

  /*
  float x1 = x0 + translate.x;
  float y1 = y0 + translate.y;
  float z1 = z0 + translate.z;
  */

  float x1 = x0;
  float y1 = y0 * cos(theta) - z0 * sin(theta);
  float z1 = y0 * sin(theta) + z0 * cos(theta);

  float x2 = x1;
  float y2 = y1;
  float z2 = z1 + 1.5;

  float x3 = x2 / z2;
  float y3 = y2 / z2;
  float z3 = 1.0 / z2;

  float x = x3 * 240 + 320;
  float y = y3 * 240 + 320 - 75;
  float z = z3;

  return {x, y, z};
}

static inline vec2 transform_uv(vec2 uv, float scale)
{

  float x = uv.x * 1.3;// + ((float)frame * 0.003) * scale;
  float y = uv.y * 1.3;// + ((float)frame * 0.003) * scale;

  return {x, y};
}

static inline void transfer_triangle(ta_parameter_writer& writer,
                                     const vertex_position * position,
                                     const vertex_texture * texture,
                                     const union triangle * triangle,
                                     const uint32_t base_color,
                                     const vec3 translate,
                                     const float scale)
{
  vec3 v1 = transform_vertex(position[triangle->a.position], translate, scale);
  vec2 uv1 = transform_uv(texture[triangle->a.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color

  vec3 v2 = transform_vertex(position[triangle->b.position], translate, scale);
  vec2 uv2 = transform_uv(texture[triangle->b.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color

  vec3 v3 = transform_vertex(position[triangle->c.position], translate, scale);
  vec2 uv3 = transform_uv(texture[triangle->c.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.x, uv3.y,
                                        base_color,
                                        0); // offset_color
}

static inline void transfer_quadrilateral(ta_parameter_writer& writer,
                                          const vertex_position * position,
                                          const vertex_texture * texture,
                                          const union quadrilateral * quadrilateral,
                                          const uint32_t base_color,
                                          const vec3 translate,
                                          const float scale)
{
  vec3 v1 = transform_vertex(position[quadrilateral->a.position], translate, scale);
  vec2 uv1 = transform_uv(texture[quadrilateral->a.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v1.x, v1.y, v1.z,
                                        uv1.x, uv1.y,
                                        base_color,
                                        0); // offset_color

  vec3 v2 = transform_vertex(position[quadrilateral->b.position], translate, scale);
  vec2 uv2 = transform_uv(texture[quadrilateral->b.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v2.x, v2.y, v2.z,
                                        uv2.x, uv2.y,
                                        base_color,
                                        0); // offset_color

  vec3 v4 = transform_vertex(position[quadrilateral->d.position], translate, scale);
  vec2 uv4 = transform_uv(texture[quadrilateral->d.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        v4.x, v4.y, v4.z,
                                        uv4.x, uv4.y,
                                        base_color,
                                        0); // offset_color

  vec3 v3 = transform_vertex(position[quadrilateral->c.position], translate, scale);
  vec2 uv3 = transform_uv(texture[quadrilateral->c.texture], scale);
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        v3.x, v3.y, v3.z,
                                        uv3.x, uv3.y,
                                        base_color,
                                        0); // offset_color
}

static inline void transfer_triangles(ta_parameter_writer& writer,
                                      const struct model * model,
                                      const struct material_descriptor * material,
                                      const struct object * object,
                                      const uint32_t list_type,
                                      const uint32_t blending,
                                      const uint32_t pixel_format,
                                      const uint32_t base_color,
                                      const vec3 translate,
                                      const float scale)
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

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  for (int i = 0; i < object->triangle_count; i++) {
    transfer_triangle(writer,
                      model->position, model->texture, &object->triangle[i],
                      base_color,
                      translate,
                      scale);
  }
  for (int i = 0; i < object->quadrilateral_count; i++) {
    transfer_quadrilateral(writer,
                           model->position, model->texture, &object->quadrilateral[i],
                           base_color,
                           translate,
                           scale);
  }
}

void transfer_scene(ta_parameter_writer& writer)
{
  const struct model * model = &speck_model;
  const struct material_descriptor * material = speck_material;

  // opaque
  if (1) {
    const uint32_t list_type = para_control::list_type::opaque;
    const uint32_t blending = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::zero
                            | tsp_instruction_word::texture_u_size::from_int(8)
                            | tsp_instruction_word::texture_v_size::from_int(8)
                            | tsp_instruction_word::texture_shading_instruction::modulate;
    const uint32_t pixel_format = texture_control_word::pixel_format::_4444;

    const uint32_t base_color = 0xffffffff;
    const vec3 translate = {0, 0, 0};
    const float scale = 0.83;

    transfer_triangles(writer,
                       model, material,
                       &speck_Cube_white,
                       list_type,
                       blending,
                       pixel_format,
                       base_color,
                       translate,
                       scale);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }

  const float scale[] {
    1.000,
    1.014,
    1.031,
    1.043,
    1.066,
    1.093,
  };

  static const uint32_t red[] {
4278190080,
3791650816,
3338665984,
2936012800,
2550136832,
2214592512,
1912602624,
1627389952,
1375731712,
1157627904,
956301312,
771751936,
620756992,
503316480,
385875968,
285212672,
218103808,
150994944,
100663296,
67108864,
33554432,
16777216,
16777216,
16777216,
  };

  // translucent
  if (1) {
    const uint32_t list_type = para_control::list_type::translucent;
    const uint32_t blending = tsp_instruction_word::src_alpha_instr::src_alpha
                            | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                            | tsp_instruction_word::texture_u_size::from_int(256)
                            | tsp_instruction_word::texture_v_size::from_int(256)
                            | tsp_instruction_word::texture_shading_instruction::modulate_alpha
                            | tsp_instruction_word::use_alpha
                            ;
    const uint32_t pixel_format = texture_control_word::pixel_format::_4444;

    int shells = 23;
    for (int i = 0; i < shells; i++) {
      const uint32_t base_color = red[i] | 0xffffff;
      const vec3 translate = {0, 0, 0};
      const float _scale = 1.0 + (float)(i) * 0.107;

      transfer_triangles(writer,
                         model, material,
                         &speck_Cube,
                         list_type,
                         blending,
                         pixel_format,
                         base_color,
                         translate,
                         _scale);
    }

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffe0) / 4];
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

  for (uint32_t i = 0; i < (sizeof (speck_material)) / (sizeof (speck_material[0])); i++) {
    const struct pixel_descriptor * pixel = &speck_material[i].pixel;

    uint32_t offset = texture_memory_alloc.texture.start + pixel->vram_offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(pixel->start);
    transfer_ta_fifo_texture_memory_32byte(dst, src, pixel->width * pixel->height * 2);
  }
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);
  transfer_textures();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::_16x4byte
			      | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 16 * 4,
      .opaque_modifier = 0,
      .translucent = 16 * 4,
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

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  for (int i = 0; i < 2; i++) {
    region_array_multipass(tile_width,
                           tile_height,
                           opb_size,
                           render_passes,
                           texture_memory_alloc.region_array[i].start,
                           texture_memory_alloc.object_list[i].start);

    background_parameter2(texture_memory_alloc.background[i].start,
                          0xff9090c0);

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[i].start,
                               texture_memory_alloc.isp_tsp_parameters[i].end,
                               texture_memory_alloc.object_list[i].start,
                               texture_memory_alloc.object_list[i].end,
                               opb_size[0].total(),
                               ta_alloc,
                               tile_width,
                               tile_height);
    writer.offset = 0;
    transfer_scene(writer);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);

    if (i == 0)
      ta_wait_translucent_list();
  }

  core_start_render2(texture_memory_alloc.region_array[0].start,
                     texture_memory_alloc.isp_tsp_parameters[0].start,
                     texture_memory_alloc.background[0].start,
                     texture_memory_alloc.framebuffer[0].start,
                     framebuffer_width);

  frame = 1;

  while (1) {
    int ta = frame & 1;
    int core = !ta;

    // 1 -> {0, 0}

    ta_wait_translucent_list();
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    writer.offset = 0;
    transfer_scene(writer);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    core_wait_end_of_render_video();
    ta_polygon_converter_transfer(writer.buf, writer.offset);

    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;

    // 0 ->

    core_start_render2(texture_memory_alloc.region_array[core].start,
                       texture_memory_alloc.isp_tsp_parameters[core].start,
                       texture_memory_alloc.background[core].start,
                       texture_memory_alloc.framebuffer[core].start,
                       framebuffer_width);

    frame += 1;
    theta += degree / 2;
  }
  serial::string("return\nreturn\nreturn\n");
}
