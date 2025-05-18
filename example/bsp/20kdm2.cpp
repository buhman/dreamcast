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
#include "holly/texture_memory_alloc5.hpp"
#include "holly/video_output.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "maple/maple.hpp"
#include "maple/maple_host_command_writer.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"

#include "memorymap.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat2x2.hpp"
#include "math/mat3x3.hpp"
#include "math/mat4x4.hpp"
#include "math/geometry.hpp"

#include "interrupt.hpp"

#include "bsp/20kdm2/textures/e7/e7walldesign01b.data.h"
#include "bsp/20kdm2/textures/e7/e7steptop2.data.h"
#include "bsp/20kdm2/textures/e7/e7dimfloor.data.h"
#include "bsp/20kdm2/textures/e7/e7brickfloor01.data.h"
#include "bsp/20kdm2/textures/e7/e7bmtrim.data.h"
#include "bsp/20kdm2/textures/e7/e7sbrickfloor.data.h"
#include "bsp/20kdm2/textures/e7/e7brnmetal.data.h"
#include "bsp/20kdm2/textures/e7/e7beam02_red.data.h"
#include "bsp/20kdm2/textures/e7/e7swindow.data.h"
#include "bsp/20kdm2/textures/e7/e7bigwall.data.h"
#include "bsp/20kdm2/textures/e7/e7panelwood.data.h"
#include "bsp/20kdm2/textures/e7/e7beam01.data.h"
#include "bsp/20kdm2/textures/gothic_floor/xstepborder5.data.h"
#include "bsp/20kdm2/textures/liquids/lavahell.data.h"
#include "bsp/20kdm2/textures/e7/e7steptop.data.h"
#include "bsp/20kdm2/textures/gothic_trim/metalblackwave01.data.h"
#include "bsp/20kdm2/textures/stone/pjrock1.data.h"
#include "bsp/20kdm2/models/mapobjects/timlamp/timlamp.data.h"
#include "bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2.data.h"
#include "bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2b.data.h"

#include "bsp/20kdm2/textures/sfx/flame1.data.h"
#include "bsp/20kdm2/textures/sfx/flame2.data.h"
#include "bsp/20kdm2/textures/sfx/flame3.data.h"
#include "bsp/20kdm2/textures/sfx/flame4.data.h"
#include "bsp/20kdm2/textures/sfx/flame5.data.h"
#include "bsp/20kdm2/textures/sfx/flame6.data.h"
#include "bsp/20kdm2/textures/sfx/flame7.data.h"
#include "bsp/20kdm2/textures/sfx/flame8.data.h"

#include "q3bsp/q3bsp.h"
#include "q3bsp/q3bsp_patch.hpp"
#include "bsp/20kdm2/maps/20kdm2.bsp.h"
#include "bsp/20kdm2/texture.h"

#include "mdxm/mdxm.h"
#include "model/tavion_new/model.glm.h"
#include "model/tavion_new/legs.vq.h"
#include "model/tavion_new/torso.vq.h"
#include "model/tavion_new/head.vq.h"
#include "model/tavion_new/face.vq.h"
#include "model/tavion_new/arm.vq.h"
#include "model/tavion_new/hands.vq.h"
#include "model/tavion_new/surface.h"
#include "model/tavion_new/texture.h"

#include "model/model.h"
#include "model/icosphere/model.h"

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"
#include "palette.hpp"
#include "printf/unparse.h"

#include "assert.h"

#undef nullptr
#undef static_assert

constexpr int font_base = ((0x7f - 0x20) + 1) * 8 * 16 / 2;

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

static vec3 sphere_position = {890, 550, 450};

static ft0::data_transfer::data_format data[4];

uint8_t send_buf[1024] __attribute__((aligned(32)));
uint8_t recv_buf[1024] __attribute__((aligned(32)));

constexpr void * bsp_start = &_binary_bsp_20kdm2_maps_20kdm2_bsp_start;

uint32_t lightmap_base = 0;
uint32_t bsp_base = 0;
uint32_t tavion_base = 0;

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

struct position_normal {
  vec3 position;
  vec3 normal;
};

static uint8_t face_cache[16384];

static position_normal vertex_cache[16384];

static inline vec3 normal_transform(const mat4x4& trans, vec3 normal)
{
  vec4 n = trans * (vec4){normal.x, normal.y, normal.z, 0.f}; // no translation component
  return {n.x, n.y, n.z};
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

void global_polygon_type_0(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
                                        | obj_control::col_type::packed_color
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::src_alpha
                                      | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                      | tsp_instruction_word::use_alpha
                                      ;

  const uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control,
                           uint32_t texture_u_v_size,
                           uint32_t texture_control_word,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f,
                           bool depth_always = false
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = ((depth_always) ? isp_tsp_instruction_word::depth_compare_mode::always : isp_tsp_instruction_word::depth_compare_mode::greater)
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::filter_mode::bilinear_filter
                                      | tsp_instruction_word::texture_shading_instruction::decal
                                      | texture_u_v_size
                                      ;

  writer.append<ta_global_parameter::polygon_type_1>() =
    ta_global_parameter::polygon_type_1(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        a,
                                        r,
                                        g,
                                        b
                                        );
}

void global_polygon_type_4(ta_parameter_writer& writer,
                           uint32_t obj_control_texture,
                           uint32_t tsp_instruction_word_0,
                           uint32_t texture_control_word_0,
                           uint32_t tsp_instruction_word_1,
                           uint32_t texture_control_word_1,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::punch_through
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control::shadow
                                        | obj_control::volume::polygon::with_two_volumes
                                        | obj_control_texture
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
    | isp_tsp_instruction_word::culling_mode::no_culling // cull_if_negative
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::filter_mode::bilinear_filter
                                      | tsp_instruction_word::texture_shading_instruction::modulate
                                      ;

  writer.append<ta_global_parameter::polygon_type_4>() =
    ta_global_parameter::polygon_type_4(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word_0 | tsp_instruction_word,
                                        texture_control_word_0,
                                        tsp_instruction_word_1 | tsp_instruction_word,
                                        texture_control_word_1,
                                        0, // data_size_for_sort_dma
                                        0, // next_address_for_sort_dma
                                        a, // face_color_alpha_0
                                        r, // face_color_r_0
                                        g, // face_color_g_0
                                        b, // face_color_b_0
                                        a, // face_color_alpha_1
                                        r, // face_color_r_1
                                        g, // face_color_g_1
                                        b  // face_color_b_1
                                        );
}

void global_texture(ta_parameter_writer& writer, int texture_ix)
{
  struct pk_texture * texture = &textures[texture_ix];

  uint32_t texture_u_v_size = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::one
                            | tsp_instruction_word::texture_u_size::from_int(texture->width)
                            | tsp_instruction_word::texture_v_size::from_int(texture->height)
                            ;

  uint32_t texture_address = texture_memory_alloc.texture.start + font_base + lightmap_base + texture->offset;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  uint32_t control = para_control::list_type::translucent
                   | obj_control::texture;
  global_polygon_type_1(writer,
                        control,
                        texture_u_v_size,
                        texture_control_word);
}

void global_tavion_texture(ta_parameter_writer& writer, int texture_ix)
{
  const struct pk_texture * texture = &tavion_textures[texture_ix];

  uint32_t texture_u_v_size = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::zero
                            | tsp_instruction_word::texture_u_size::from_int(texture->width)
                            | tsp_instruction_word::texture_v_size::from_int(texture->height)
                            ;

  uint32_t texture_address = texture_memory_alloc.texture.start + font_base + lightmap_base + bsp_base + texture->offset;
  uint32_t texture_control_word = texture_control_word::vq_compressed
                                | texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  uint32_t control = para_control::list_type::opaque
                   | obj_control::texture;
  global_polygon_type_1(writer,
                        control,
                        texture_u_v_size,
                        texture_control_word);
}

/*
void global_lightmap(ta_parameter_writer& writer, int lightmap_ix)
{
  uint32_t texture_u_v_size = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::zero
                            | tsp_instruction_word::texture_u_size::from_int(128)
                            | tsp_instruction_word::texture_v_size::from_int(128)
                            ;

  uint32_t texture_address = texture_memory_alloc.texture.start + font_base + 128 * 128 * 2 * lightmap_ix;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::non_twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  global_polygon_type_1(writer,
                        obj_control::texture,
                        texture_u_v_size,
                        texture_control_word);
}
*/

void global_texture_lightmap(ta_parameter_writer& writer, int texture_ix, int lightmap_ix)
{
  pk_texture * texture = &textures[texture_ix];
  int texture_offset = texture->offset;
  int lightmap_offset = 128 * 128 * 2 * lightmap_ix;

  uint32_t tsp_instruction_word_0 = tsp_instruction_word::src_alpha_instr::one
                                  | tsp_instruction_word::dst_alpha_instr::zero
                                  | tsp_instruction_word::texture_u_size::from_int(texture->width)
                                  | tsp_instruction_word::texture_v_size::from_int(texture->height)
                                  ;

  uint32_t texture_address_0 = texture_memory_alloc.texture.start + font_base + lightmap_base + texture_offset;
  uint32_t texture_control_word_0 = texture_control_word::pixel_format::_1555
                                  | texture_control_word::scan_order::twiddled
                                  | texture_control_word::texture_address(texture_address_0 / 8)
                                  ;

  uint32_t tsp_instruction_word_1;
  if (lightmap_offset >= 0) {
    tsp_instruction_word_1 = tsp_instruction_word::src_alpha_instr::other_color
                           | tsp_instruction_word::dst_alpha_instr::zero
                           | tsp_instruction_word::texture_u_size::from_int(128)
                           | tsp_instruction_word::texture_v_size::from_int(128)
                           ;
  } else {
    tsp_instruction_word_1 = tsp_instruction_word::src_alpha_instr::zero
                           | tsp_instruction_word::dst_alpha_instr::one
                           | tsp_instruction_word::texture_u_size::from_int(128)
                           | tsp_instruction_word::texture_v_size::from_int(128)
                           ;
  }

  uint32_t texture_address_1 = texture_memory_alloc.texture.start + font_base + lightmap_offset;
  uint32_t texture_control_word_1 = texture_control_word::pixel_format::_565
                                  | texture_control_word::scan_order::non_twiddled
                                  | texture_control_word::texture_address(texture_address_1 / 8)
                                  ;

  global_polygon_type_4(writer,
                        obj_control::texture,
                        tsp_instruction_word_0,
                        texture_control_word_0,
                        tsp_instruction_word_1,
                        texture_control_word_1);
}

void transform_vertices(uint8_t * buf, int length, const mat4x4& trans)
{
  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(buf);

  int count = length / (sizeof (struct q3bsp_vertex));

  for (int i = 0; i < count; i++) {
    vec3 v = {vert[i].position[0], vert[i].position[1], vert[i].position[2]};
    vec3 n = {vert[i].normal[0], vert[i].normal[1], vert[i].normal[2]};

    //printf("%f %f %f\n", v.x, v.y, v.z);

    vertex_cache[i].position = trans * v;
    vertex_cache[i].normal = normal_transform(trans, n);
  }
}

static inline void render_tri_type_2(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     float li)
{
  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        li);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        li);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        li);
}

