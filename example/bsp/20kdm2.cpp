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
#include "bsp/20kdm2/textures/sfx/flame2.data.h"
#include "bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2.data.h"
#include "bsp/20kdm2/models/mapobjects/gratelamp/gratetorch2b.data.h"

#include "q3bsp/q3bsp.h"
#include "bsp/20kdm2/maps/20kdm2.bsp.h"
#include "bsp/20kdm2/texture.h"

#include "model/model.h"
#include "model/icosphere/model.h"

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"
#include "palette.hpp"
#include "printf/unparse.h"

#include "assert.h"

constexpr int font_offset = ((0x7f - 0x20) + 1) * 8 * 16 / 2;

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

static vec3 sphere_position = {890, 480, 450};

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
                           uint32_t obj_control_texture,
                           uint32_t texture_u_v_size,
                           uint32_t texture_control_word,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control_texture
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::cull_if_negative
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::bilinear_filter
                                      | tsp_instruction_word::texture_shading_instruction::modulate
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

void global_texture(ta_parameter_writer& writer, int ix)
{
  struct pk_texture * texture = &textures[ix];

  uint32_t texture_u_v_size = tsp_instruction_word::texture_u_size::from_int(texture->width)
                            | tsp_instruction_word::texture_v_size::from_int(texture->height)
                            ;

  uint32_t texture_address = texture_memory_alloc.texture.start + font_offset + texture->offset * 2;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::non_twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  global_polygon_type_1(writer,
                        obj_control::texture,
                        texture_u_v_size,
                        texture_control_word);
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

static int type7_tri_count = 0;
static int vis_tri_count = 0;
static int total_tri_count = 0;

static inline void render_tri_type_7(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     vec2 at,
                                     vec2 bt,
                                     vec2 ct,
                                     float ai,
                                     float bi,
                                     float ci)
{
  type7_tri_count += 1;

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
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        ci,
                                        0);
}

static inline void render_clip_tri_type_7(ta_parameter_writer& writer,
                                          vec3 ap,
                                          vec3 bp,
                                          vec3 cp,
                                          vec2 at,
                                          vec2 bt,
                                          vec2 ct,
                                          float li)
{
  //return;
  const vec3 plane_point = {0.f, 0.f, 1.f};
  const vec3 plane_normal = {0.f, 0.f, 1.f};

  vec3 preclip_position[] = {ap, bp, cp};
  vec2 preclip_texture[] = {at, bt, ct};

  vec3 clip_position[4];
  vec2 clip_texture[4];
  int output_length = geometry::clip_polygon_uv<3>(clip_position,
                                                   clip_texture,
                                                   plane_point,
                                                   plane_normal,
                                                   preclip_position,
                                                   preclip_texture);

  {
    vec3 ap;
    vec3 bp;
    vec3 cp;
    vec3 dp;

    const vec2& at = clip_texture[0];
    const vec2& bt = clip_texture[1];
    const vec2& ct = clip_texture[2];
    const vec2& dt = clip_texture[3];
    if (output_length >= 3) {
      ap = screen_transform(clip_position[0]);
      bp = screen_transform(clip_position[1]);
      cp = screen_transform(clip_position[2]);

      render_tri_type_7(writer,
                        ap,
                        bp,
                        cp,
                        at,
                        bt,
                        ct,
                        li,
                        li,
                        li);
    }
    if (output_length >= 4) {
      dp = screen_transform(clip_position[3]);

      render_tri_type_7(writer,
                        ap,
                        cp,
                        dp,
                        at,
                        ct,
                        dt,
                        li,
                        li,
                        li);
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
    intensity += 0.7f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
    if (intensity > 1.0f)
      intensity = 1.0f;
  }
  return intensity;
}

static vec3 light_vec = {20, -20, -20};

static inline void transfer_face(ta_parameter_writer& writer, q3bsp_face_t * face, int * last_texture)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);

  q3bsp_direntry * me = &header->direntries[LUMP_MESHVERTS];
  q3bsp_meshvert_t * meshvert = reinterpret_cast<q3bsp_meshvert_t *>(&buf[me->offset]);

  int meshvert_ix = face->meshvert;
  q3bsp_meshvert_t * mv = &meshvert[meshvert_ix];

  int triangles = face->n_meshverts / 3;

  int textures_length = (sizeof (textures)) / (sizeof (textures[0]));

  bool has_texture = 1 &&
    (face->texture >= 0) &&
    (face->texture < textures_length) &&
    (textures[face->texture].size != 0);

  if (face->texture != *last_texture) {
    *last_texture = face->texture;
    if (has_texture) {
      global_texture(writer, face->texture);
    } else {
      //global_polygon_type_1(writer, 0, 0, 0);
    }
  }

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
    float li = light_intensity(light_vec, n);


    if (has_texture) {
      float v_mul = textures[face->texture].v_mul;
      vec2 at = {vert[aix].texcoord[0], vert[aix].texcoord[1] * v_mul};
      vec2 bt = {vert[bix].texcoord[0], vert[bix].texcoord[1] * v_mul};
      vec2 ct = {vert[cix].texcoord[0], vert[cix].texcoord[1] * v_mul};

      if (ap.z < 0 || bp.z < 0 || cp.z < 0) {
        render_clip_tri_type_7(writer,
                               ap,
                               bp,
                               cp,
                               at,
                               bt,
                               ct,
                               li);

      } else {
        render_tri_type_7(writer,
                          screen_transform(ap),
                          screen_transform(bp),
                          screen_transform(cp),
                          at,
                          bt,
                          ct,
                          li,
                          li,
                          li);
      }
    } else {
      /*
      render_tri_type_2(writer,
                        screen_transform(ap),
                        screen_transform(bp),
                        screen_transform(cp),
                        li,
                        li,
                        li);
      */
    }
  }
}

