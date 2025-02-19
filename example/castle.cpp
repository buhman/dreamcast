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

#include "x/x.hpp"
#include "model/castle/model.hpp"

const float deg = 0.017453292519943295;

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

vec3 screen_transform(const mat4x4& screen, vec3 v)
{
  v = screen * v;

  float dim = 480 / 2.0;

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
                              vec3 cp)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        base_color
                                        ); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        base_color
                                        ); // offset_color

  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        base_color
                                        ); // offset_color
}

constexpr inline mat4x4 screen_rotation(float theta)
{
  //float zt = -0.7853981633974483 + (0.2);
  float zt = deg * 0;
  float xt = -(sin(theta * 4) * 0.5 + 0.5);
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
    0, -1, 0, 300,
    0, 0, 1, 500,
    0, 0, 0, 1,
  };

  return t * rx * rz;
}

uint32_t xorshift_state = 0x12345678;

/* The state must be initialized to non-zero */
uint32_t xorshift32()
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  uint32_t x = xorshift_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return xorshift_state = x;
}

const frame * frames[] = {
  &frame_x3ds_Box08,
  &frame_x3ds_Box09,
  &frame_x3ds_Box02,
  &frame_x3ds_Cone02,
  &frame_x3ds_Cylinder02,
  &frame_x3ds_Box21,
  &frame_x3ds_Box22,
  &frame_x3ds_Box23,
  &frame_x3ds_Box24,
  &frame_x3ds_Box25,
  &frame_x3ds_Box26,
  &frame_x3ds_Box27,
  &frame_x3ds_Cylinder03,
  &frame_x3ds_Cone03,
  &frame_x3ds_Cylinder01,
  &frame_x3ds_Box01,
  &frame_x3ds_Box28,
  &frame_x3ds_Box03,
  &frame_x3ds_roof,
  &frame_x3ds_walkway,
};

static inline void render_mesh(ta_parameter_writer& writer,
                               const mat4x4& screen,
                               const mat4x4& model,
                               const mesh * mesh)
{
  //mesh->vertices;
  for (int i = 0; i < mesh->n_faces; i++) {
    const auto& indices = mesh->faces[i].face_vertex_indices;
    uint32_t base_color = xorshift32();
    render_tri(writer,
               base_color,
               screen_transform(screen, model * mesh->vertices[indices[0]]),
               screen_transform(screen, model * mesh->vertices[indices[1]]),
               screen_transform(screen, model * mesh->vertices[indices[2]]));
  }
}

static inline void render_frame(ta_parameter_writer& writer,
                                const mat4x4& screen,
                                const frame * frame)
{
  xorshift_state = 0x12345678;

  const mat4x4 identity{};

  const mat4x4 * transform_matrix = &identity;

  int ix = 0;
  while (true) {
    const data_object * obj = frame->objects[ix];
    if (obj == nullptr)
      break;

    switch (obj->tag) {
    case tag::frame_transform_matrix:
      transform_matrix = &obj->frame_transform_matrix.frame_matrix;
      break;
    case tag::mesh:
      render_mesh(writer, screen, *transform_matrix, &obj->mesh);
      break;
    default:
      break;
    }

    ix += 1;
  }
}

void render_castle(ta_parameter_writer& writer, const mat4x4& screen)
{
  for (uint32_t i = 0; i < (sizeof (frames)) / (sizeof (frames[0])); i++) {
    render_frame(writer, screen, frames[i]);
  }
}

static float theta = deg * 360;
void transfer_scene(ta_parameter_writer& writer)
{
  const mat4x4 screen = screen_rotation(theta);

  global_polygon_type_0(writer, texture_memory_alloc.texture.start,
                        para_control::list_type::opaque,
                        isp_tsp_instruction_word::culling_mode::no_culling);

  render_castle(writer, screen);

  // end of opaque list
  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  theta += deg * 0.1;
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
    //serial::string("wait render_done");
    while (render_done == 0) {
      asm volatile ("nop");
    };

    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;
    while (spg_status::vsync(holly.SPG_STATUS));
  }
}