static int typen_tri_count = 0;
static int vis_tri_count = 0;
static int total_tri_count = 0;

static inline void render_tri_type_7(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     vec2 at,
                                     vec2 bt,
                                     vec2 ct,
                                     float li)
{
  typen_tri_count += 1;

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        li,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        li,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        li,
                                        0);
}

static inline void render_tri_type_13(ta_parameter_writer& writer,
                                      vec3 ap,
                                      vec3 bp,
                                      vec3 cp,
                                      vec2 at0,
                                      vec2 bt0,
                                      vec2 ct0,
                                      vec2 at1,
                                      vec2 bt1,
                                      vec2 ct1,
                                      float li0,
                                      float li1)
{
  typen_tri_count += 1;

  writer.append<ta_vertex_parameter::polygon_type_13>() =
    ta_vertex_parameter::polygon_type_13(polygon_vertex_parameter_control_word(false),
                                         ap.x, ap.y, ap.z,
                                         at0.x, at0.y,
                                         li0,
                                         0, // offset intensity 0
                                         at1.x, at1.y,
                                         li1,
                                         0  // offset intensity 1
                                         );

  writer.append<ta_vertex_parameter::polygon_type_13>() =
    ta_vertex_parameter::polygon_type_13(polygon_vertex_parameter_control_word(false),
                                         bp.x, bp.y, bp.z,
                                         bt0.x, bt0.y,
                                         li0,
                                         0, // offset intensity 0
                                         bt1.x, bt1.y,
                                         li1,
                                         0 // offset intensity 1
                                         );

  writer.append<ta_vertex_parameter::polygon_type_13>() =
    ta_vertex_parameter::polygon_type_13(polygon_vertex_parameter_control_word(true),
                                         cp.x, cp.y, cp.z,
                                         ct0.x, ct0.y,
                                         li0,
                                         0, // offset intensity 0
                                         ct1.x, ct1.y,
                                         li1,
                                         0  // offset intensity 1
                                         );
}

static inline void render_clip_tri_type_13(ta_parameter_writer& writer,
                                           vec3 ap,
                                           vec3 bp,
                                           vec3 cp,
                                           vec2 at0,
                                           vec2 bt0,
                                           vec2 ct0,
                                           vec2 at1,
                                           vec2 bt1,
                                           vec2 ct1,
                                           float li0,
                                           float li1)
{
  //return;
  const vec3 plane_point = {0.f, 0.f, 1.f};
  const vec3 plane_normal = {0.f, 0.f, 1.f};

  vec3 preclip_position[] = {ap, bp, cp};
  vec2 preclip_texture0[] = {at0, bt0, ct0};
  vec2 preclip_texture1[] = {at1, bt1, ct1};

  vec3 clip_position[4];
  vec2 clip_texture0[4];
  vec2 clip_texture1[4];
  int output_length = geometry::clip_polygon_3<3>(clip_position,
                                                  clip_texture0,
                                                  clip_texture1,
                                                  plane_point,
                                                  plane_normal,
                                                  preclip_position,
                                                  preclip_texture0,
                                                  preclip_texture1);

  {
    vec3 ap;
    vec3 bp;
    vec3 cp;
    vec3 dp;

    const vec2& at0 = clip_texture0[0];
    const vec2& bt0 = clip_texture0[1];
    const vec2& ct0 = clip_texture0[2];
    const vec2& dt0 = clip_texture0[3];

    const vec2& at1 = clip_texture1[0];
    const vec2& bt1 = clip_texture1[1];
    const vec2& ct1 = clip_texture1[2];
    const vec2& dt1 = clip_texture1[3];

    if (output_length >= 3) {
      ap = screen_transform(clip_position[0]);
      bp = screen_transform(clip_position[1]);
      cp = screen_transform(clip_position[2]);

      render_tri_type_13(writer,
                         ap,
                         bp,
                         cp,
                         at0,
                         bt0,
                         ct0,
                         at1,
                         bt1,
                         ct1,
                         li0,
                         li1);
    }
    if (output_length >= 4) {
      dp = screen_transform(clip_position[3]);

      render_tri_type_13(writer,
                         ap,
                         cp,
                         dp,
                         at0,
                         ct0,
                         dt0,
                         at1,
                         ct1,
                         dt1,
                         li0,
                         li1);
    }
  }
}

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
    intensity += 0.9f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
    if (intensity > 1.0f)
      intensity = 1.0f;
  }
  return intensity;
}

static vec3 light_vec = {20, -20, -20};

static inline void transfer_face_meshverts(ta_parameter_writer& writer, q3bsp_face_t * face)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);

  q3bsp_direntry * me = &header->direntries[LUMP_MESHVERTS];
  q3bsp_meshvert_t * meshvert = reinterpret_cast<q3bsp_meshvert_t *>(&buf[me->offset]);

  int meshvert_ix = face->meshvert;
  q3bsp_meshvert_t * mv = &meshvert[meshvert_ix];

  int triangles = face->n_meshverts / 3;

  for (int j = 0; j < triangles; j++) {

    int aix = mv[j * 3 + 0].offset + face->vertex;
    int bix = mv[j * 3 + 1].offset + face->vertex;
    int cix = mv[j * 3 + 2].offset + face->vertex;

    vec3 ap = vertex_cache[aix].position;
    vec3 bp = vertex_cache[bix].position;
    vec3 cp = vertex_cache[cix].position;

    vis_tri_count += 1;

    if (ap.z < 0 && bp.z < 0 && cp.z < 0) {
      continue;
    }

    vec3 n = vertex_cache[aix].normal;
    float li0 = light_intensity(light_vec, n);
    const float li1 = 2.0;

    float v_mul = textures[face->texture].v_mul;
    vec2 at = {vert[aix].texture[0], vert[aix].texture[1] * v_mul};
    vec2 bt = {vert[bix].texture[0], vert[bix].texture[1] * v_mul};
    vec2 ct = {vert[cix].texture[0], vert[cix].texture[1] * v_mul};

    vec2 alm = {vert[aix].lightmap[0], vert[aix].lightmap[1]};
    vec2 blm = {vert[bix].lightmap[0], vert[bix].lightmap[1]};
    vec2 clm = {vert[cix].lightmap[0], vert[cix].lightmap[1]};

    if (ap.z < 0 || bp.z < 0 || cp.z < 0) {
      render_clip_tri_type_13(writer,
                              ap,
                              bp,
                              cp,
                              at,
                              bt,
                              ct,
                              alm,
                              blm,
                              clm,
                              li0,
                              li1);
    } else {
      vec3 aps = screen_transform(ap);
      vec3 bps = screen_transform(bp);
      vec3 cps = screen_transform(cp);

      render_tri_type_13(writer,
                         aps,
                         bps,
                         cps,
                         at,
                         bt,
                         ct,
                         alm,
                         blm,
                         clm,
                         li0,
                         li1);
    }
  }
}

