#include <stdint.h>
#include <bit>

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

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "memorymap.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"
#include "math/geometry.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#include "model/model.h"
#include "model/haunted_mansion/material.h"
#include "model/haunted_mansion/model_female.h"
#include "model/haunted_mansion/model_mansion.h"
#include "model/haunted_mansion/model_cone.h"
#include "model/haunted_mansion/model_cube.h"

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

static ft0::data_transfer::data_format data[4];

uint8_t send_buf[1024] __attribute__((aligned(32)));
uint8_t recv_buf[1024] __attribute__((aligned(32)));

void do_get_condition()
{
  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft0::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  for (int port = 0; port < 4; port++) {
    auto& data_fields = host_command[port].bus_data.data_fields;
    data_fields.function_type = std::byteswap(function_type::controller);
  }
  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    if (bus_data.command_code != response_type::command_code) {
      return;
    }
    auto& data_fields = bus_data.data_fields;
    if ((std::byteswap(data_fields.function_type) & function_type::controller) == 0) {
      return;
    }

    data[port].digital_button = data_fields.data.digital_button;
    for (int i = 0; i < 6; i++) {
      data[port].analog_coordinate_axis[i]
        = data_fields.data.analog_coordinate_axis[i];
    }
  }
}

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t texture_address,
                           uint32_t list,
                           uint32_t cull)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control::shadow
                                        ;
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      ;

  const uint32_t texture_control_word = 0;

  const float alpha = 1.0f;
  const float r = 0.6f;
  const float g = 0.6f;
  const float b = 0.6f;

  writer.append<ta_global_parameter::polygon_type_1>() =
    ta_global_parameter::polygon_type_1(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        alpha,
                                        r,
                                        g,
                                        b
                                        );
}

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480 / 2.0;

  return {
    v.x / (1.f * v.z) * dim + 640 / 2.0f,
    v.y / (1.f * v.z) * dim + 480 / 2.0f,
    1 / v.z,
  };
}

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               float ai,
                               float bi,
                               float ci,
                               float di)
{
  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        ai);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bi);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        di);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ci);
}

static inline void render_tri(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               float ai,
                               float bi,
                               float ci)
{
  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        ai);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bi);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ci);
}

constexpr inline mat4x4 screen_rotation(float theta, float x, float y)
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
    1, 0, 0, 0.0f + x,
    0, 1, 0, 1,
    0, 0, 1, 1.2f + y,
    0, 0, 0, 1,
  };

  return ry * t * rx * rz;
}

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

static inline float inverse_length(vec3 v)
{
  float f = dot(v, v);
  return _fsrra(f);
}

float light_intensity(vec3 light_vec, vec3 n)
{
  float n_dot_l = dot(n, light_vec);

  float intensity = 0.5f;
  if (n_dot_l > 0) {
    intensity += 0.5f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
    if (intensity > 1.0f)
      intensity = 1.0f;
  }
  return intensity;
}


static inline void render_clip_tri(ta_parameter_writer& writer, vec3& light_vec, vec3 * preclip_position, vec3 * preclip_normal)
{
  const vec3 plane_point = {0.f, 0.f, 1.f};
  const vec3 plane_normal = {0.f, 0.f, 1.f};

  vec3 clip_position[4];
  vec3 clip_normal[4];
  int output_length = geometry::clip_polygon_uv<3>(clip_position,
                                                   clip_normal,
                                                   plane_point,
                                                   plane_normal,
                                                   preclip_position,
                                                   preclip_normal);

  float ai;
  float bi;
  float ci;
  float di;

  vec3 ap;
  vec3 bp;
  vec3 cp;
  vec3 dp;

  if (output_length >= 3) {
    // 012
    ap = screen_transform(clip_position[0]);
    bp = screen_transform(clip_position[1]);
    cp = screen_transform(clip_position[2]);

    ai = light_intensity(light_vec, clip_normal[0]);
    bi = light_intensity(light_vec, clip_normal[1]);
    ci = light_intensity(light_vec, clip_normal[2]);

    render_tri(writer,
               ap,
               bp,
               cp,
               ai,
               bi,
               ci);
  }
  if (output_length >= 4) {
    // 023
    dp = screen_transform(clip_position[3]);

    di = light_intensity(light_vec, clip_normal[3]);

    render_tri(writer,
               ap,
               cp,
               dp,
               ai,
               ci,
               di);
  }
}

