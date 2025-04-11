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

struct boid
{
  vec3 position;
  vec3 velocity;
};

struct boid boids[50];
const int boids_length = (sizeof (boids)) / (sizeof (boids[0]));

void boid_initialize_positions()
{
  for (int i = 0; i < boids_length; i++) {
    boids[i].velocity = {0, 0, 0};
  }

  boids[0].position = {240, 240, 0.1};
  boids[1].position = {-20, 640, 0.1};
  boids[2].position = {100, -20, 0.1};
  boids[3].position = {-20, -20, 0.1};
  boids[4].position = {900, 600, 0.1};

  for (int i = 5; i < boids_length; i ++ ){
    boids[i].position = {0, 0, 0.1};
  }
}

vec3 rule1(struct boid * boid)
{
  vec3 center = {0, 0, 0};

  for (int i = 0; i < boids_length; i++) {
    if (&boids[i] == boid)
      continue;

    center = center + boids[i].position;
  }

  center = center / (float)(boids_length - 1);

  return (center - boid->position) / 100.f;
}

vec3 rule2(struct boid * boid)
{
  vec3 c = {0, 0, 0};
  for (int i = 0; i < boids_length; i++) {
    if (&boids[i] == boid)
      continue;
    if (magnitude(boids[i].position - boid->position) < 20.f) {
      c = c - (boids[i].position - boid->position);
    }
  }
  return c;
}

vec3 rule3(struct boid * boid)
{
  vec3 pv = {0, 0, 0};

  for (int i = 0; i < boids_length; i++) {
    if (&boids[i] == boid)
      continue;

    pv = pv + boids[i].velocity;
  }

  pv = pv / (float)(boids_length - 1);

  return (pv - boid->velocity) / 10.0f;
}

vec3 bound_position(struct boid * b)
{
  float xmin = 0;
  float xmax = 640;
  float ymin = 0;
  float ymax = 480;
  float zmin = 0.1;
  float zmax = 0.2;

  vec3 v;

  if (b->position.x < xmin)
    v.x = 5;
  if (b->position.x > xmax)
    v.x = -5;
  if (b->position.y < ymin)
    v.y = 5;
  if (b->position.y > ymax)
    v.y = -5;
  /*
  if (b->position.z < zmin)
    v.z = 0.01;
  if (b->position.z > zmax)
    v.z = -0.01;
  */

  return v;
}

void limit_velocity(struct boid * boid)
{
  float limit = 8;
  vec3 v;

  if (magnitude(boid->velocity) > limit) {
    boid->velocity = (boid->velocity / magnitude(boid->velocity)) * limit;
  }
}

void update_boids()
{
  for (int i = 0; i < boids_length; i++) {
    struct boid * b = &boids[i];
    vec3 v1 = rule1(b);
    vec3 v2 = rule2(b);
    vec3 v3 = rule3(b);
    vec3 v4 = bound_position(b);

    b->velocity = b->velocity + v1 + v2 + v3 + v4;

    limit_velocity(b);
    b->velocity.z = 0;

    b->position = b->position + b->velocity;
  }
}

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t list,
                           uint32_t cull)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud
                                        ;
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = 0
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      ;

  const uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data size for sort dma
                                        0  // next address for sort dma
                                        );
}

static inline void render_tri_type_0(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     uint32_t ac,
                                     uint32_t bc,
                                     uint32_t cc)
{
  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        ac);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bc);

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        cc);
}

void dvec(const char * s, vec3 v)
{
  serial::string(s);
  serial::character(' ');
  serial::integer<uint32_t>(v.x, ' ');
  serial::integer<uint32_t>(v.y, ' ');
  serial::integer<uint32_t>(v.z);
}

void render_boid(ta_parameter_writer& writer, struct boid * boid)
{
  const float size = 15.f;

  float mag = magnitude(boid->velocity);
  vec3 normal;
  if (mag == 0)
    normal = {1, 0, 0};
  else
    normal = boid->velocity / mag;

  vec3 perp = {-normal.y, normal.x, normal.z};

  vec3 a = boid->position + normal * size;
  vec3 b = boid->position - normal * size * 0.5f + perp * size * 0.5f;
  vec3 c = boid->position - normal * size * 0.5f - perp * size * 0.5f;

  /*
  dvec("n", normal);
  dvec("a", a);
  dvec("b", b);
  dvec("c", c);
  dvec("p", boid->position);
  */

  render_tri_type_0(writer, a, b, c, 0xff00ff00, 0xff808000, 0xff808000);
}

void transfer_scene(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer,
                        para_control::list_type::opaque,
                        isp_tsp_instruction_word::culling_mode::no_culling);

  for (int i = 0; i < boids_length; i++) {
    render_boid(writer, &boids[i]);
  }

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void texture_init()
{
  volatile uint32_t * base = &texture_memory64[texture_memory_alloc.texture.start / 4];
  for (int i = 0; i < 8 * 8 / 2; i++) {
    base[i] = 0xffffffff;
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
  texture_init();

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

  video_output::set_mode_vga();
  boid_initialize_positions();

  while (1) {
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);
    update_boids();
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