static inline void transfer_face_patch_surfaces(ta_parameter_writer& writer, const mat4x4& trans, q3bsp_face_t * face, int face_ix)
{
  using namespace q3bsp_patch;

  patch * patch = NULL;
  for (int i = 0; i < patch_count; i++) {
    if (patches[i].face_ix == face_ix) {
      patch = &patches[i];
      break;
    }
  }
  assert(patch != nullptr);

  const int width = face->size[0];
  const int height = face->size[1];
  const int h_surfaces = (width - 1) / 2;
  const int v_surfaces = (height - 1) / 2;
  const int surface_count = h_surfaces * v_surfaces;
  assert(surface_count > 0);

  const vertex_plm * vertices = &patch_vertices[patch->vertex_ix];
  const bezier::triangle * triangles = &patch_triangles[patch->triangle_ix];

  const int triangle_count = surface_count * triangles_per_surface;
  for (int i = 0; i < triangle_count; i++) {
    vis_tri_count += 1;

    const bezier::triangle * triangle = &triangles[i];
    assert(triangle->a >= 0 && triangle->b >= 0 && triangle->c >= 0);
    if (triangle->a == triangle->b && triangle->b == triangle->c) {
      printf("face_ix %d %d\n", face_ix, i);
      printf("        %d %d %d\n", triangle->a, triangle->b, triangle->c);
      assert(false);
    }

    const vertex_plm * av = &vertices[triangle->a];
    const vertex_plm * bv = &vertices[triangle->b];
    const vertex_plm * cv = &vertices[triangle->c];

    vec3 ap = trans * av->l;
    vec3 bp = trans * bv->l;
    vec3 cp = trans * cv->l;

    if (ap.z < 0 || bp.z < 0 || cp.z < 0) {
      continue;
      //printf("cont %f %f %f\n", ap.x, ap.y, ap.z);
    }
    /*
    printf("%f %f %f\n", ap.x, ap.y, ap.z);
    printf("%f %f %f\n", bp.x, bp.y, bp.z);
    printf("%f %f %f\n", cp.x, cp.y, cp.z);
    */

    vec3 aps = screen_transform(ap);
    vec3 bps = screen_transform(bp);
    vec3 cps = screen_transform(cp);

    const vec2& at = av->m;
    const vec2& bt = bv->m;
    const vec2& ct = cv->m;

    const vec2& alm = av->n;
    const vec2& blm = bv->n;
    const vec2& clm = cv->n;

    const vec3 n = normal_transform(trans, av->o);
    float li0 = light_intensity(light_vec, n);
    float li1 = 2.0;

    render_tri_type_13(writer,
                       aps,
                       bps,
                       cps,
                       at,
                       bt,
                       ct,
                       alm,
                       blm,
                       clm,
                       li0,
                       li1);
  }
}

void transfer_faces(ta_parameter_writer& writer, const mat4x4& trans)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  int last_texture = -1;
  int last_lm_index = -1;

  const int textures_length = (sizeof (textures)) / (sizeof (textures[0]));

  for (int i = 0; i < face_count; i++) {
    q3bsp_face_t * face = &faces[i];

    bool has_texture =
      (face->texture >= 0) &&
      (face->texture < textures_length) &&
      (textures[face->texture].size != 0);

    if (!has_texture)
      continue;

    if (face->texture != last_texture || face->lm_index != last_lm_index) {
      last_texture = face->texture;
      last_lm_index = face->lm_index;

      global_texture_lightmap(writer, face->texture, face->lm_index);
    }

    if (face->type == FACE_TYPE_POLYGON || face->type == FACE_TYPE_MESH)
      transfer_face_meshverts(writer, face);
    if (face->type == FACE_TYPE_PATCH)
      transfer_face_patch_surfaces(writer, trans, face, i);
  }
}

int count_face_triangles()
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  int sum = 0;

  for (int i = 0; i < face_count; i++) {
    int triangles = faces[i].n_meshverts / 3;
    sum += triangles;
  }

  return sum;
}

const int flame1_ix = 27;

const vec3 billboard_p[] = {
  (vec3){-1, -2, 0} * 10.f,
  (vec3){ 1, -2, 0} * 10.f,
  (vec3){ 1,  2, 0} * 10.f,
  (vec3){-1,  2, 0} * 10.f,
};

const vec2 billboard_t[] = {
  {0, 0},
  {1, 0},
  {1, 1},
  {0, 1},
};

int anim_count = 0;
int flame_ix = 0;

static inline void transfer_face_billboard(ta_parameter_writer& writer, q3bsp_face_t * face)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);

  q3bsp_direntry * me = &header->direntries[LUMP_MESHVERTS];
  q3bsp_meshvert_t * meshvert = reinterpret_cast<q3bsp_meshvert_t *>(&buf[me->offset]);

  int meshvert_ix = face->meshvert;
  q3bsp_meshvert_t * mv = &meshvert[meshvert_ix];

  int triangles = face->n_meshverts / 3;

  assert(face->texture == 23 || face->texture == 24);

  float li = 1;

  for (int j = 0; j < triangles; j++) {

    int aix = mv[j * 3 + 0].offset + face->vertex;
    int bix = mv[j * 3 + 1].offset + face->vertex;
    int cix = mv[j * 3 + 2].offset + face->vertex;

    vec3 ap = vertex_cache[aix].position;
    vec3 bp = vertex_cache[bix].position;
    vec3 cp = vertex_cache[cix].position;

    vis_tri_count += 1;

    if (ap.z < 0 || bp.z < 0 || cp.z < 0) {
      continue;
    }

    vec2 at = {vert[aix].texture[0], vert[aix].texture[1]};
    vec2 bt = {vert[bix].texture[0], vert[bix].texture[1]};
    vec2 ct = {vert[cix].texture[0], vert[cix].texture[1]};

    render_tri_type_7(writer,
                      screen_transform(ap),
                      screen_transform(bp),
                      screen_transform(cp),
                      at,
                      bt,
                      ct,
                      li);
  }
}

void transfer_billboard(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  global_texture(writer, flame1_ix + flame_ix);

  /*
  const vec2& at = billboard_t[0];
  const vec2& bt = billboard_t[1];
  const vec2& ct = billboard_t[2];
  const vec2& dt = billboard_t[3];
  */

  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  for (int i = 0; i < face_count; i++) {
    q3bsp_face_t * face = &faces[i];
    if (!(face->texture == 23 || face->texture == 24))
      continue;

    if (!face_cache[i])
      continue;

    transfer_face_billboard(writer, face);
  }

  if (anim_count++ > 3) {
    flame_ix += 1;
    if (flame_ix >= 8)
      flame_ix = 0;
    anim_count = 0;
  }
}