static inline vec3 light_trans(mat4x4& trans, vec3 normal)
{
  vec4 n = trans * (vec4){normal.x, normal.y, normal.z, 0.f}; // no translation component
  return {n.x, n.y, n.z};
}

static inline void transform_quad(ta_parameter_writer& writer, mat4x4& trans, vec3& light_vec, const union quadrilateral * quad, const vertex_position * position, const vertex_normal * normal)
{
  vec3 a = trans * position[quad->v[0].position];
  vec3 b = trans * position[quad->v[1].position];
  vec3 c = trans * position[quad->v[2].position];
  vec3 d = trans * position[quad->v[3].position];

  if (a.z < 0 && b.z < 0 && c.z < 0 && d.z < 0) {
    return;
  }

  vec3 an = light_trans(trans, normal[quad->v[0].normal]);
  vec3 bn = light_trans(trans, normal[quad->v[1].normal]);
  vec3 cn = light_trans(trans, normal[quad->v[2].normal]);
  vec3 dn = light_trans(trans, normal[quad->v[3].normal]);

  if (a.z < 0 || b.z < 0 || c.z < 0 || d.z < 0) {
    // abd
    // dbc
    {
      vec3 preclip_position[3] = {a, b, d};
      vec3 preclip_normal[3] = {an, bn, dn};

      render_clip_tri(writer, light_vec, preclip_position, preclip_normal);
    }
    {
      vec3 preclip_position[3] = {d, b, c};
      vec3 preclip_normal[3] = {dn, bn, cn};

      render_clip_tri(writer, light_vec, preclip_position, preclip_normal);
    }
  } else {
    float ai = light_intensity(light_vec, an);
    float bi = light_intensity(light_vec, bn);
    float ci = light_intensity(light_vec, cn);
    float di = light_intensity(light_vec, dn);

    render_quad(writer,
                screen_transform(a),
                screen_transform(b),
                screen_transform(c),
                screen_transform(d),
                ai,
                bi,
                ci,
                di);
  }
}

static inline void transform_tri(ta_parameter_writer& writer, mat4x4& trans, vec3& light_vec, const union triangle * tri, const vertex_position * position, const vertex_normal * normal)
{
  vec3 a = trans * position[tri->v[0].position];
  vec3 b = trans * position[tri->v[1].position];
  vec3 c = trans * position[tri->v[2].position];

  if (a.z < 0 && b.z < 0 && c.z < 0) return;

  vec3 an = light_trans(trans, normal[tri->v[0].normal]);
  vec3 bn = light_trans(trans, normal[tri->v[1].normal]);
  vec3 cn = light_trans(trans, normal[tri->v[2].normal]);

  if (a.z < 0 || b.z < 0 || c.z < 0) {
    vec3 preclip_position[3] = {a, b, c};
    vec3 preclip_normal[3] = {an, bn, cn};

    render_clip_tri(writer, light_vec, preclip_position, preclip_normal);
  } else {
    float ai = light_intensity(light_vec, an);
    float bi = light_intensity(light_vec, bn);
    float ci = light_intensity(light_vec, cn);

    render_tri(writer,
               screen_transform(a),
               screen_transform(b),
               screen_transform(c),
               ai,
               bi,
               ci);
  }
}

void render_model(ta_parameter_writer& writer, const mat4x4& model_trans, const mat4x4& screen, const struct model * model)
{
  vec3 light_vec = {20, 1, -20};

  mat4x4 trans = screen * model_trans;

  for (int j = 0; j < model->object_count; j++) {
    const struct object * object = model->object[j];

    for (int i = 0; i < object->quadrilateral_count; i++) {
      const union quadrilateral * quad = &object->quadrilateral[i];
      transform_quad(writer, trans, light_vec, quad, model->position, model->normal);
    }

    for (int i = 0; i < object->triangle_count; i++) {
      const union triangle * tri = &object->triangle[i];
      transform_tri(writer, trans, light_vec, tri, model->position, model->normal);
    }
  }
}

