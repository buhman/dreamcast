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

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "sh7091/vbr.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "memorymap.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#include "model/model.h"
#include "model/bear/material.h"
#include "model/bear/model.h"
#include "model/bear/bear.data.h"

void vbr100()
{
  serial::string("vbr100\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);

  uint32_t spc;
  uint32_t ssr;
  asm volatile ("stc spc,%0" : "=r" (spc));
  asm volatile ("stc ssr,%0" : "=r" (ssr));
  serial::string("spc ");
  serial::integer(spc);
  serial::string("ssr ");
  serial::integer(ssr);

  while (1);
}

void vbr400()
{
  serial::string("vbr400\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);

  uint32_t spc;
  uint32_t ssr;
  asm volatile ("stc spc,%0" : "=r" (spc));
  asm volatile ("stc ssr,%0" : "=r" (ssr));
  serial::string("spc ");
  serial::integer(spc);
  serial::string("ssr ");
  serial::integer(ssr);

  while (1);
}

static int render_done = 0;

void vbr600()
{
  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) {
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(system.ISTERR);
    }

    if (istnrm & istnrm::end_of_render_tsp) {
      system.ISTNRM = istnrm::end_of_render_tsp
                    | istnrm::end_of_render_isp
                    | istnrm::end_of_render_video;
      render_done = 1;
      return;
    }
  }

  serial::string("vbr600\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);

  serial::string("istnrm: ");
  serial::integer<uint32_t>(system.ISTNRM);
  serial::string("isterr: ");
  serial::integer<uint32_t>(system.ISTERR);

  uint32_t spc;
  uint32_t ssr;
  asm volatile ("stc spc,%0" : "=r" (spc));
  asm volatile ("stc ssr,%0" : "=r" (ssr));
  serial::string("spc ");
  serial::integer(spc);
  serial::string("ssr ");
  serial::integer(ssr);

  while (1);
}

void interrupt_init()
{
  system.IML2NRM = 0;
  system.IML2ERR = 0;
  system.IML2EXT = 0;

  system.IML4NRM = 0;
  system.IML4ERR = 0;
  system.IML4EXT = 0;

  system.IML6NRM = 0;
  system.IML6ERR = 0;
  system.IML6EXT = 0;

  system.ISTERR = 0xffffffff;
  system.ISTNRM = 0xffffffff;

  sh7091.CCN.INTEVT = 0;
  sh7091.CCN.EXPEVT = 0;

  uint32_t vbr = reinterpret_cast<uint32_t>(&__vbr_link_start) - 0x100;
  serial::string("vbr ");
  serial::integer<uint32_t>(vbr);
  serial::string("vbr100 ");
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&vbr100));

  asm volatile ("ldc %0,vbr"
		:
		: "r" (vbr));

  uint32_t sr;
  asm volatile ("stc sr,%0"
		: "=r" (sr));
  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  sr &= ~sh::sr::bl; // BL
  sr &= ~sh::sr::imask(15); // imask

  serial::string("sr ");
  serial::integer<uint32_t>(sr);

  asm volatile ("ldc %0,sr"
		:
		: "r" (sr));
}

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t list,
                           uint32_t cull,
                           vec3 color)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud
                                        ;
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      ;

  const uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // offset color
                                        0  // sort dma
                                        );
}

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t texture_address,
                           uint32_t list,
                           uint32_t cull,
                           vec3 color)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control::texture
                                        ;
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::texture_shading_instruction::modulate
                                      | tsp_instruction_word::texture_u_size::from_int(512)
                                      | tsp_instruction_word::texture_v_size::from_int(512)
                                      ;

  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  const float alpha = 1.0f;

  writer.append<ta_global_parameter::polygon_type_1>() =
    ta_global_parameter::polygon_type_1(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        alpha,
                                        color.x,
                                        color.y,
                                        color.z
                                        );
}

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480 / 2.0;

  return {
    v.x / v.z * dim + 640 / 2.0f,
    v.y / v.z * dim + 480 / 2.0f,
    1 / v.z,
  };
}

static inline void render_quad_type_0(ta_parameter_writer& writer,
                                      vec3 ap,
                                      vec3 bp,
                                      vec3 cp,
                                      vec3 dp,
                                      uint32_t base_color)
{
  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        base_color);
}

static inline void render_quad_type_7(ta_parameter_writer& writer,
                                      vec3 ap,
                                      vec3 bp,
                                      vec3 cp,
                                      vec3 dp,
                                      vec2 at,
                                      vec2 bt,
                                      vec2 ct,
                                      vec2 dt,
                                      float ai,
                                      float bi,
                                      float ci,
                                      float di)
{
  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        ai,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        bi,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        di,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        ci,
                                        0);
}