void transfer_icosphere(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  const struct model * model = &icosphere_model;
  const struct object * object = model->object[0];
  const vertex_position * position = model->position;
  const vertex_normal * normal = model->normal;

  float s = 50;
  mat4x4 scale = {
    s, 0, 0, 0,
    0, -s, 0, 0,
    0, 0, s, 0,
    0, 0, 0, 1
  };

  mat4x4 translate = {
    1, 0, 0, sphere_position.x,
    0, 1, 0, sphere_position.y,
    0, 0, 1, sphere_position.z,
    0, 0, 0, 1
  };

  mat4x4 trans = screen_trans * translate * scale;

  float a = 1.0f;
  float r = 0.9f;
  float g = 0.5f;
  float b = 0.0f;
  uint32_t control = para_control::list_type::opaque;
  uint32_t texture_u_v_size = tsp_instruction_word::src_alpha_instr::one
                            | tsp_instruction_word::dst_alpha_instr::zero;
  uint32_t texture_control_word = 0;
  global_polygon_type_1(writer,
                        control,
                        texture_u_v_size,
                        texture_control_word,
                        a,
                        r,
                        g,
                        b,
                        true);

  for (int i = 0; i < object->triangle_count; i++) {
    const union triangle * tri = &object->triangle[i];
    vec3 ap = trans * position[tri->v[0].position];
    vec3 bp = trans * position[tri->v[1].position];
    vec3 cp = trans * position[tri->v[2].position];

    if (ap.z < 0 || bp.z < 0 || cp.z < 0) return;

    vec3 n = normal_transform(trans, normal[tri->v[0].normal]);
    float li = light_intensity(light_vec, n);

    render_tri_type_2(writer,
                      screen_transform(ap),
                      screen_transform(bp),
                      screen_transform(cp),
                      li);
  }
}

struct mdxm_trans {
  vec3 position;
  vec3 normal;
};

static inline void transfer_mdxm_surface(ta_parameter_writer& writer, const mat4x4& trans, const mdxm_surface_t * surface)
{
  mdxm_vertex_t * v = (mdxm_vertex_t *) (((uint8_t *)surface) + surface->offset_verts);

  mdxm_trans transformed[surface->num_verts];
  for (int i = 0; i < surface->num_verts; i++) {
    vec3 position = {v[i].position[0], v[i].position[1], v[i].position[2]};
    vec3 normal   = {v[i].normal[0],   v[i].normal[1],   v[i].normal[2]  };

    transformed[i].position = trans * position;
    transformed[i].normal = normal_transform(trans, normal);
  }

  mdxm_triangle_t * triangles = (mdxm_triangle_t *)(((uint8_t *)surface) + surface->offset_triangles);
  mdxm_vertex_texture_coord_t * texture = (mdxm_vertex_texture_coord_t *)&v[surface->num_verts];

  for (int i = 0; i < surface->num_triangles; i++) {
    const vec3& ap = transformed[triangles[i].index[0]].position;
    const vec3& bp = transformed[triangles[i].index[1]].position;
    const vec3& cp = transformed[triangles[i].index[2]].position;
    //if (ap.z < 0 || bp.z < 0 || cp.z < 0) continue;

    const vec3& n = transformed[triangles[i].index[0]].normal;
    float li = light_intensity(light_vec, n);
    vec2 at = {texture[triangles[i].index[0]].texture[0], texture[triangles[i].index[0]].texture[1]};
    vec2 bt = {texture[triangles[i].index[1]].texture[0], texture[triangles[i].index[1]].texture[1]};
    vec2 ct = {texture[triangles[i].index[2]].texture[0], texture[triangles[i].index[2]].texture[1]};

    render_tri_type_7(writer,
                      screen_transform(ap),
                      screen_transform(bp),
                      screen_transform(cp),
                      at,
                      bt,
                      ct,
                      li);
  }
}

void transfer_tavion(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  float s = 1;
  mat4x4 scale = {
    s, 0, 0, 0,
    0, -s, 0, 0,
    0, 0, s, 0,
    0, 0, 0, 1
  };

  mat4x4 translate = {
    1, 0, 0, sphere_position.x,
    0, 1, 0, sphere_position.y,
    0, 0, 1, sphere_position.z,
    0, 0, 0, 1
  };

  mat4x4 trans = screen_trans * translate * scale;

  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_model_tavion_new_model_glm_start);
  mdxm_header_t * header = (mdxm_header_t *)(buf);
  mdxm_lod_t * lod = (mdxm_lod_t *)&buf[header->offset_lods];
  const int surface_offset = (sizeof (mdxm_lod_t)) + (header->num_surfaces * (sizeof (mdxm_lod_surf_offset_t)));
  mdxm_surface_t * surface = (mdxm_surface_t *)(((uint8_t *)lod) + surface_offset);
  //int count = 0;

  int last_texture_ix = -1;

  for (int i = 0; i < header->num_surfaces; i++) {
    //printf("surf %d\n", i);
    if (i > 36)
      break;
    if (tavion_surface[i] >= 0) {
      int texture_ix = tavion_surface[i];
      if (tavion_surface[i] != last_texture_ix)
        global_tavion_texture(writer, texture_ix);
      last_texture_ix = texture_ix;
      //printf("mdxm %d\n", i);
      transfer_mdxm_surface(writer, trans, surface);
    }

    // next surface
    surface = (mdxm_surface_t *)(((uint8_t *)surface) + surface->offset_end);
  }
  //printf("count: %d\n", count);
}

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               uint32_t base_color)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

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

void render_bounding_box(ta_parameter_writer& writer, const mat4x4& trans, vec3 max, vec3 min, uint32_t color)
{
  vec3 a = max;
  vec3 b = min;

  global_polygon_type_0(writer);

  // ax
  render_quad(writer,
              screen_transform(trans * (vec3){a.x, a.y, a.z}),
              screen_transform(trans * (vec3){a.x, b.y, a.z}),
              screen_transform(trans * (vec3){a.x, b.y, b.z}),
              screen_transform(trans * (vec3){a.x, a.y, b.z}),
              color);

  // bx
  render_quad(writer,
              screen_transform(trans * (vec3){b.x, a.y, a.z}),
              screen_transform(trans * (vec3){b.x, b.y, a.z}),
              screen_transform(trans * (vec3){b.x, b.y, b.z}),
              screen_transform(trans * (vec3){b.x, a.y, b.z}),
              color);

  // ay
  render_quad(writer,
              screen_transform(trans * (vec3){b.x, a.y, a.z}),
              screen_transform(trans * (vec3){a.x, a.y, a.z}),
              screen_transform(trans * (vec3){a.x, a.y, b.z}),
              screen_transform(trans * (vec3){b.x, a.y, b.z}),
              color);

  // by
  render_quad(writer,
              screen_transform(trans * (vec3){b.x, b.y, a.z}),
              screen_transform(trans * (vec3){a.x, b.y, a.z}),
              screen_transform(trans * (vec3){a.x, b.y, b.z}),
              screen_transform(trans * (vec3){b.x, b.y, b.z}),
              color);

  // az
  render_quad(writer,
              screen_transform(trans * (vec3){b.x, a.y, a.z}),
              screen_transform(trans * (vec3){b.x, b.y, a.z}),
              screen_transform(trans * (vec3){a.x, b.y, a.z}),
              screen_transform(trans * (vec3){a.x, a.y, a.z}),
              color);

  // bz
  render_quad(writer,
              screen_transform(trans * (vec3){b.x, a.y, b.z}),
              screen_transform(trans * (vec3){b.x, b.y, b.z}),
              screen_transform(trans * (vec3){a.x, b.y, b.z}),
              screen_transform(trans * (vec3){a.x, a.y, b.z}),
              color);
}

int format_float(char * s, float num, int pad_length)
{
  int offset = 0;
  bool negative = num < 0;
  if (negative) num = -num;
  int32_t whole = num;
  int digits = digits_base10(whole);
  offset += unparse_base10_unsigned(&s[offset], whole, pad_length, ' ');
  if (negative)
    s[offset - (digits + 1)] = '-';
  s[offset++] = '.';
  int32_t fraction = (int32_t)((num - (float)whole) * 1000.0);
  if (fraction < 0)
    fraction = -fraction;
  offset += unparse_base10_unsigned(&s[offset], fraction, 3, '0');
  return offset;
}

void render_matrix(ta_parameter_writer& writer, const mat4x4& trans)
{
  for (int row = 0; row < 4; row++) {
    char __attribute__((aligned(4))) s[64];
    for (uint32_t i = 0; i < (sizeof (s)) / 4; i++)
      reinterpret_cast<uint32_t *>(s)[i] = 0x20202020;

    int offset = 0;
    offset += format_float(&s[offset], trans[row][0], 7);
    offset += format_float(&s[offset], trans[row][1], 7);
    offset += format_float(&s[offset], trans[row][2], 7);
    offset += format_float(&s[offset], trans[row][3], 7);

    font_bitmap::transform_string(writer,
                                  texture_memory_alloc.texture.start,
                                  8,  16, // texture
                                  8,  16, // glyph
                                  16 + 2 * 8, // position x
                                  16 + row * 16, // position y
                                  s, offset,
                                  para_control::list_type::opaque);
  }
}

