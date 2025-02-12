#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/software_ta.hpp"
#include "holly/texture_memory_alloc3.hpp"

#include "model/model.h"
#include "model/cube/model.h"
#include "model/plane/model.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3x3.hpp"
#include "math/mat4x4.hpp"
#include "math/math.hpp"


using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat3x3 = mat<3, 3, float>;
using mat4x4 = mat<4, 4, float>;

const float deg = 0.017453292519943295;

#define _fsrra(n) (1.0f / (sqrtf(n)))

static inline float inverse_length(vec3 v)
{
  float f = dot(v, v);
  return _fsrra(f);
}

static inline int max(int a, int b)
{
  return (a > b) ? a : b;
}

static inline int min(int a, int b)
{
  return (a > b) ? b : a;
}

void global_polygon_type_0(ta_parameter_writer& writer, bool shadow)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | (shadow ? obj_control::shadow : 0)
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

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
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

void global_polygon_type_1(ta_parameter_writer& writer, bool shadow, float r, float g, float b)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::intensity_mode_1
                                        | (shadow ? obj_control::shadow : 0)
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      ;

  const uint32_t texture_control_word = 0;

  const float alpha = 1.0f;

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

void transfer_line(ta_parameter_writer& writer, vec3 p1, vec3 p2, uint32_t base_color)
{
  float dy = p2.y - p1.y;
  float dx = p2.x - p1.x;
  float d = _fsrra(dx * dx + dy * dy) * 0.7f;
  float dy1 = dy * d;
  float dx1 = dx * d;

  assert(p1.z < 1);
  assert(p2.z < 1);

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

vec3 screen_transform(const mat4x4& screen, vec3 v)
{
  v = screen * v;

  float dim = 480 / 2.0 * 1.5;

  return {
    v.x / v.z * dim + 640 / 2.0f,
    v.y / v.z * dim + 480 / 2.0f,
    1 / v.z,
  };
}

void render_basis(ta_parameter_writer& writer, const mat4x4& screen)
{
  global_polygon_type_0(writer, false);

  vec3 origin = screen_transform(screen, {0, 0, 0});
  vec3 z = screen_transform(screen, {0, 0, 1});
  vec3 y = screen_transform(screen, {0, 1, 0});
  vec3 x = screen_transform(screen, {1, 0, 0});

  uint32_t base_color = 0xffffff;

  // magenta: Z
  transfer_line(writer, origin, z, base_color);

  // yellow: Y
  transfer_line(writer, origin, y, base_color);

  // cyan: X
  transfer_line(writer, origin, x, base_color);
}

//#define LINE_DRAWING 1

static inline void render_quad(ta_parameter_writer& writer,
                               uint32_t base_color,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp)
{
#ifdef LINE_DRAWING
  transfer_line(writer, ap, bp, base_color);
  transfer_line(writer, bp, cp, base_color);
  transfer_line(writer, cp, dp, base_color);
  transfer_line(writer, dp, ap, base_color);
#else
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
#endif
}

static inline void render_quad_type2(ta_parameter_writer& writer,
                                     float intensity,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     vec3 dp)
{
#ifdef LINE_DRAWING
#else
  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        intensity);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        intensity);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        intensity);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        intensity);
#endif
}

static inline void render_tri(ta_parameter_writer& writer,
                              uint32_t base_color,
                              vec3 ap,
                              vec3 bp,
                              vec3 cp)
{
#ifdef LINE_DRAWING
  transfer_line(writer, ap, bp, base_color);
  transfer_line(writer, bp, cp, base_color);
  transfer_line(writer, cp, ap, base_color);
#else
#endif
}

static inline void render_last_tri_mod(ta_parameter_writer& writer)
{
#ifdef LINE_DRAWING
#else
  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                             | para_control::list_type::opaque_modifier_volume
                                             | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                               | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
                                         last_isp_tsp_instruction_word);
#endif
}