void global_modifier_volume(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque_modifier_volume
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(parameter_control_word,
					 isp_tsp_instruction_word
					 );
}

void global_modifier_volume_last_triangle(ta_parameter_writer& writer, uint32_t volume_instruction)
{
  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                             | para_control::list_type::opaque_modifier_volume
                                             | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = volume_instruction
                                               | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
                                         last_isp_tsp_instruction_word);
}

static inline void render_tri_mod(ta_parameter_writer& writer,
                                  vec3 ap,
                                  vec3 bp,
                                  vec3 cp)
{
  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         ap.x, ap.y, ap.z,
                                         bp.x, bp.y, bp.z,
                                         cp.x, cp.y, cp.z);
}

void render_inclusion_cube(ta_parameter_writer& writer)
{
  global_modifier_volume(writer);

  const struct object * object = cube_object[0];
  const vertex_position * position = cube_position;

  float scale = 1000.f;
  const mat4x4 model = {
    scale, 0, 0, 0,
    0, scale, 0, 0,
    0, 0, scale, 0,
    0, 0, 0, 1,
  };

  global_modifier_volume(writer);

  for (int i = 0; i < object->triangle_count - 1; i++) {
    const union triangle * tri = &object->triangle[i];
    vec3 a = model * position[tri->v[0].position];
    vec3 b = model * position[tri->v[1].position];
    vec3 c = model * position[tri->v[2].position];
    render_tri_mod(writer,
                   a,
                   b,
                   c);
  }

  global_modifier_volume_last_triangle(writer, isp_tsp_instruction_word::volume_instruction::inside_last_polygon);

  const union triangle * tri = &object->triangle[object->triangle_count - 1];
  vec3 a = model * position[tri->v[0].position];
  vec3 b = model * position[tri->v[1].position];
  vec3 c = model * position[tri->v[2].position];
  render_tri_mod(writer,
                 a,
                 b,
                 c);
}

void render_cone(ta_parameter_writer& writer, const mat4x4& cone_model)
{
  const struct object * object = cone_object[0];
  const vertex_position * position = cone_position;

  float _scale = 1.f;
  const mat4x4 scale = {
    _scale, 0, 0, 0,
    0, _scale, 0, 0,
    0, 0, 0.5, 0.1,
    0, 0, 0, 1,
  };
  const mat4x4 trans = cone_model * scale;

  global_modifier_volume(writer);

  for (int i = 0; i < object->triangle_count - 1; i++) {
    const union triangle * tri = &object->triangle[i];
    vec3 a = trans * position[tri->v[0].position];
    vec3 b = trans * position[tri->v[1].position];
    vec3 c = trans * position[tri->v[2].position];
    render_tri_mod(writer,
                   screen_transform(a),
                   screen_transform(b),
                   screen_transform(c));
  }

  global_modifier_volume_last_triangle(writer, isp_tsp_instruction_word::volume_instruction::outside_last_polygon);

  const union triangle * tri = &object->triangle[object->triangle_count - 1];
  vec3 a = trans * position[tri->v[0].position];
  vec3 b = trans * position[tri->v[1].position];
  vec3 c = trans * position[tri->v[2].position];
  render_tri_mod(writer,
                 screen_transform(a),
                 screen_transform(b),
                 screen_transform(c));
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen, const mat4x4& cone_model)
{
  // opaque
  {
    global_polygon_type_1(writer, texture_memory_alloc.texture.start,
                          para_control::list_type::opaque,
                          isp_tsp_instruction_word::culling_mode::no_culling);
    float scale = 1.f;
    float translate = 0.f;
    const mat4x4 model = {
      scale, 0, 0, 0,
      0, scale, 0, 0,
      0, 0, -scale, translate,
      0, 0, 0, 1,
    };
    render_model(writer, model, screen, &mansion_model);
  }
  // end of opaque list
  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  {
    render_inclusion_cube(writer);
    render_cone(writer, cone_model);
  }
  // end of modifier volume list
  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
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

  /*
  {
    uint32_t offset = texture_memory_alloc.texture.start;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(&_binary_model_dragon_dragon_data_start);
    uint32_t size = reinterpret_cast<uint32_t>(&_binary_model_dragon_dragon_data_size);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }

  {
    uint32_t offset = texture_memory_alloc.texture.start + 131072;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(&_binary_model_dragon_chrome_data_start);
    uint32_t size = reinterpret_cast<uint32_t>(&_binary_model_dragon_chrome_data_size);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
  */
}