void render_sphere_position(ta_parameter_writer& writer)
{
  char __attribute__((aligned(4))) s[64] = "pos:    ";
  for (uint32_t i = 2; i < ((sizeof (s)) - 8) / 4; i++)
    reinterpret_cast<uint32_t *>(s)[i] = 0x20202020;

  int offset = 8;
  int row = 5;
  offset += format_float(&s[offset], sphere_position[0], 7);
  offset += format_float(&s[offset], sphere_position[1], 7);
  offset += format_float(&s[offset], sphere_position[2], 7);

  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 2 * 8, // position x
                                16 + row * 16, // position y
                                s, offset,
                                para_control::list_type::opaque);
}

void render_zero_position(ta_parameter_writer& writer, const mat4x4& screen_trans_inv)
{
  char __attribute__((aligned(4))) s[64] = "zero:   ";
  for (uint32_t i = 2; i < ((sizeof (s)) - 8) / 4; i++)
    reinterpret_cast<uint32_t *>(s)[i] = 0x20202020;

  vec3 zero = {0, 0, 0};
  vec3 pos = screen_trans_inv * zero;

  int offset = 8;
  int row = 6;
  offset += format_float(&s[offset], pos[0], 7);
  offset += format_float(&s[offset], pos[1], 7);
  offset += format_float(&s[offset], pos[2], 7);

  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 2 * 8, // position x
                                16 + row * 16, // position y
                                s, offset,
                                para_control::list_type::opaque);
}

static int root_ix = 0;

void render_ix(ta_parameter_writer& writer, int row, char * s, int ix)
{
  int offset = 15;
  bool is_leaf = ix < 0;
  if (ix < 0)
    ix = -(ix - 1);

  offset += unparse_base10_unsigned(&s[offset], ix, 5, ' ');

  if (is_leaf) {
    s[offset++] = ' ';
    s[offset++] = '(';
    s[offset++] = 'l';
    s[offset++] = 'e';
    s[offset++] = 'a';
    s[offset++] = 'f';
    s[offset++] = ')';
  } else {
    s[offset++] = ' ';
    s[offset++] = '(';
    s[offset++] = 'n';
    s[offset++] = 'o';
    s[offset++] = 'd';
    s[offset++] = 'e';
    s[offset++] = ')';
  }

  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 50 * 8, // position x
                                16 + row * 16, // position y
                                s, offset,
                                para_control::list_type::opaque);
}

void render_leaf_ix(ta_parameter_writer& writer)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);
  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);
  q3bsp_node_t * root = &nodes[root_ix];

  {
    char s[32] = "root:          ";
    int row = 0;
    render_ix(writer, row, s, root_ix);
  }

  {
    char s[32] = "root.child[0]: ";
    int row = 1;
    render_ix(writer, row, s, root->children[0]);
  }

  {
    char s[32] = "root.child[1]: ";
    int row = 2;
    render_ix(writer, row, s, root->children[1]);
  }
}

void render_num(ta_parameter_writer& writer, int row, char * s, int num, int offset)
{
  offset += unparse_base10_unsigned(&s[offset], num, 5, ' ');

  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 50 * 8, // position x
                                16 + row * 16, // position y
                                s, offset,
                                para_control::list_type::opaque);
}

void render_tris_count(ta_parameter_writer& writer)
{
  int offset = 18;
  {
    char s[32] = "total tris:       ";
    int row = 0;
    render_num(writer, row, s, total_tri_count, offset);
  }
  {
    char s[32] = "bsp-visible tris: ";
    int row = 1;
    render_num(writer, row, s, vis_tri_count, offset);
  }
  {
    char s[32] = "rendered tris:    ";
    int row = 2;
    render_num(writer, row, s, typen_tri_count, offset);
  }
}

void render_bounding_box_mm(ta_parameter_writer& writer, const mat4x4& trans, int mins[3], int maxs[3], uint32_t color)
{
  vec3 max = {(float)maxs[0], (float)maxs[1], (float)maxs[2]};
  vec3 min = {(float)mins[0], (float)mins[1], (float)mins[2]};
  render_bounding_box(writer, trans, max, min, color);
}

void render_bounding_boxes(ta_parameter_writer& writer, const mat4x4& trans)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  q3bsp_leaf_t * leafs = reinterpret_cast<q3bsp_leaf_t *>(&buf[le->offset]);

  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);
  q3bsp_node_t * root = &nodes[root_ix];

  {
    if (root->children[0] >= 0) {
      q3bsp_node_t * a = &nodes[root->children[0]];
      uint32_t color = 0x80ff00e6;
      render_bounding_box_mm(writer, trans, a->mins, a->maxs, color);
    } else {
      int leaf_ix = -(root->children[0] + 1);
      q3bsp_leaf_t * leaf = &leafs[leaf_ix];
      uint32_t color = 0x80ff0016;
      render_bounding_box_mm(writer, trans, leaf->maxs, leaf->mins, color);
    }

    if (root->children[1] >= 0) {
      q3bsp_node_t * b = &nodes[root->children[1]];
      uint32_t color = 0x8000ffe6;
      render_bounding_box_mm(writer, trans, b->mins, b->maxs, color);
    } else {
      int leaf_ix = -(root->children[1] + 1);
      q3bsp_leaf_t * leaf = &leafs[leaf_ix];
      uint32_t color = 0x8000ff16;
      render_bounding_box_mm(writer, trans, leaf->maxs, leaf->mins, color);
    }
  }
}

bool vec3_in_bb(vec3 v, int mins[3], int maxs[3])
{
  return
    v.x >= mins[0] &&
    v.y >= mins[1] &&
    v.z >= mins[2] &&
    v.x <= maxs[0] &&
    v.y <= maxs[1] &&
    v.z <= maxs[2];
}

void render_leaf_faces(ta_parameter_writer& writer, const mat4x4& trans, q3bsp_leaf_t * leaf)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  //int leafface 	First leafface for leaf.
  //int n_leaffaces 	Number of leaffaces for leaf.

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  q3bsp_direntry * lef = &header->direntries[LUMP_LEAFFACES];
  q3bsp_leafface_t * leaffaces = reinterpret_cast<q3bsp_leafface_t *>(&buf[lef->offset]);

  q3bsp_leafface_t * lf = &leaffaces[leaf->leafface];

  int last_texture = -1;
  int last_lm_index = -1;

  const int textures_length = (sizeof (textures)) / (sizeof (textures[0]));

  for (int i = 0; i < leaf->n_leaffaces; i++) {
    int face_ix = lf[i].face;
    if (face_cache[face_ix] != 0)
      continue;
    face_cache[face_ix] = 1;

    q3bsp_face_t * face = &faces[face_ix];

    bool has_texture =
      (face->texture >= 0) &&
      (face->texture < textures_length) &&
      (textures[face->texture].size != 0);

    if (!has_texture)
      continue;

    if (face->texture != last_texture || face->lm_index != last_lm_index) {
      last_texture = face->texture;
      last_lm_index = face->lm_index;

      global_texture_lightmap(writer, face->texture, face->lm_index);
    }

    if (face->type == FACE_TYPE_POLYGON || face->type == FACE_TYPE_MESH)
      transfer_face_meshverts(writer, face);
    if (face->type == FACE_TYPE_PATCH)
      transfer_face_patch_surfaces(writer, trans, face, face_ix);
  }
}

q3bsp_leaf_t * bb_leaf = NULL;
q3bsp_leaf_t * mm_leaf = NULL;

q3bsp_leaf_t * find_leaf(const vec3 pos)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  q3bsp_leaf_t * leafs = reinterpret_cast<q3bsp_leaf_t *>(&buf[le->offset]);

  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);
  q3bsp_node_t * root = &nodes[0];

  while (true) {
    bool a_inside;
    bool b_inside;
    q3bsp_node_t * new_root = NULL;
    if (root->children[0] >= 0) {
      q3bsp_node_t * node = &nodes[root->children[0]];
      a_inside = vec3_in_bb(pos, node->mins, node->maxs);
      if (a_inside) {
        new_root = node;
      }
    } else {
      int leaf_ix = -(root->children[0] + 1);
      q3bsp_leaf_t * leaf = &leafs[leaf_ix];
      a_inside = vec3_in_bb(pos, leaf->mins, leaf->maxs);
      if (a_inside) {
        return leaf;
      }
    }
    if (root->children[1] >= 0) {
      q3bsp_node_t * node = &nodes[root->children[1]];
      b_inside = vec3_in_bb(pos, node->mins, node->maxs);
      if (b_inside) {
        new_root = node;
      }
    } else {
      int leaf_ix = -(root->children[1] + 1);
      q3bsp_leaf_t * leaf = &leafs[leaf_ix];
      b_inside = vec3_in_bb(pos, leaf->mins, leaf->maxs);
      if (b_inside) {
        return leaf;
      }
    }

    if (!(a_inside || b_inside))
      return nullptr;
    assert(a_inside || b_inside);
    assert(new_root != NULL);
    root = new_root;
  }
}