void transfer_faces(ta_parameter_writer& writer)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  int last_texture = -1;

  for (int i = 0; i < face_count; i++) {
    transfer_face(writer, &faces[i], &last_texture);
  }
}

int count_face_triangles()
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
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
  global_polygon_type_1(writer, 0, 0, 0, a, r, g, b);

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
                      li,
                      li,
                      li);
  }
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
                                8,  16, // texture
                                8,  16, // glyph
                                16 + 50 * 8, // position x
                                16 + row * 16, // position y
                                s, offset,
                                para_control::list_type::opaque);
}

void render_leaf_ix(ta_parameter_writer& writer)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
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
    render_num(writer, row, s, type7_tri_count, offset);
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
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
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
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  //int leafface 	First leafface for leaf.
  //int n_leaffaces 	Number of leaffaces for leaf.

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  q3bsp_face_t * faces = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  q3bsp_direntry * lef = &header->direntries[LUMP_LEAFFACES];
  q3bsp_leafface_t * leaffaces = reinterpret_cast<q3bsp_leafface_t *>(&buf[lef->offset]);

  q3bsp_leafface_t * lf = &leaffaces[leaf->leafface];

  int last_texture = -1;

  for (int i = 0; i < leaf->n_leaffaces; i++) {
    int face_ix = lf[i].face;
    if (face_cache[face_ix] != 0)
      continue;
    face_cache[face_ix] = 1;
    transfer_face(writer, &faces[face_ix], &last_texture);
  }
}