static inline void render_tri_mod(ta_parameter_writer& writer,
                                  vec3 ap,
                                  vec3 bp,
                                  vec3 cp)
{
#ifdef LINE_DRAWING
  transfer_line(writer, ap, bp, base_color);
  transfer_line(writer, bp, cp, base_color);
  transfer_line(writer, cp, ap, base_color);
#else
  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         ap.x, ap.y, ap.z,
                                         bp.x, bp.y, bp.z,
                                         cp.x, cp.y, cp.z);
#endif
}

void set_edge_coloring(uint8_t * edge_coloring,
                       const int edge_stride,
                       bool l_dot_n_b, int a, int b)
{
  int ma = min(a, b);
  int mb = max(a, b);

  int bit = 1 << ((int)l_dot_n_b);

  edge_coloring[ma * edge_stride + mb] |= bit;
}

struct edge {
  int a;
  int b;
};

static uint32_t _random;

uint32_t xorshift()
{
  uint32_t x = _random;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return _random = x;
}

static inline void render_extension_mesh(ta_parameter_writer& writer,
                                         vec3 ap,
                                         vec3 bp,
                                         vec3 cp,
                                         vec3 dp,
                                         vec3 ep,
                                         vec3 fp,
                                         vec3 apo,
                                         vec3 bpo,
                                         vec3 cpo,
                                         vec3 dpo,
                                         vec3 epo,
                                         vec3 fpo)
{
  render_tri_mod(writer, ap, bp, apo);
  render_tri_mod(writer, bpo, apo, bp);

  render_tri_mod(writer, bp, cp, bpo);
  render_tri_mod(writer, cpo, bpo, cp);

  render_tri_mod(writer, cp, dp, cpo);
  render_tri_mod(writer, dpo, cpo, dp);

  render_tri_mod(writer, dp, ep, dpo);
  render_tri_mod(writer, epo, dpo, ep);

  render_tri_mod(writer, ep, fp, epo);
  render_tri_mod(writer, fpo, epo, fp);

  render_tri_mod(writer, fp, ap, fpo);
  render_last_tri_mod(writer);
  render_tri_mod(writer, apo, fpo, ap);

  /*
  random = 0x12345789;

  render_quad(writer, xorshift(), ap, bp, bpo, apo);

  render_quad(writer, xorshift(), bp, cp, cpo, bpo);

  render_quad(writer, xorshift(), cp, dp, dpo, cpo);

  render_quad(writer, xorshift(), dp, ep, epo, dpo);

  render_quad(writer, xorshift(), ep, fp, fpo, epo);

  render_quad(writer, xorshift(), fp, ap, apo, fpo);
  */
}

