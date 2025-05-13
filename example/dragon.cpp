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
#include "math/mat4x4.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using mat4x4 = mat<4, 4, float>;

#include "md2/md2.h"
#include "md2/md2_normals.h"
#include "model/dragon/model.c"
#include "model/dragon/dragon.data.h"
#include "model/dragon/dragon.data.pal.h"
#include "model/dragon/chrome.data.h"
#include "model/dragon/inverse_cube.h"

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
  serial::string("vbr400");
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

void global_polygon_type_0(ta_parameter_writer& writer, uint32_t texture_address, uint32_t list, uint32_t cull)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::texture_u_size::from_int(256)
                                      | tsp_instruction_word::texture_v_size::from_int(256)
                                      ;

  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
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
}

void global_polygon_type_0b(ta_parameter_writer& writer, uint32_t texture_address, uint32_t cull)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::src_alpha
                                      | tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
                                      | tsp_instruction_word::texture_u_size::from_int(64)
                                      | tsp_instruction_word::texture_v_size::from_int(64)
                                      | tsp_instruction_word::use_alpha
    | tsp_instruction_word::filter_mode::bilinear_filter
    | tsp_instruction_word::texture_shading_instruction::decal_alpha;

  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
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
}

vec3 screen_transform(const mat4x4& screen, vec3 v)
{
  v = screen * v;

  float dim = 480 / 2.0 * 8;

  return {
    v.x / v.z * dim + 640 / 2.0f,
    v.y / v.z * dim + 480 / 2.0f,
    1 / v.z,
  };
}

static inline void render_tri(ta_parameter_writer& writer,
                              uint32_t base_color,
                              vec3 ap,
                              vec3 bp,
                              vec3 cp,
                              vec2 at,
                              vec2 bt,
                              vec2 ct)
{
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        base_color,
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        base_color,
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        base_color,
                                        0); // offset_color
}

static inline void render_quad(ta_parameter_writer& writer,
                               uint32_t base_color,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               vec2 at,
                               vec2 bt,
                               vec2 ct,
                               vec2 dt)
{
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        base_color,
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        base_color,
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        base_color,
                                        0); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        base_color,
                                        0); // offset_color
}


static inline vec3 frame_vertex_to_vec3(struct md2_frame * frame, int index)
{
  return {
    ((float)frame->verts[index].v[0]) * frame->scale.x + frame->translate.x,
    ((float)frame->verts[index].v[1]) * frame->scale.y + frame->translate.y,
    ((float)frame->verts[index].v[2]) * frame->scale.z + frame->translate.z,
  };
}

static inline vec2 texture_coordinate_to_vec2(struct md2_texture_coordinate * st, float div, int index)
{
  return {
    ((float)st[index].s) * div,
    ((float)st[index].t) * div,
  };
}
constexpr inline mat4x4 screen_rotation(float theta)
{
  //float zt = -0.7853981633974483 + (0.2);
  float zt = -0.7853981633974483 + (0.2) + theta * 0.5;
  float xt = 0.7853981633974483 * 2.5;
  //float xt = 0.7853981633974483 * 3.7;

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
    0, 1, 0, -90,
    0, 0, 1, 180,
    0, 0, 0, 1,
  };

  return t * rx * rz;
}

static int animation_tick = 0;
constexpr int animation_frames = 40;
constexpr int ticks_per_animation_frame = 16;
constexpr float tick_div = 1.0f / (float)ticks_per_animation_frame;

void render_dragon(ta_parameter_writer& writer, const mat4x4& model, const mat4x4& screen)
{
  int frame_ix0 = animation_tick / ticks_per_animation_frame;
  int frame_ix1 = frame_ix0 + 1;
  if (frame_ix1 >= animation_frames)
    frame_ix1 = 0;

  struct md2_frame * frame0 = &dragon_header.frames[frame_ix0];
  struct md2_frame * frame1 = &dragon_header.frames[frame_ix1];

  float st_div = 1.0f / 256.0f;

  uint32_t base_color = 0xff000000;
  for (int i = 0; i < dragon_header.num_tris; i++) {
    struct md2_triangle * t = &dragon_header.tris[i];

    vec3 a0 = frame_vertex_to_vec3(frame0, t->vertex[0]);
    vec3 b0 = frame_vertex_to_vec3(frame0, t->vertex[1]);
    vec3 c0 = frame_vertex_to_vec3(frame0, t->vertex[2]);

    vec3 a1 = frame_vertex_to_vec3(frame1, t->vertex[0]);
    vec3 b1 = frame_vertex_to_vec3(frame1, t->vertex[1]);
    vec3 c1 = frame_vertex_to_vec3(frame1, t->vertex[2]);

    float lerp = (float)(animation_tick - (frame_ix0 * ticks_per_animation_frame)) * tick_div;

    vec3 a = model * (a0 + ((a1 - a0) * lerp));
    vec3 b = model * (b0 + ((b1 - b0) * lerp));
    vec3 c = model * (c0 + ((c1 - c0) * lerp));

    vec2 at = texture_coordinate_to_vec2(dragon_header.st, st_div, t->st[0]);
    vec2 bt = texture_coordinate_to_vec2(dragon_header.st, st_div, t->st[1]);
    vec2 ct = texture_coordinate_to_vec2(dragon_header.st, st_div, t->st[2]);

    render_tri(writer,
               base_color,
               screen_transform(screen, a),
               screen_transform(screen, b),
               screen_transform(screen, c),
               at,
               bt,
               ct);
  }
}

