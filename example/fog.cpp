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

vec3 square[] = {
  {0 + 50, 0 + 50, 0.00392156862745098},
  {640 - 50, 0 + 50, 0.9725490196078431},
  {640 - 50, 480 - 50, 0.9725490196078431},
  {0 + 50, 480 - 50, 0.00392156862745098},
};

vec2 texcoord[] = {
  {0, 0},
  {1, 0},
  {1, 1},
  {0, 1},
};

uint32_t offset[] = {
  0,
  0xffUL << 24,
  0xffUL << 24,
  0
};

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t list,
                           uint32_t cull)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | list
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud
                                        | obj_control::offset
                                        | obj_control::texture
                                        ;
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | cull;

  const uint32_t tsp_instruction_word = 0
    //| tsp_instruction_word::fog_control::per_vertex
    | tsp_instruction_word::fog_control::look_up_table
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::texture_shading_instruction::modulate
                                      | tsp_instruction_word::texture_u_size::from_int(8)
                                      | tsp_instruction_word::texture_v_size::from_int(8)
                                      ;

  const uint32_t texture_address = texture_memory_alloc.texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data size for sort dma
                                        0  // next address for sort dma
                                        );
}

static inline void render_quad_type_3(ta_parameter_writer& writer,
                                      vec3 ap,
                                      vec3 bp,
                                      vec3 cp,
                                      vec3 dp,
                                      vec2 at,
                                      vec2 bt,
                                      vec2 ct,
                                      vec2 dt,
                                      uint32_t base_color,
                                      uint32_t aoc,
                                      uint32_t boc,
                                      uint32_t coc,
                                      uint32_t doc)
{
  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        base_color,
                                        aoc);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        base_color,
                                        boc);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        base_color,
                                        doc);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        base_color,
                                        coc);
}

void transfer_scene(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer,
                        para_control::list_type::opaque,
                        isp_tsp_instruction_word::culling_mode::no_culling);

  render_quad_type_3(writer,
                     square[0],
                     square[1],
                     square[2],
                     square[3],
                     texcoord[0],
                     texcoord[1],
                     texcoord[2],
                     texcoord[3],
                     0xff00ff00,
                     offset[0],
                     offset[1],
                     offset[2],
                     offset[3]);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void fog_init()
{
  // for look-up table mode
  // 2 ** 0 * 128 = 1.0
  /*
  holly.FOG_DENSITY = 0
    | fog_density::fog_scale_mantissa(128)
    | fog_density::fog_scale_exponent(0)
    ;
  */
  holly.FOG_DENSITY = 0
    | fog_density::fog_scale_mantissa(255)
    | fog_density::fog_scale_exponent(7)
    ;

  holly.FOG_CLAMP_MIN = 0x0;
  holly.FOG_CLAMP_MAX = 0xffffffff;
  holly.FOG_COL_RAM = 0xff0000; // for look-up table mode
  holly.FOG_COL_VERT = 0xff0000; // red

  for (int i = 0; i < 128; i++) {
    holly.FOG_TABLE[i] = ((i * 2) << 8) | ((i + 1) * 2);
  }
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
  fog_init();
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