void render_visible_faces(ta_parameter_writer& writer, const mat4x4& trans, const vec3 pos)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  q3bsp_leaf_t * leafs = reinterpret_cast<q3bsp_leaf_t *>(&buf[le->offset]);

  q3bsp_direntry * ve = &header->direntries[LUMP_VISDATA];
  q3bsp_visdata_t * visdata = reinterpret_cast<q3bsp_visdata_t *>(&buf[ve->offset]);

  bb_leaf = find_leaf(pos);
  if (bb_leaf == NULL)
    return;

  //uint32_t color = 0x8000ff16;
  //render_bounding_box_mm(writer, trans, bb_leaf->maxs, bb_leaf->mins, color);
  render_leaf_faces(writer, trans, bb_leaf);

  int leaf_count = le->length / (sizeof (struct q3bsp_leaf));
  for (int i = 0; i < leaf_count; i++) {
    q3bsp_leaf_t * leaf = &leafs[i];

    //  Cluster x is visible from cluster y if the (1 << y % 8) bit of vecs[x * sz_vecs + y / 8] is set.
    /*
    if (leaf->mins[2] > 450 && leaf->maxs[2] > 450)
      continue;
    */

    int y = bb_leaf->cluster;
    int x = leaf->cluster;
    bool visible = (visdata->vecs[x * visdata->sz_vecs + y / 8] & (1 << (y % 8))) != 0;
    if (visible) {
      //uint32_t color = 0x40ff00e6;
      //render_bounding_box_mm(writer, trans, leaf->maxs, leaf->mins, color);
      render_leaf_faces(writer, trans, leaf);
    }
  }
}

void transfer_modifier_volume(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque_modifier_volume;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(parameter_control_word,
					 isp_tsp_instruction_word
					 );

  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         0, 0, 1,
                                         640, 0, 1,
                                         640, 480, 1);

  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                             | para_control::list_type::opaque_modifier_volume
                                             | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                               | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
                                         last_isp_tsp_instruction_word);

  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         0, 0, 1,
                                         640, 480, 1,
                                         0, 480, 1);
}

void transfer_brushes(ta_parameter_writer& writer, const mat4x4& trans);

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans, const mat4x4& screen_trans_inv)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  const mat4x4 trans = screen_trans;

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  transform_vertices(&buf[ve->offset], ve->length, trans);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  int face_count = fe->length / (sizeof (struct q3bsp_face));

  // opaque list
  {
    transfer_icosphere(writer, trans);
    //transfer_tavion(writer, trans);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }

  // punch through list
  {
    //vec3 pos = screen_trans_inv * (vec3){0, 0, 0};
    vec3 pos = sphere_position;
    typen_tri_count = 0;
    vis_tri_count = 0;
    for (int i = 0; i < face_count; i++) face_cache[i] = 0;
    render_visible_faces(writer, trans, pos);
    //render_tris_count(writer);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }

  // translucent list
  {
    //render_bounding_box_mm(writer, trans, mm_leaf->maxs, mm_leaf->mins, 0x4000ff00);

    transfer_billboard(writer, trans);

    //transfer_brushes(writer, trans);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }

  // modifier volume list
  {
    transfer_modifier_volume(writer);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 3];

constexpr inline mat4x4 rotate_x(float t)
{
  mat4x4 r = {
    1, 0, 0, 0,
    0, cos(t), -sin(t), 0,
    0, sin(t), cos(t), 0,
    0, 0, 0, 1,
  };
  return r;
}

constexpr inline mat4x4 rotate_y(float t)
{
  mat4x4 r = {
     cos(t), 0, sin(t), 0,
    0, 1, 0, 0,
    -sin(t), 0, cos(t), 0,
    0, 0, 0, 1,
  };
  return r;
}

constexpr inline mat4x4 rotate_z(float t)
{
  mat4x4 r = {
    cos(t), -sin(t), 0, 0,
    sin(t), cos(t), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };
  return r;
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  assert((((int)dst) & 31) == 0);
  assert((((int)length) & 31) == 0);

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

void transfer_lightmaps()
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * lme = &header->direntries[LUMP_LIGHTMAPS];
  q3bsp_lightmap_t * lightmaps = reinterpret_cast<q3bsp_lightmap_t *>(&buf[lme->offset]);
  int count = lme->length / (sizeof (struct q3bsp_lightmap));

  uint16_t temp[128 * 128];

  lightmap_base = 0;

  for (int i = 0; i < count; i++) {
    q3bsp_lightmap_t * lightmap = &lightmaps[i];

    for (int j = 0; j < 128 * 128; j++) {
      uint8_t * c = &lightmap->u8[j * 3];
      temp[j] = rgb565(c[0], c[1], c[2]);
    }

    uint32_t offset = texture_memory_alloc.texture.start + font_base + lightmap_base;
    assert((offset & 31) == 0); // lightmap
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    uint32_t size = 128 * 128 * 2;
    transfer_ta_fifo_texture_memory_32byte(dst, temp, size);

    lightmap_base += 128 * 128 * 2;
  }
}

void transfer_bsp_textures()
{
  const int textures_length = (sizeof (textures)) / (sizeof (textures[0]));

  bsp_base = 0;

  for (int i = 0; i < textures_length; i++) {
    uint32_t offset = texture_memory_alloc.texture.start + font_base + lightmap_base + textures[i].offset;
    assert((offset & 31) == 0); // bsp
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = textures[i].start;
    uint32_t size = textures[i].size;
    size = (size + 31) & (~31);
    assert((size & 31) == 0);
    assert(offset + size < 0x800000);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);

    bsp_base += (int)size;
  }
}

void transfer_tavion_textures()
{
  const int textures_length = (sizeof (tavion_textures)) / (sizeof (tavion_textures[0]));

  tavion_base = 0;

  for (int i = 0; i < textures_length; i++) {
    uint32_t offset = texture_memory_alloc.texture.start + font_base + lightmap_base + bsp_base + tavion_textures[i].offset;
    assert((offset & 31) == 0); // tavion
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = tavion_textures[i].start;
    uint32_t size = tavion_textures[i].size;
    size = (size + 31) & ~31;
    assert(offset + size < 0x800000);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);

    tavion_base += (int)size;
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  transfer_lightmaps();
  printf("lightmap base: %d\n", lightmap_base);
  transfer_bsp_textures();
  printf("bsp base: %d\n", bsp_base);
  transfer_tavion_textures();
  printf("tavion base: %d\n", tavion_base);

  int total = 8 * 1024 * 1024;
  int used = texture_memory_alloc.texture.start
    + font_base
    + lightmap_base
    + bsp_base
    + tavion_base;

  printf("texture memory free %d\n", total - used);
}

static uint8_t brush_cache[2048];
//static volatile int brushside_comparisons;

static inline bool collision_brush(q3bsp_plane_t * planes,
                                   q3bsp_brushside_t * brushsides,
                                   int n_brushsides,
                                   vec3 pos)
{
  for (int i = 0; i < n_brushsides; i++) {
    q3bsp_brushside_t * brushside = &brushsides[i];
    q3bsp_plane_t * plane = &planes[brushside->plane];

    vec4 plane_eq = {plane->normal[0], plane->normal[1], plane->normal[2], -plane->dist};
    vec4 position = {pos.x, pos.y, pos.z, 1.0f};
    float sign = dot(plane_eq, position);

    //brushside_comparisons += 1;

    if (sign > 0)
      return true;
  }

  return false;
}

static inline bool collision_leaf(q3bsp_leaf_t * leaf, vec3 pos)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * br = &header->direntries[LUMP_BRUSHES];
  q3bsp_brush_t * brushes = reinterpret_cast<q3bsp_brush_t *>(&buf[br->offset]);

  q3bsp_direntry * lbr = &header->direntries[LUMP_LEAFBRUSHES];
  q3bsp_leafbrush_t * leafbrushes = reinterpret_cast<q3bsp_leafbrush_t *>(&buf[lbr->offset]);

  q3bsp_direntry * bs = &header->direntries[LUMP_BRUSHSIDES];
  q3bsp_brushside_t * brushsides = reinterpret_cast<q3bsp_brushside_t *>(&buf[bs->offset]);

  q3bsp_direntry * p = &header->direntries[LUMP_PLANES];
  q3bsp_plane_t * planes = reinterpret_cast<q3bsp_plane_t *>(&buf[p->offset]);

  q3bsp_leafbrush_t * lbs = &leafbrushes[leaf->leafbrush];
  for (int i = 0; i < leaf->n_leafbrushes; i++) {
    int brush_ix = lbs[i].brush;
    if (brush_cache[brush_ix])
      continue;
    brush_cache[brush_ix] = true;

    q3bsp_brush_t * brush = &brushes[brush_ix];
    bool col = collision_brush(planes,
                               &brushsides[brush->brushside],
                               brush->n_brushsides,
                               pos);
    if (col)
      return true;
  }

  return false;
}

bool collision(vec3 a, vec3 b)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);
  q3bsp_direntry * br = &header->direntries[LUMP_BRUSHES];

  int brush_count = br->length / (sizeof (struct q3bsp_brush));

  for (int i = 0; i < brush_count; i++)
    brush_cache[i] = false;

  //brushside_comparisons = 0;

  q3bsp_leaf_t * a_leaf = find_leaf(a);
  if (a_leaf == nullptr) {
    printf("a_leaf null\n");
    return true;
  }
  q3bsp_leaf_t * b_leaf = find_leaf(b);
  if (b_leaf == nullptr) {
    printf("b_leaf null\n");
    return true;
  }

  if (collision_leaf(a_leaf, a))
    return true;

  if (collision_leaf(b_leaf, b))
    return true;

  //printf("brushside_comparisons: %d\n", brushside_comparisons);

  return false;
}