void render_silhouette(ta_parameter_writer& writer,
                       const mat4x4& screen,
                       const mat4x4& model,
                       const vec3 light_vec,
                       const uint8_t * edge_coloring,
                       const int edge_stride)
{
  struct edge silhouette[6];
  int ix = 0;

  for (int a = 0; a < edge_stride; a++) {
    for (int b = 0; b < edge_stride; b++) {
      uint8_t coloring = edge_coloring[a * edge_stride + b];
      if (coloring == 0b11) {
        silhouette[ix++] = {a, b};
      }
    }
  }
  assert(ix == 6);

  int last_ix = 0;
  int order_ix = 0;
  int order_vtx[6];
  order_vtx[order_ix++] = silhouette[0].a;

  // calculate vertex ordering
  while (order_ix < 6) {
    for (int i = 1; i < 6; i++) {
      if (i == last_ix)
        continue;

      int last_vtx = order_vtx[order_ix - 1];
      if (last_vtx == silhouette[i].a) {
        last_ix = i;
        order_vtx[order_ix++] = silhouette[i].b;
        break;
      }
      if (last_vtx == silhouette[i].b) {
        last_ix = i;
        order_vtx[order_ix++] = silhouette[i].a;
        break;
      }
    }
  }

  const vec3 * position = cube_position;

  vec3 ap = screen_transform(screen, model * position[order_vtx[0]]);
  vec3 bp = screen_transform(screen, model * position[order_vtx[1]]);
  vec3 cp = screen_transform(screen, model * position[order_vtx[2]]);
  vec3 dp = screen_transform(screen, model * position[order_vtx[3]]);
  vec3 ep = screen_transform(screen, model * position[order_vtx[4]]);
  vec3 fp = screen_transform(screen, model * position[order_vtx[5]]);

  float scale = 5;
  mat4x4 translate = {
    1, 0, 0, -light_vec.x * scale,
    0, 1, 0, -light_vec.y * scale,
    0, 0, 1, -light_vec.z * scale,
    0, 0, 0, 1,
  };

  mat4x4 model2 = model * translate;

  vec3 apo = screen_transform(screen, model2 * position[order_vtx[0]]);
  vec3 bpo = screen_transform(screen, model2 * position[order_vtx[1]]);
  vec3 cpo = screen_transform(screen, model2 * position[order_vtx[2]]);
  vec3 dpo = screen_transform(screen, model2 * position[order_vtx[3]]);
  vec3 epo = screen_transform(screen, model2 * position[order_vtx[4]]);
  vec3 fpo = screen_transform(screen, model2 * position[order_vtx[5]]);

  if (0) { // perimeter
    uint32_t base_color = 0xff0080;

    transfer_line(writer, ap, bp, base_color);
    transfer_line(writer, bp, cp, base_color);
    transfer_line(writer, cp, dp, base_color);
    transfer_line(writer, dp, ep, base_color);
    transfer_line(writer, ep, fp, base_color);
    transfer_line(writer, fp, ap, base_color);
  }

  if (1) { // near end cap
    render_tri_mod(writer, ap, bp, cp);
    render_tri_mod(writer, cp, dp, ep);
    render_tri_mod(writer, ep, fp, ap);
    render_tri_mod(writer, ap, cp, ep);
  }

  if (1) { // far end cap
    render_tri_mod(writer, apo, bpo, cpo);
    render_tri_mod(writer, cpo, dpo, epo);
    render_tri_mod(writer, epo, fpo, apo);
    render_tri_mod(writer, apo, cpo, epo);
  }

  if (1) {
    render_extension_mesh(writer,
                          ap,
                          bp,
                          cp,
                          dp,
                          ep,
                          fp,
                          apo,
                          bpo,
                          cpo,
                          dpo,
                          epo,
                          fpo);
  }
}

void render_cube(ta_parameter_writer& writer,
                 const mat4x4& screen,
                 const vec3 light_vec,
                 float theta)
{
  //float ct = cos(theta);
  //float st = sin(theta);
  float scale = 0.3f;
  const mat4x4 s = {
    scale, 0, 0, 0,
    0, scale, 0, 0,
    0, 0, scale, 0,
    0, 0, 0, 1,
  };
  /*
  const mat4x4 rz = {
    ct, -st, 0, 0,
    st,  ct, 0, 0,
     0,   0, 1, 0,
     0,   0, 0, 1,
  };
  */
  mat4x4 model = s;

  const vec3 * normal = cube_normal;
  const vec3 * position = cube_position;
  const union quadrilateral * quadrilateral = cube_Cube_quadrilateral;

  const int edge_stride = 8;
  const int edge_coloring_length = edge_stride * edge_stride;
  uint8_t __attribute__((aligned(4))) edge_coloring[edge_coloring_length];
  for (int i = 0; i < edge_coloring_length / 4; i++)
    reinterpret_cast<uint32_t *>(edge_coloring)[i] = 0;

  //uint32_t base_color = l_dot_n_b ? 0xff8000 : 0x0080ff;
  const float red = 0.0f;
  const float green = 0.5f;
  const float blue = 1.0f;

  global_polygon_type_1(writer, false, red, green, blue); // no self-shadow

  for (int i = 0; i < 6; i++) {
    const union quadrilateral& q = quadrilateral[i];
    vec3 n3 = normal[q.a.normal];
    vec4 n4 = model * (vec4){n3.x, n3.y, n3.z, 0.f}; // no translation component
    vec3 n = {n4.x, n4.y, n4.z};
    float n_dot_l = dot(n, light_vec);
    bool n_dot_l_b = n_dot_l > 0;

    set_edge_coloring(edge_coloring, edge_stride, n_dot_l_b, q.a.position, q.b.position);
    set_edge_coloring(edge_coloring, edge_stride, n_dot_l_b, q.b.position, q.c.position);
    set_edge_coloring(edge_coloring, edge_stride, n_dot_l_b, q.c.position, q.d.position);
    set_edge_coloring(edge_coloring, edge_stride, n_dot_l_b, q.d.position, q.a.position);

    vec3 ap = model * position[q.a.position];
    vec3 bp = model * position[q.b.position];
    vec3 cp = model * position[q.c.position];
    vec3 dp = model * position[q.d.position];

    vec3 sap = screen_transform(screen, ap);
    vec3 sbp = screen_transform(screen, bp);
    vec3 scp = screen_transform(screen, cp);
    vec3 sdp = screen_transform(screen, dp);

    float intensity = 0.2f;

    if (n_dot_l > 0) {
      intensity += 0.5f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
      if (intensity > 1.0f)
        intensity = 1.0f;
    }

    render_quad_type2(writer,
                      intensity,
                      sap,
                      sbp,
                      scp,
                      sdp);
  }

  if (0) {
    // end of opaque list
    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    global_modifier_volume(writer);

    render_silhouette(writer,
                      screen,
                      model,
                      light_vec,
                      edge_coloring,
                      edge_stride);
  }
}