void transfer_palette()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  /*
  uint16_t * src = reinterpret_cast<uint16_t *>(&_binary_model_dragon_dragon_data_pal_start);
  uint32_t size = reinterpret_cast<uint32_t>(&_binary_model_dragon_dragon_data_pal_size);

  for (uint32_t i = 0; i < size / 2; i++) {
    holly.PALETTE_RAM[i] = src[i];
  }
  */
}

constexpr inline mat4x4 update_cone()
{
  static float rx = 0;
  static float ry = 0;

  float tx = 0;
  float ty = 0;
  if (ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0) {
    tx =  0.5;
  } else if (ft0::data_transfer::digital_button::da(data[0].digital_button) == 0) {
    tx = -0.5;
  }

  if (ft0::data_transfer::digital_button::la(data[0].digital_button) == 0) {
    ty = -0.5;
  } else if (ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0) {
    ty =  0.5;
  }

  /*
    rx = 0, tx = 1, dx = 1;
    rx = 1, tx = 1, dx = 0;
    rx = -1, tx = 1, dx = 2;
   */

  float dx = tx - rx;
  float dy = ty - ry;

  rx += dx * 0.05;
  ry += dy * 0.05;

  mat4x4 mrx = {
    1, 0, 0, 0,
    0, cos(rx), -sin(rx), 0,
    0, sin(rx),  cos(rx), 0,
    0, 0, 0, 1,
  };

  mat4x4 mry = {
     cos(ry), 0, sin(ry), 0,
    0, 1, 0, 0,
    -sin(ry), 0, cos(ry), 0,
    0, 0, 0, 1,
  };

  return mry * mrx;
}

constexpr inline mat4x4 update_analog(mat4x4& screen)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;
  float x = 0.05f * -x_;
  float y = 0.05f * y_;

  mat4x4 t = {
    1, 0, 0, x,
    0, 1, 0, 0,
    0, 0, 1, y,
    0, 0, 0, 1,
  };

  float theta = 0;
  if (l_ > 0.1f) {
    theta = -0.05f * l_;
  } else if (r_ > 0.1f) {
    theta = 0.05f * r_;
  }

  mat4x4 ry = {
     cos(theta), 0, sin(theta), 0,
    0, 1, 0, 0,
    -sin(theta), 0, cos(theta), 0,
    0, 0, 0, 1,
  };

  return t * ry * screen;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);

  serial::integer<uint32_t>((sizeof (female_position)) / (sizeof (female_position[0])));

  interrupt_init();

  constexpr uint32_t ta_alloc = 0
                              | ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 16 * 4,
      .opaque_modifier = 16 * 4,
      .translucent = 0,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::intensity_volume_mode
                       | fpu_shad_scale::scale_factor_for_shadows(128);

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
  transfer_palette();
  video_output::set_mode_vga();

  do_get_condition();

  mat4x4 screen = screen_rotation(0, 0, 0);

  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    screen = update_analog(screen);
    mat4x4 cone_model = update_cone();

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    writer.offset = 0;
    transfer_scene(writer, screen, cone_model);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_modifier_volume_list();

    render_done = 0;
    core_start_render2(texture_memory_alloc.region_array[core].start,
                       texture_memory_alloc.isp_tsp_parameters[core].start,
                       texture_memory_alloc.background[core].start,
                       texture_memory_alloc.framebuffer[core].start,
                       framebuffer_width);
    while (render_done == 0) {
      asm volatile ("nop");
    };

    while (spg_status::vsync(holly.SPG_STATUS));
    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;
  }
}
