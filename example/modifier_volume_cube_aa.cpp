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

#include "model/model.h"
#include "model/cube/model.h"
#include "model/plane/model.h"

#include "font/terminus/ter_u32n.data.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat3x3.hpp"
#include "math/mat4x4.hpp"
#include "math/math.hpp"

#define assert(b)                                                       \
  do {                                                                  \
    if (!(b)) {                                                         \
      serial::string(__FILE__);                                         \
      serial::character(':');                                           \
      serial::integer<uint32_t>(__LINE__, ' ');                         \
      serial::string(__func__);                                         \
      serial::string(": assertion failed: ");                           \
      serial::string(#b);                                               \
      serial::character('\n');                                          \
      while (1);                                                        \
    }                                                                   \
  } while (0);

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat3x3 = mat<3, 3, float>;
using mat4x4 = mat<4, 4, float>;

void vbr100()
{
  serial::string("vbr100\n");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);
  while (1);
}

void vbr400()
{
  serial::string("vbr400");
  serial::string("expevt ");
  serial::integer<uint16_t>(sh7091.CCN.EXPEVT);
  serial::string("intevt ");
  serial::integer<uint16_t>(sh7091.CCN.INTEVT);
  serial::string("tra ");
  serial::integer<uint16_t>(sh7091.CCN.TRA);
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

  serial::string("vbr600");
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


  serial::string("halt\n");
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

const float deg = 0.017453292519943295;

#define _fsrra(n)                              \
  ({                                           \
    float v = (n);                             \
    asm("fsrra %0"                             \
        : "=f" (v)                             \
        : "0" (v));                            \
    v;                                         \
  })

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

void transfer_glyph(ta_parameter_writer& writer, float x, float y, int c)
{
  constexpr int width = 16;
  constexpr int height = 32;

  const uint32_t parameter_control_word = para_control::para_type::sprite
                                        | para_control::list_type::punch_through
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::src_alpha
                                      | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                      | tsp_instruction_word::texture_u_size::from_int(width)
                                      | tsp_instruction_word::texture_v_size::from_int(height);

  const int ix = c - ' ';
  const int offset = (width * height * ix) / 2;
  const uint32_t texture_address = texture_memory_alloc.texture.start + offset;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_4bpp_palette
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  constexpr vec2 v[4] = {
    { 0.f, 0.f },
    { 1.f, 0.f },
    { 1.f, 1.f },
    { 0.f, 1.f },
  };

  constexpr uint32_t base_color = 0;
  writer.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
                                isp_tsp_instruction_word,
                                tsp_instruction_word,
                                texture_control_word,
                                base_color,
                                0,  // offset_color
                                0,  // data_size_for_sort_dma
                                0); // next_address_for_sort_dma

  writer.append<ta_vertex_parameter::sprite_type_1>() =
    ta_vertex_parameter::sprite_type_1(para_control::para_type::vertex_parameter,
				       v[0].x * width  + x,
				       v[0].y * height + y,
				       0.1f,
				       v[1].x * width  + x,
				       v[1].y * height + y,
				       0.1f,
				       v[2].x * width  + x,
				       v[2].y * height + y,
				       0.1f,
				       v[3].x * width  + x,
				       v[3].y * height + y,
                                       uv_16bit(v[0].x, v[0].y),
                                       uv_16bit(v[1].x, v[1].y),
				       uv_16bit(v[2].x, v[2].y));
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

void render_basis_text(ta_parameter_writer& writer, const mat4x4& screen)
{
  vec3 z = screen_transform(screen, (vec3){0, 0, 1} * 1.2f);
  vec3 y = screen_transform(screen, (vec3){0, 1, 0} * 1.2f);
  vec3 x = screen_transform(screen, (vec3){1, 0, 0} * 1.2f);

  transfer_glyph(writer, z.x - 5, z.y - 10, 'z');
  transfer_glyph(writer, y.x - 5, y.y - 10, 'y');
  transfer_glyph(writer, x.x - 5, x.y - 10, 'x');
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

static uint32_t random;

uint32_t xorshift()
{
  uint32_t x = random;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return random = x;
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

  if (1) {
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

  uint32_t offset = texture_memory_alloc.texture.start;
  void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
  void * src = reinterpret_cast<void *>(&_binary_font_terminus_ter_u32n_data_start);
  uint32_t size = reinterpret_cast<uint32_t>(&_binary_font_terminus_ter_u32n_data_size);
  transfer_ta_fifo_texture_memory_32byte(dst, src, size);
}

static inline uint16_t argb1555(int a, int r, int g, int b)
{
  return ((a & 1) << 15) | ((r & 31) << 10) | ((g & 31) << 5) | ((b & 31) << 0);
}

void transfer_palette()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb1555;

  holly.PALETTE_RAM[  0 + 0] = argb1555(0,  0,  0,  0);
  holly.PALETTE_RAM[  0 + 1] = argb1555(1, 31, 31, 31);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);

  interrupt_init();
  //asm volatile ("trapa #0");

  transfer_textures();
  transfer_palette();

  constexpr uint32_t ta_alloc = 0
    //ta_alloc_ctrl::pt_opb::_16x4byte
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
      //.punch_through = 16 * 4
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  system.IML6NRM = istnrm::end_of_render_tsp;

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::intensity_volume_mode
                       | fpu_shad_scale::scale_factor_for_shadows(128);
  video_output::set_mode_vga();

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

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  int ta = 0;
  int core = 0;

  const float degree = 0.017453292519943295 / 5;
  float theta = 0;

  while (1) {
    const mat4x4 screen = screen_rotation(theta);

    vec3 light_vec = update_light();

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    writer.offset = 0;
    transfer_scene(writer, screen, light_vec);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_modifier_volume_list();

    render_done = 0;
    core_start_render2(texture_memory_alloc.region_array[core].start,
                       texture_memory_alloc.isp_tsp_parameters[core].start,
                       texture_memory_alloc.background[core].start,
                       texture_memory_alloc.framebuffer[core].start,
                       framebuffer_width);
    //serial::string("wait render_done");
    while (render_done == 0) {
      asm volatile ("nop");
    };

    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;

    theta += degree;
  }
  serial::string("return\nreturn\nreturn\nreturn\n");
}