void render_plane(ta_parameter_writer& writer,
                  const mat4x4& screen,
                  const vec3 light_vec)
{
  const vec3 * normal = plane_normal;
  const vec3 * position = plane_position;
  const union quadrilateral * quadrilateral = plane_Plane.quadrilateral;
  int count = plane_Plane.quadrilateral_count;

  float scale = 3;
  float translate = 1;
  const mat4x4 model = {
    scale, 0, 0, 0,
    0, scale, 0, 0,
    0, 0, scale, translate,
    0, 0, 0, 1,
  };

  //uint32_t base_color = 0xffff80;
  const float red = 1.0f;
  const float green = 1.0f;
  const float blue = 0.5f;

  global_polygon_type_1(writer, true, red, green, blue); // with shadow

  for (int i = 0; i < count; i++) {
    const union quadrilateral& q = quadrilateral[i];

    vec3 ap = model * position[q.a.position];
    vec3 bp = model * position[q.b.position];
    vec3 cp = model * position[q.c.position];
    vec3 dp = model * position[q.d.position];

    float intensity = 0.2f;

    vec4 _n = normal[q.a.normal];
    vec4 n4 = model * (vec4){_n.x, _n.y, _n.z, 0}; // no translation component
    vec3 n = {n4.x, n4.y, n4.z};
    float n_dot_l = -dot(n, light_vec);

    if (n_dot_l > 0) {
      intensity += 0.5f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
      if (intensity > 1.0f)
        intensity = 1.0f;
    }

    vec3 sap = screen_transform(screen, ap);
    vec3 sbp = screen_transform(screen, bp);
    vec3 scp = screen_transform(screen, cp);
    vec3 sdp = screen_transform(screen, dp);

    render_quad_type2(writer,
                      intensity,
                      sap,
                      sbp,
                      scp,
                      sdp);
  }
}

constexpr inline mat4x4 screen_rotation(float theta)
{
  float zt = -0.7853981633974483 / 1 + theta / 5;
  float xt = -0.7853981633974483 * 0.7  + 0.3 * sin(theta / 3);

  mat4x4 rx = {
    1, 0, 0, 0,
    0, cos(xt), -sin(xt), 0,
    0, sin(xt), cos(xt), 0,
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
    0, 1, 0, 0,
    0, 0, 1, 2.5,
    0, 0, 0, 1,
  };

  return t * rx * rz;
}