void render_cube(ta_parameter_writer& writer, const mat4x4& screen)
{
  float s = 100.f;
  float x = -10;
  float y = -100;
  float z = 0.0f;
  const mat4x4 translate = {
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1,
  };
  const mat4x4 scale = {
    s, 0, 0, 0,
    0, s, 0, 0,
    0, 0, s, 0,
    0, 0, 0, 1,
  };
  const mat4x4 model = translate * scale;

  float x1 = -10;
  float y1 = -100;
  float z1 = -200.0f;
  const mat4x4 translate1 = {
    1, 0, 0, x1,
    0, 1, 0, y1,
    0, 0, 1, z1,
    0, 0, 0, 1,
  };
  const mat4x4 model1 = translate1 * scale;

  const struct object * object = inverse_cube_model.object[0];
  const uint32_t base_color = 0x40ffffff;

  global_polygon_type_0b(writer,
                         texture_memory_alloc.texture.start + 131072,
                         isp_tsp_instruction_word::culling_mode::cull_if_positive);

  for (int i = 0; i < object->quadrilateral_count; i++) {
    const union quadrilateral * quad = &object->quadrilateral[i];
    vec3 a = inverse_cube_model.position[quad->a.position];
    vec3 b = inverse_cube_model.position[quad->b.position];
    vec3 c = inverse_cube_model.position[quad->c.position];
    vec3 d = inverse_cube_model.position[quad->d.position];

    vec3 ap = model * a;
    vec3 bp = model * b;
    vec3 cp = model * c;
    vec3 dp = model * d;

    vec2 at = inverse_cube_model.texture[quad->a.texture];
    vec2 bt = inverse_cube_model.texture[quad->b.texture];
    vec2 ct = inverse_cube_model.texture[quad->c.texture];
    vec2 dt = inverse_cube_model.texture[quad->d.texture];

    render_quad(writer,
                base_color,
                screen_transform(screen, ap),
                screen_transform(screen, bp),
                screen_transform(screen, cp),
                screen_transform(screen, dp),
                at * 3.0f,
                bt * 3.0f,
                ct * 3.0f,
                dt * 3.0f);

    render_quad(writer,
                base_color,
                screen_transform(screen, model1 * a),
                screen_transform(screen, model1 * b),
                screen_transform(screen, model1 * c),
                screen_transform(screen, model1 * d),
                at * 3.0f,
                bt * 3.0f,
                ct * 3.0f,
                dt * 3.0f);
  }
}

void transfer_scene(ta_parameter_writer& writer)
{
  const float deg = 0.017453292519943295 / 4;
  static float theta = deg;
  const mat4x4 screen = screen_rotation(theta);

  theta += deg;

  // opaque
  if (1) {
    global_polygon_type_0(writer, texture_memory_alloc.texture.start,
                          para_control::list_type::opaque,
                          isp_tsp_instruction_word::culling_mode::cull_if_negative);
    float scale = 0.3f;
    float translate = -92.8f;
    const mat4x4 model = {
      scale, 0, 0, 0,
      0, scale, 0, 0,
      0, 0, scale, translate,
      0, 0, 0, 1,
    };
    render_dragon(writer, model, screen);
  }

  {
    global_polygon_type_0(writer, texture_memory_alloc.texture.start,
                          para_control::list_type::opaque,
                          isp_tsp_instruction_word::culling_mode::cull_if_positive);
    float scale = 0.3f;
    float translate = -107.35f;
    const mat4x4 model = {
      scale, 0, 0, 0,
      0, scale, 0, 0,
      0, 0, -scale, translate,
      0, 0, 0, 1,
    };
    render_dragon(writer, model, screen);
  }

  // end of opaque list
  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  render_cube(writer, screen);

  // end of translucent list
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
}

void transfer_palette()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  uint16_t * src = reinterpret_cast<uint16_t *>(&_binary_model_dragon_dragon_data_pal_start);
  uint32_t size = reinterpret_cast<uint32_t>(&_binary_model_dragon_dragon_data_pal_size);

  for (uint32_t i = 0; i < size / 2; i++) {
    holly.PALETTE_RAM[i] = src[i];
  }
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);

  interrupt_init();

  constexpr uint32_t ta_alloc = 0
                              | ta_alloc_ctrl::pt_opb::no_list
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

  animation_tick = 0x000004ff;

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
    //serial::string("wait opaque_list");
    ta_wait_translucent_list();

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

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;
    while (spg_status::vsync(holly.SPG_STATUS));
  }
}