void render_visible_faces(ta_parameter_writer& writer, const mat4x4& trans, const vec3 pos)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  q3bsp_leaf_t * leafs = reinterpret_cast<q3bsp_leaf_t *>(&buf[le->offset]);

  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);
  q3bsp_node_t * root = &nodes[0];

  q3bsp_direntry * ve = &header->direntries[LUMP_VISDATA];
  q3bsp_visdata_t * visdata = reinterpret_cast<q3bsp_visdata_t *>(&buf[ve->offset]);

  q3bsp_leaf_t * bb_leaf = NULL;

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
        bb_leaf = leaf;
        break;
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
        bb_leaf = leaf;
        break;
      }
    }

    /*
    if (!(a_inside ^ b_inside)) {
      printf("root_ix %d\n", root - nodes);
    }
    */

    assert(a_inside || b_inside);
    //assert(new_root != NULL);
    root = new_root;
  }

  assert(bb_leaf != NULL);
  uint32_t color = 0x8000ff16;
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
      uint32_t color = 0x40ff00e6;
      //render_bounding_box_mm(writer, trans, leaf->maxs, leaf->mins, color);
      render_leaf_faces(writer, trans, leaf);
    }
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans, const mat4x4& screen_trans_inv)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  const mat4x4 trans = screen_trans;

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  transform_vertices(&buf[ve->offset], ve->length, trans);

  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  int face_count = fe->length / (sizeof (struct q3bsp_face));

  //transfer_faces(writer);
  transfer_icosphere(writer, trans);

  render_matrix(writer, screen_trans);
  //render_leaf_ix(writer);
  render_sphere_position(writer);
  //render_zero_position(writer, screen_trans_inv);

  vec3 pos = screen_trans_inv * (vec3){0, 0, 0};
  type7_tri_count = 0;
  vis_tri_count = 0;
  for (int i = 0; i < face_count; i++) face_cache[i] = 0;
  render_visible_faces(writer, trans, pos);
  render_tris_count(writer);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  global_polygon_type_0(writer);
  render_quad(writer,
              {0, 0, 0},
              {0, 0, 0},
              {0, 0, 0},
              {0, 0, 0},
              0);

  root_ix = 1552;
  //render_bounding_boxes(writer, trans);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 2];

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

  int textures_length = (sizeof (textures)) / (sizeof (textures[0]));
  for (int i = 0; i < textures_length; i++) {
    uint32_t offset = texture_memory_alloc.texture.start + font_offset + textures[i].offset * 2;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = textures[i].start;
    uint32_t size = textures[i].size;
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
}

static bool push = false;

mat4x4 update_analog(const mat4x4& screen)
{
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

  float z = 0;
  if (ua && !da) z = -10;
  if (da && !ua) z =  10;

  mat4x4 t = {
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1,
  };

  float yt = 0.05f * x_;
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

  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_bsp_20kdm2_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);
  //q3bsp_direntry * le = &header->direntries[LUMP_LEAFS];
  //int num_leaves = le->length / (sizeof (struct q3bsp_leaf));
  q3bsp_direntry * ne = &header->direntries[LUMP_NODES];
  q3bsp_node_t * nodes = reinterpret_cast<q3bsp_node_t *>(&buf[ne->offset]);

  if (0) {
    if (db_x && !db_y && !push) {
      push = true;
      //leaf_ix -= 1;
      //if (leaf_ix < 0) leaf_ix = num_leaves - 1;

      int ix = nodes[root_ix].children[0];
      if (ix >= 0)
        root_ix = ix;
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
  } else {
    if (db_x && !db_b) {
      sphere_position.x -= 10;
    }
    if (db_b && !db_x) {
      sphere_position.x += 10;
    }
    if (db_y && !db_a) {
      sphere_position.y += 10;
    }
    if (db_a && !db_y) {
      sphere_position.y -= 10;
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
                                         8,  // texture_width
                                         16, // texture_height
                                         src);
  printf("font_offset %d actual %d\n", font_offset, offset);
}

int main()
{
  serial::init(0);

  total_tri_count = count_face_triangles();

  interrupt_init();
  transfer_textures();
  transfer_font();
  palette_data<3>();

  constexpr uint32_t ta_alloc = 0
                              | ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::_16x4byte
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int ta_cont_count = 1;
  constexpr struct opb_size opb_size[ta_cont_count] = {
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

  system.IML6NRM = istnrm::end_of_render_tsp;

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  for (int i = 0; i < 2; i++) {
    region_array_multipass(tile_width,
                           tile_height,
                           opb_size,
                           ta_cont_count,
                           texture_memory_alloc.region_array[i].start,
                           texture_memory_alloc.object_list[i].start);

    background_parameter2(texture_memory_alloc.background[i].start,
                          0xff202040);
  }

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  video_output::set_mode_vga();

  int ta = 0;
  int core = 0;

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

  do_get_condition();

  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    trans = update_analog(trans);

    mat4x4 trans_inv = inverse(trans);

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);

    writer.offset = 0;
    transfer_scene(writer, trans, trans_inv);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_translucent_list();
    //ta_wait_opaque_list();

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