void render_light_vec(ta_parameter_writer& writer, const mat4x4& screen, vec3 l)
{
  vec3 a = screen_transform(screen, {0, 0, 0});
  vec3 b = screen_transform(screen, l * 0.5f);

  transfer_line(writer, a, b, 0x00ff00);
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen, vec3 light_vec)
{
  // opaque
  render_basis(writer, screen);

  render_light_vec(writer, screen, light_vec);

  if (1) {
    render_plane(writer,
                 screen,
                 light_vec);
  }

  static float cube_theta = 0;
  render_cube(writer,
              screen,
              light_vec,
              cube_theta);
  cube_theta += deg;

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  // punch_through
  /*
  render_basis_text(writer, screen);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  */
}

vec3 update_light()
{
  static float ltheta = 2;

  vec3 light_origin = {0, 0, 0};
  vec3 light_pos = {1, 1, 2};

  mat3x3 rot = {
    cos(ltheta), -sin(ltheta), 0,
    sin(ltheta), cos(ltheta), 0,
    0, 0, 1,
  };

  light_pos = rot * light_pos;
  ltheta += deg / 8;

  vec3 light_vec = light_origin - light_pos;

  return light_vec;
}

static inline uint16_t argb1555(int a, int r, int g, int b)
{
  return ((a & 1) << 15) | ((r & 31) << 10) | ((g & 31) << 5) | ((b & 31) << 0);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void region_array_multipass2(const uint32_t width,  // in tile units (1 tile unit = 32 pixels)
                             const uint32_t height, // in tile units (1 tile unit = 32 pixels)
                             const struct opb_size * opb_size,
                             const uint32_t num_render_passes,
                             const uint32_t region_array_start,
                             const uint32_t object_list_start,
                             uint32_t * dest)
{
  auto region_array = reinterpret_cast<region_array_entry *>
    (&dest[region_array_start / 4]);

  const uint32_t num_tiles = width * height;
  uint32_t ol_base[num_render_passes];

  ol_base[0] = object_list_start;
  for (uint32_t pass = 1; pass < num_render_passes; pass++) {
    ol_base[pass] = ol_base[pass - 1] + num_tiles * opb_size[pass - 1].total();
  }

  uint32_t ix = 0;

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      for (uint32_t pass = 0; pass < num_render_passes; pass++) {
        region_array[ix].tile = REGION_ARRAY__TILE_Y_POSITION(y)
                              | REGION_ARRAY__TILE_X_POSITION(x);

        if (pass == (num_render_passes - 1) && y == (height - 1) && x == (width - 1))
          region_array[ix].tile |= REGION_ARRAY__LAST_REGION;

        if (pass != (num_render_passes - 1))
          region_array[ix].tile |= REGION_ARRAY__FLUSH_ACCUMULATE;

        if (pass > 0)
          region_array[ix].tile |= REGION_ARRAY__Z_CLEAR;

        uint32_t tile_index = y * width + x;
        region_array[ix].opaque_list_pointer                      = (opb_size[pass].opaque               == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + (opb_size[pass].opaque * tile_index)
                                                                     );

        region_array[ix].opaque_modifier_volume_list_pointer      = (opb_size[pass].opaque_modifier      == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 )
                                                                                   + (opb_size[pass].opaque_modifier * tile_index)
                                                                     );

        region_array[ix].translucent_list_pointer                 = (opb_size[pass].translucent          == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 )
                                                                                   + (opb_size[pass].translucent * tile_index)
                                                                     );
        region_array[ix].translucent_modifier_volume_list_pointer = (opb_size[pass].translucent_modifier == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 + opb_size[pass].translucent
                                                                                                 )
                                                                                   + (opb_size[pass].translucent_modifier * tile_index)
                                                                     );
        region_array[ix].punch_through_list_pointer               = (opb_size[pass].punch_through        == 0) ? REGION_ARRAY__LIST_POINTER__EMPTY :
                                                                    (ol_base[pass] + num_tiles * ( opb_size[pass].opaque
                                                                                                 + opb_size[pass].opaque_modifier
                                                                                                 + opb_size[pass].translucent
                                                                                                 + opb_size[pass].translucent_modifier
                                                                                                 )
                                                                                   + (opb_size[pass].punch_through * tile_index)
                                                                     );
        fprintf(stderr, "ra_ol %d %d %08x\n", x, y, region_array[ix].opaque_list_pointer);
        ix += 1;
      }
    }
  }
}