constexpr inline mat4x4 screen_rotation(float theta)
{
  //float zt = -0.7853981633974483 + (0.2);
  float zt = -0.7853981633974483 * 0;
  float yt = -0.7853981633974483 * theta;
  float xt = 0.7853981633974483 * 4;
  //float xt = 0.7853981633974483 * 3.7;

  mat4x4 rx = {
    1, 0, 0, 0,
    0, cos(xt), -sin(xt), 0,
    0, sin(xt), cos(xt), 0,
    0, 0, 0, 1,
  };

  mat4x4 ry = {
     cos(yt), 0, sin(yt), 0,
    0, 1, 0, 0,
    -sin(yt), 0, cos(yt), 0,
    0, 0, 0, 1,
  };

  mat4x4 rz = {
    cos(zt), -sin(zt), 0, 0,
    sin(zt), cos(zt), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  mat4x4 t = {
    1, 0, 0, 0,
    0, 1, 0, 6.5,
    0, 0, 1, 8.5,
    0, 0, 0, 1,
  };

  return t * ry * rx * rz;
}

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

static inline float inverse_length(vec3 v)
{
  float f = dot(v, v);
  return _fsrra(f);
}

float light_intensity(vec3 light_vec, mat4x4& trans, vec3 n0)
{
  vec4 n1 = trans * (vec4){n0.x, n0.y, n0.z, 0.f}; // no translation component
  vec3 n = {n1.x, n1.y, n1.z};
  float n_dot_l = dot(n, light_vec);

  float intensity = 0.5f;
  if (n_dot_l > 0) {
    intensity += 0.5f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
    if (intensity > 1.0f)
      intensity = 1.0f;
  }
  return intensity;
}

static int animation_tick = 0;
constexpr int animation_frames = 4;
constexpr int ticks_per_animation_frame = 16;
constexpr float tick_div = 1.0f / (float)ticks_per_animation_frame;

void render_object_intensity(ta_parameter_writer& writer, const mat4x4& model_trans, const mat4x4& screen,
                             const struct model * model,
                             const struct object * object0,
                             const struct object * object1)
{
  int frame_ix0 = animation_tick / ticks_per_animation_frame;
  float lerp = (float)(animation_tick - (frame_ix0 * ticks_per_animation_frame)) * tick_div;

  vec3 light_vec = {20, 1, -20};

  mat4x4 trans = screen * model_trans;

  for (int i = 0; i < object0->quadrilateral_count; i++) {
    const union quadrilateral * quad0 = &object0->quadrilateral[i];
    const union quadrilateral * quad1 = &object1->quadrilateral[i];
    vec3 a0 = model->position[quad0->v[0].position];
    vec3 b0 = model->position[quad0->v[1].position];
    vec3 c0 = model->position[quad0->v[2].position];
    vec3 d0 = model->position[quad0->v[3].position];

    vec3 a1 = model->position[quad1->v[0].position];
    vec3 b1 = model->position[quad1->v[1].position];
    vec3 c1 = model->position[quad1->v[2].position];
    vec3 d1 = model->position[quad1->v[3].position];

    vec3 a = trans * (a0 + ((a1 - a0) * lerp));
    vec3 b = trans * (b0 + ((b1 - b0) * lerp));
    vec3 c = trans * (c0 + ((c1 - c0) * lerp));
    vec3 d = trans * (d0 + ((d1 - d0) * lerp));

    vec2 at = model->texture[quad0->v[0].texture] * (vec2){1, -1};
    vec2 bt = model->texture[quad0->v[1].texture] * (vec2){1, -1};
    vec2 ct = model->texture[quad0->v[2].texture] * (vec2){1, -1};
    vec2 dt = model->texture[quad0->v[3].texture] * (vec2){1, -1};

    vec3 an0 = model->normal[quad0->v[0].normal];
    vec3 bn0 = model->normal[quad0->v[1].normal];
    vec3 cn0 = model->normal[quad0->v[2].normal];
    vec3 dn0 = model->normal[quad0->v[3].normal];

    vec3 an1 = model->normal[quad1->v[0].normal];
    vec3 bn1 = model->normal[quad1->v[1].normal];
    vec3 cn1 = model->normal[quad1->v[2].normal];
    vec3 dn1 = model->normal[quad1->v[3].normal];

    vec3 an = (an0 + ((an1 - an0) * lerp));
    vec3 bn = (bn0 + ((bn1 - bn0) * lerp));
    vec3 cn = (cn0 + ((cn1 - cn0) * lerp));
    vec3 dn = (dn0 + ((dn1 - dn0) * lerp));

    float ai = light_intensity(light_vec, trans, an);
    float bi = light_intensity(light_vec, trans, bn);
    float ci = light_intensity(light_vec, trans, cn);
    float di = light_intensity(light_vec, trans, dn);

    render_quad_type_7(writer,
                       screen_transform(a),
                       screen_transform(b),
                       screen_transform(c),
                       screen_transform(d),
                       at,
                       bt,
                       ct,
                       dt,
                       ai,
                       bi,
                       ci,
                       di);
  }
}

void render_object_packed(ta_parameter_writer& writer, const mat4x4& model_trans, const mat4x4& screen,
                          const struct model * model, const struct object * object)
{
  mat4x4 trans = screen * model_trans;
  uint32_t base_color = 0xffff0000;

  for (int i = 0; i < object->quadrilateral_count; i++) {
    const union quadrilateral * quad = &object->quadrilateral[i];
    vec3 a = trans * model->position[quad->v[0].position];
    vec3 b = trans * model->position[quad->v[1].position];
    vec3 c = trans * model->position[quad->v[2].position];
    vec3 d = trans * model->position[quad->v[3].position];

    render_quad_type_0(writer,
                       screen_transform(a),
                       screen_transform(b),
                       screen_transform(c),
                       screen_transform(d),
                       base_color);
  }
}

void transfer_scene(ta_parameter_writer& writer)
{
  const float deg = 0.017453292519943295 / 4;
  static float theta = deg;
  const mat4x4 screen = screen_rotation(theta);

  theta += deg;

  int frame_ix0 = animation_tick / ticks_per_animation_frame;
  int frame_ix1 = frame_ix0 + 1;
  if (frame_ix1 >= animation_frames)
    frame_ix1 = 0;

  float scale = 2.0f;
  float translate = 0.f;
  const mat4x4 model_trans = {
    scale, 0, 0, 0,
    0, scale, 0, 0,
    0, 0, -scale, translate,
    0, 0, 0, 1,
  };

  // opaque
  {
    global_polygon_type_1(writer, texture_memory_alloc.texture.start,
                          para_control::list_type::opaque,
                          isp_tsp_instruction_word::culling_mode::cull_if_negative,
                          {1, 1, 1});

    const struct model * model = &bear_model;
    const struct object * object0 = bear_object[frame_ix0 * 2];
    const struct object * object1 = bear_object[frame_ix1 * 2];

    render_object_intensity(writer, model_trans, screen, model, object0, object1);
  }

  {
    global_polygon_type_1(writer, texture_memory_alloc.texture.start,
                          para_control::list_type::opaque,
                          isp_tsp_instruction_word::culling_mode::cull_if_negative,
                          {0, 0, 0});

    const struct model * model = &bear_model;
    const struct object * object0 = bear_object[(frame_ix0 * 2) + 1];
    const struct object * object1 = bear_object[(frame_ix1 * 2) + 1];

    render_object_intensity(writer, model_trans, screen, model, object0, object1);
  }

  // end of opaque list
  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  // increment tick
  animation_tick += 1;
  if (animation_tick >= animation_frames * ticks_per_animation_frame)
    animation_tick = 0;
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

  {
    uint32_t offset = texture_memory_alloc.texture.start;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(&_binary_model_bear_bear_data_start);
    uint32_t size = reinterpret_cast<uint32_t>(&_binary_model_bear_bear_data_size);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);

  serial::integer<uint32_t>((sizeof (bear_position)) / (sizeof (bear_position[0])));

  interrupt_init();

  constexpr uint32_t ta_alloc = 0
                              | ta_alloc_ctrl::pt_opb::no_list
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

  system.IML6NRM = istnrm::end_of_render_tsp;

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  for (int i = 0; i < 2; i++) {
    region_array_multipass(tile_width,
                           tile_height,
                           opb_size,
                           render_passes,
                           texture_memory_alloc.region_array[i].start,
                           texture_memory_alloc.object_list[i].start);

    background_parameter2(texture_memory_alloc.background[i].start,
                          0xff202040);
  }

  int ta = 0;
  int core = 0;

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  transfer_textures();
  video_output::set_mode_vga();

  while (1) {
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
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_list();

    render_done = 0;
    core_start_render2(texture_memory_alloc.region_array[core].start,
                       texture_memory_alloc.isp_tsp_parameters[core].start,
                       texture_memory_alloc.background[core].start,
                       texture_memory_alloc.framebuffer[core].start,
                       framebuffer_width);
    while (render_done == 0) {
      asm volatile ("nop");
    };

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;
    while (spg_status::vsync(holly.SPG_STATUS));
  }
}