static bool push = false;

mat4x4 update_analog(const mat4x4& screen)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;
  int da = ft0::data_transfer::digital_button::da(data[0].digital_button) == 0;
  int ua = ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0;

  int db_a = ft0::data_transfer::digital_button::a(data[0].digital_button) == 0;
  int db_b = ft0::data_transfer::digital_button::b(data[0].digital_button) == 0;
  int db_x = ft0::data_transfer::digital_button::x(data[0].digital_button) == 0;
  int db_y = ft0::data_transfer::digital_button::y(data[0].digital_button) == 0;

  float x = 0;
  if (ra && !la) x = -10;
  if (la && !ra) x =  10;

  float y = -7 * y_;

  /*
  float z = 0;
  if (ua && !da) z = -10;
  if (da && !ua) z =  10;
  */
  float z = -7.0f * r_ + 7.0f * l_;

  mat4x4 t = {
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1,
  };

  float yt = -0.05f * x_;
  float xt = 0.05f * y_;

  /*
  mat4x4 rx = {
    1, 0, 0, 0,
    0, cos(xt), -sin(xt), 0,
    0, sin(xt), cos(xt), 0,
    0, 0, 0, 1,
  };
  */

  mat4x4 ry = {
     cos(yt), 0, sin(yt), 0,
    0, 1, 0, 0,
    -sin(yt), 0, cos(yt), 0,
    0, 0, 0, 1,
  };

  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);
  //q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  //int num_leaves = le->length / (sizeof (struct q3bsp_leaf));
  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);

  //printf("%d %d\n", draw_tavion_surface, tavion_surface[draw_tavion_surface]);
  if (0) {
    if (db_x && !db_y && !push) {
      push = true;
      //leaf_ix -= 1;
      //if (leaf_ix < 0) leaf_ix = num_leaves - 1;

      /*
      int ix = nodes[root_ix].children[0];
      if (ix >= 0)
        root_ix = ix;
      */
    }
    if (db_y && !db_x && !push) {
      push = true;
      //leaf_ix += 1;
      //if (leaf_ix > num_leaves) leaf_ix = 0;
      int ix = nodes[root_ix].children[1];
      if (ix >= 0)
        root_ix = ix;
    }
    if (!db_x && !db_y) {
      push = false;
    }
  } else if (1) {
    vec3 destination = {sphere_position.x, sphere_position.y, sphere_position.z};

    if (db_x && !db_b) {
      destination.x -= 10;
    }
    if (db_b && !db_x) {
      destination.x += 10;
    }
    if (db_y && !db_a) {
      destination.y += 10;
    }
    if (db_a && !db_y) {
      destination.y -= 10;
    }
    if (ua && !da) {
      destination.z += 10;
    }
    if (da && !ua) {
      destination.z -= 10;
    }

    if (db_x || db_b || db_y || db_a || ua || da) {
      if (!collision(sphere_position, destination)) {
        sphere_position = destination;
      } else {
        //serial::string("collision\n");
      }
    }
  }

  return ry * t * screen;
}

void transfer_font()
{
  const uint8_t * src = reinterpret_cast<const uint8_t *>(&_binary_font_verite_8x16_verite_8x16_data_start);
  uint32_t offset = font_bitmap::inflate(1,  // pitch
                                         8,  // width
                                         16, // height
                                         texture_memory_alloc.texture.start,
                                         8,  // texture_width
                                         16, // texture_height
                                         src);
  printf("font_base %d actual %d\n", font_base, offset);
}

void vbr100()
{
  serial::string("vbr100\n");
  interrupt_exception();
}

void vbr400()
{
  serial::string("vbr400\n");
  interrupt_exception();
}

const int framebuffer_width = 640;
const int framebuffer_height = 480;
const int tile_width = framebuffer_width / 32;
const int tile_height = framebuffer_height / 32;

constexpr uint32_t ta_alloc = 0
                            | ta_alloc_ctrl::pt_opb::_32x4byte
                            | ta_alloc_ctrl::tm_opb::no_list
                            | ta_alloc_ctrl::t_opb::_8x4byte
                            | ta_alloc_ctrl::om_opb::_8x4byte
                            | ta_alloc_ctrl::o_opb::_8x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 8 * 4,
    .opaque_modifier = 8 * 4,
    .translucent = 8 * 4,
    .translucent_modifier = 0,
    .punch_through = 32 * 4
  }
};

static volatile int ta_in_use = 0;
static volatile int core_in_use = 0;
static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static inline void pump_events(uint32_t istnrm)
{
  if (istnrm & istnrm::v_blank_in) {
    system.ISTNRM = istnrm::v_blank_in;

    next_frame = 1;
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[next_frame_ix].start;
  }

  if (istnrm & istnrm::end_of_render_tsp) {
    system.ISTNRM = istnrm::end_of_render_tsp
                  | istnrm::end_of_render_isp
                  | istnrm::end_of_render_video;

    next_frame_ix = framebuffer_ix;
    framebuffer_ix += 1;
    if (framebuffer_ix >= 3) framebuffer_ix = 0;

    core_in_use = 0;
  }

  if (istnrm & istnrm::end_of_transferring_opaque_modifier_volume_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_modifier_volume_list;

    core_in_use = 1;
    core_start_render2(texture_memory_alloc.region_array.start,
                       texture_memory_alloc.isp_tsp_parameters.start,
                       texture_memory_alloc.background[0].start,
                       texture_memory_alloc.framebuffer[framebuffer_ix].start,
                       framebuffer_width);

    ta_in_use = 0;
  }
}

void vbr600()
{
  uint32_t sr;
  asm volatile ("stc sr,%0" : "=r" (sr));
  sr |= sh::sr::imask(15);
  asm volatile ("ldc %0,sr" : : "r" (sr));
  //serial::string("imask\n");

  //check_pipeline();

  if (sh7091.CCN.EXPEVT == 0 && sh7091.CCN.INTEVT == 0x320) {
    uint32_t istnrm = system.ISTNRM;
    uint32_t isterr = system.ISTERR;

    if (isterr) {
      serial::string("isterr: ");
      serial::integer<uint32_t>(system.ISTERR);
    }

    pump_events(istnrm);

    sr &= ~sh::sr::imask(15);
    asm volatile ("ldc %0,sr" : : "r" (sr));

    return;
  }

  serial::string("vbr600\n");
  interrupt_exception();
}

/*
  typedef struct q3bsp_plane {
  float normal[3];
  float dist;
} q3bsp_plane_t;
*/

uint32_t colors[] = {
  0xffffff,
  0xfcf400,
  0xff6400,
  0xdd0202,
  0xf10285,
  0x4600a6,
  0x0000d5,
  0x00aee9,
  0x1ab90c,
  0x006408,
  0x582800,
  0x917135,
  0xc1c1c1,
  0x818181,
  0x3e3efe,
  0x000000,
};
uint32_t colors2[] = {
  0x7f7f7f,
  0x7e7a00,
  0x7f3200,
  0x6e0101,
  0x780142,
  0x230053,
  0x00006a,
  0x005774,
  0x0d5c06,
  0x003204,
  0x2c1400,
  0x48381a,
  0x606060,
  0x404040,
  0x1f1f7f,
  0x000000,
};