struct vertex_parameter {
  float x;
  float y;
  float z;
  uint32_t base_color;
}; // ISP_BACKGND_T skip(1)

struct isp_tsp_parameter {
  uint32_t isp_tsp_instruction_word;
  uint32_t tsp_instruction_word;
  uint32_t texture_control_word;
  vertex_parameter vertex[3];
};

void background_parameter3(const uint32_t background_start,
			   const uint32_t color,
                           uint32_t * dst)
{
  auto parameter = reinterpret_cast<isp_tsp_parameter *>
    (&dst[background_start / 4]);

  parameter->isp_tsp_instruction_word
    = isp_tsp_instruction_word::depth_compare_mode::always
    | isp_tsp_instruction_word::culling_mode::no_culling;

  parameter->tsp_instruction_word
    = tsp_instruction_word::src_alpha_instr::one
    | tsp_instruction_word::dst_alpha_instr::zero
    | tsp_instruction_word::fog_control::no_fog;

  parameter->texture_control_word
    = 0;

  parameter->vertex[0].x = 0.f;
  parameter->vertex[0].y = 0.f;
  parameter->vertex[0].z = 1.f/100000;
  parameter->vertex[0].base_color = color;

  parameter->vertex[1].x = 639.f;
  parameter->vertex[1].y = 0.f;
  parameter->vertex[1].z = 1.f/100000;
  parameter->vertex[1].base_color = color;

  parameter->vertex[2].x = 639.f;
  parameter->vertex[2].y = 479.f;
  parameter->vertex[2].z = 1.f/100000;
  parameter->vertex[2].base_color = color;
}

static uint8_t __attribute__((aligned(32))) texture_memory[8 * 1024 * 1024];

int main()
{
  constexpr uint32_t ta_alloc = 0
    //ta_alloc_ctrl::pt_opb::_16x4byte
			      | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
    //| ta_alloc_ctrl::om_opb::_16x4byte
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int render_passes = 1;
  constexpr struct opb_size opb_size[render_passes] = {
    {
      .opaque = 16 * 4,
      //.opaque_modifier = 16 * 4,
      .translucent = 0,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  for (int i = 0; i < 2; i++) {
    region_array_multipass2(tile_width,
                            tile_height,
                            opb_size,
                            render_passes,
                            texture_memory_alloc.region_array[i].start,
                            texture_memory_alloc.object_list[i].start,
                            (uint32_t*)texture_memory);

    background_parameter3(texture_memory_alloc.background[i].start,
                          0xff202040,
                          (uint32_t*)texture_memory);
  }

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  int ta = 0;
  int core = 0;

  const float degree = 0.017453292519943295 / 5;
  float theta = 0;

  const mat4x4 screen = screen_rotation(theta);

  vec3 light_vec = update_light();

  writer.offset = 0;
  transfer_scene(writer, screen, light_vec);

  //size_t len = fwrite(writer.buf, 1, writer.offset, stdout);

  struct ta_configuration config;

  config.isp_base = texture_memory_alloc.isp_tsp_parameters[ta].start;
  config.isp_limit = texture_memory_alloc.isp_tsp_parameters[ta].end;
  config.ol_base = texture_memory_alloc.object_list[ta].start;
  config.ol_limit = texture_memory_alloc.object_list[ta].end;
  config.alloc_ctrl = ta_alloc;
  config.next_opb_init = 0;
  config.tile_x_num = tile_width;
  config.tile_y_num = tile_height;

  software_ta_init(&config);
  software_ta_transfer(writer.buf, writer.offset, texture_memory);

  size_t len = fwrite(texture_memory, 1, (sizeof (texture_memory)), stdout);
  assert(len == (sizeof (texture_memory)));
}