static_assert((sizeof (colors)) / (sizeof (colors[0])) == 16);

void transfer_line(ta_parameter_writer& writer, vec3 p1, vec3 p2, uint32_t base_color)
{
  float dy = p2.y - p1.y;
  float dx = p2.x - p1.x;
  float d = _fsrra(dx * dx + dy * dy) * 0.7f;
  float dy1 = dy * d;
  float dx1 = dx * d;

  //assert(p1.z < 1);
  //assert(p2.z < 1);

  const vec3 v[4] = {
    { p1.x +  dy1, p1.y + -dx1, p1.z },
    { p1.x + -dy1, p1.y +  dx1, p1.z },
    { p2.x + -dy1, p2.y +  dx1, p2.z },
    { p2.x +  dy1, p2.y + -dx1, p2.z },
  };

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v[0].x, v[0].y, v[0].z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v[1].x, v[1].y, v[1].z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        v[3].x, v[3].y, v[3].z,
                                        base_color);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        v[2].x, v[2].y, v[2].z,
                                        base_color);
}

vec3 perpendicular(vec3 n)
{
  int b0 = (abs(n[0]) <  abs(n[1])) && (abs(n[0]) <  abs(n[2]));
  int b1 = (abs(n[1]) <= abs(n[0])) && (abs(n[1]) <  abs(n[2]));
  int b2 = (abs(n[2]) <= abs(n[0])) && (abs(n[2]) <= abs(n[1]));

  return cross(n, {(float)b0, (float)b1, (float)b2});
}

vec3 rodrigues_rotation(vec3 k, vec3 v, float theta)
{
  // k - axis of rotation
  // v - vector to be rotated

  float ct = cos(theta);
  float st = sin(theta);

  vec3 v_rot = v * ct + cross(k, v) * st + k * dot(k, v) * (1 - ct);

  return v_rot;
}

void transfer_brushsides(ta_parameter_writer& writer,
                         const mat4x4& trans,
                         q3bsp_plane_t * planes,
                         q3bsp_brushside_t * brushsides,
                         int n_brushsides)
{
  for (int i = 0; i < n_brushsides; i++) {
    q3bsp_brushside_t * brushside = &brushsides[i];
    q3bsp_plane_t * plane = &planes[brushside->plane];

    vec4 plane_eq = {plane->normal[0], plane->normal[1], plane->normal[2], -plane->dist};
    vec4 pos = {sphere_position.x, sphere_position.y, sphere_position.z, 1.0f};
    float sign = dot(plane_eq, pos);

    uint32_t base_color;
    if (sign > 0)
      base_color = 0x50000000 | colors[i & 15];
    else
      base_color = 0x60000000 | 0;

    vec3 a = {0, 0, 0};
    vec3 normal = {plane->normal[0], plane->normal[1], plane->normal[2]};
    //printf("%f %f %f\n", normal.x, normal.y, normal.z);
    vec3 b = normal * plane->dist;

    vec3 ap = trans * a;
    vec3 bp = trans * b;

    if (ap.z < 0 || bp.z < 0)
      continue;

    /*
    transfer_line(writer,
                  screen_transform(ap),
                  screen_transform(bp),
                  base_color);
    */
    transfer_line(writer,
                  screen_transform(bp),
                  screen_transform(trans * (b + normal * 100.f)),
                  base_color);

    vec3 perp = perpendicular(normal);

    float scale = 500;
    vec3 cp1 = trans * (b + (perp * scale));
    vec3 cp2 = trans * (b + (perp * -scale));

    /*
    transfer_line(writer,
                  screen_transform(bp),
                  screen_transform(cp),
                  base_color);
    */

    vec3 perp2 = rodrigues_rotation(normal, perp, pi / 2.0f);

    vec3 dp1 = trans * (b + (perp2 * scale));
    vec3 dp2 = trans * (b + (perp2 * -scale));

    /*
    transfer_line(writer,
                  screen_transform(bp),
                  screen_transform(dp),
                  base_color);
    */

    render_quad(writer,
                screen_transform(cp1),
                screen_transform(dp1),
                screen_transform(cp2),
                screen_transform(dp2),
                base_color);
  }
}

void transfer_brushes(ta_parameter_writer& writer, const mat4x4& trans)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * br = &header->direntries[LUMP_BRUSHES];
  q3bsp_brush_t * brushes = reinterpret_cast<q3bsp_brush_t *>(&buf[br->offset]);

  q3bsp_direntry * lbr = &header->direntries[LUMP_LEAFBRUSHES];
  q3bsp_leafbrush_t * leafbrushes = reinterpret_cast<q3bsp_leafbrush_t *>(&buf[lbr->offset]);

  q3bsp_direntry * bs = &header->direntries[LUMP_BRUSHSIDES];
  q3bsp_brushside_t * brushsides = reinterpret_cast<q3bsp_brushside_t *>(&buf[bs->offset]);

  q3bsp_direntry * p = &header->direntries[LUMP_PLANES];
  q3bsp_plane_t * planes = reinterpret_cast<q3bsp_plane_t *>(&buf[p->offset]);

  //q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  //q3bsp_leaf_t * leafs = reinterpret_cast<q3bsp_leaf_t *>(&buf[le->offset]);

  int brush_count = br->length / (sizeof (struct q3bsp_brush));

  //printf("brush_count %d\n", brush_count);

  global_polygon_type_0(writer);

  //for (int i = 0; i < brush_count; i++) {

  //printf("leaf_ix %d\n", bb_leaf - leafs);

  q3bsp_leaf_t * leaf = bb_leaf;
  mm_leaf = leaf;
  q3bsp_leafbrush_t * lbs = &leafbrushes[leaf->leafbrush];

  //printf("n_lbs %d\n", leaf->n_leafbrushes);
  for (int i = 0; i < leaf->n_leafbrushes; i++) {

    q3bsp_brush_t * brush = &brushes[lbs[i].brush];
    transfer_brushsides(writer,
                        trans,
                        planes,
                        &brushsides[brush->brushside],
                        brush->n_brushsides);
    break;
  }
}

void debug_bsp()
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * me = &header->direntries[LUMP_MODELS];
  q3bsp_model_t * models = reinterpret_cast<q3bsp_model_t *>(&buf[me->offset]);

  int model_count = me->length / (sizeof (struct q3bsp_model));
  printf("model %d\n", model_count);
  printf("      %d %d\n", models[0].n_faces, models[0].face);
  const float * mins = models[0].mins;
  const float * maxs = models[0].mins;
  printf("      %f %f %f\n", mins[0], mins[1], mins[2]);
  printf("      %f %f %f\n", maxs[0], maxs[1], maxs[2]);

  printf("\n\n");
}

int main()
{
  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  serial::init(0);
  debug_bsp();

  total_tri_count = count_face_triangles();

  interrupt_init();
  transfer_textures();
  transfer_font();
  palette_data<3>();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.ISP_FEED_CFG  = isp_feed_cfg::cache_size_for_translucency(0x200)
                      | isp_feed_cfg::punch_through_chunk_size(0x040)
                      | isp_feed_cfg::pre_sort_mode
                      ;

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_modifier_volume_list;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff202040);

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  video_output::set_mode_vga();

  /*
  mat4x4 trans1 = {
    1.0,  0.0,   0.000, -1123.0,
    0.0,  -0.888, -0.458,  859.0,
    0.0,  0.458, -0.888,  791.0,
    0.0,  0.000, 0.000,    1.0,
  };
  */
  mat4x4 trans = {
    1.0,   0.0,   0.0, -894.0,
    0.0,   0.0,  -1.0,  451.0,
    0.0,   1.0,   0.0, -465.0,
    0.0,   0.0,   0.0,    1.0,
  };

  q3bsp_patch::triangulate_patches(bsp_start);
  printf("patch_count %d\n", q3bsp_patch::patch_count);

  do_get_condition();

  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    trans = update_analog(trans);
    mat4x4 trans_inv = inverse(trans);
    writer.offset = 0;
    transfer_scene(writer, trans, trans_inv);

    while (ta_in_use);
    while (core_in_use);
    ta_in_use = 1;
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters.start,
                               texture_memory_alloc.isp_tsp_parameters.end,
                               texture_memory_alloc.object_list.start,
                               texture_memory_alloc.object_list.end,
                               opb_size[0].total(),
                               ta_alloc,
                               tile_width,
                               tile_height);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);

    while (next_frame == 0);
    next_frame = 0;
  }
}
