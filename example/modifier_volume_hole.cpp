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
#include "math/transform.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#include "interrupt.hpp"
#include "assert.h"

#include "model/blender_export.h"
//#include "model/modifier_volume_test/closed_volume.h"
#include "model/modifier_volume_test/open_volume.h"

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
                            | ta_alloc_ctrl::pt_opb::no_list
                            | ta_alloc_ctrl::tm_opb::no_list
                            | ta_alloc_ctrl::t_opb::no_list
                            | ta_alloc_ctrl::om_opb::_32x4byte
                            | ta_alloc_ctrl::o_opb::_32x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 32 * 4,
    .translucent = 0,
    .translucent_modifier = 0,
    .punch_through = 0
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

  if (istnrm & istnrm::end_of_transferring_opaque_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

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

static inline void global_polygon(ta_parameter_writer& writer,
                                  uint32_t list,
                                  float mod)
{
   uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                   | list
                                   | obj_control::col_type::intensity_mode_1
                                   | obj_control::gouraud
                                   | obj_control::shadow
                                   ;

   uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                     | isp_tsp_instruction_word::culling_mode::no_culling;

   uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                 | tsp_instruction_word::dst_alpha_instr::zero
                                 | tsp_instruction_word::texture_shading_instruction::decal
                                 | tsp_instruction_word::fog_control::no_fog
                                 ;

   uint32_t texture_control_word = 0;

   float a = 1.0f * mod;
   float r = 1.0f * mod;
   float g = 1.0f * mod;
   float b = 1.0f * mod;

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

static inline void global_polygon_modifier_volume(ta_parameter_writer& writer)
{
   uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                   | para_control::list_type::opaque_modifier_volume
                                   ;

   uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                     | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(parameter_control_word,
                                         isp_tsp_instruction_word);
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

static inline void global_last_in_volume(ta_parameter_writer& writer)
{
  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                             | para_control::list_type::opaque_modifier_volume
                                             | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                               | isp_tsp_instruction_word::culling_mode::no_culling;

  writer.append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
                                         last_isp_tsp_instruction_word);
}

static inline void render_quad_modifier_volume(ta_parameter_writer& writer,
                                               vec3 ap,
                                               vec3 bp,
                                               vec3 cp,
                                               vec3 dp,
                                               bool last_tri)
{
  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         ap.x, ap.y, ap.z,
                                         bp.x, bp.y, bp.z,
                                         dp.x, dp.y, dp.z);

  if (last_tri) {
    global_last_in_volume(writer);
  }

  writer.append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         bp.x, bp.y, bp.z,
                                         dp.x, dp.y, dp.z,
                                         cp.x, cp.y, cp.z);
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

static inline void render_polygon(ta_parameter_writer& writer,
                                  const polygon * polygon,
                                  const vec3 * position)
{
  vec3 ap = position[polygon->a];
  vec3 bp = position[polygon->b];
  vec3 cp = position[polygon->c];
  vec3 dp = position[polygon->d];

  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0) {
    return;
  }

  float ai = 1.0;
  float bi = 1.0;
  float ci = 1.0;
  float di = 1.0;

  render_quad(writer,
              screen_transform(ap),
              screen_transform(bp),
              screen_transform(cp),
              screen_transform(dp),
              ai, bi, ci, di);
}

static inline void render_polygon_modifier_volume(ta_parameter_writer& writer,
                                                  const polygon * polygon,
                                                  const vec3 * position,
                                                  bool last)
{
  vec3 ap = position[polygon->a];
  vec3 bp = position[polygon->b];
  vec3 cp = position[polygon->c];
  vec3 dp = position[polygon->d];

  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0) {
    return;
  }

  render_quad_modifier_volume(writer,
                              screen_transform(ap),
                              screen_transform(bp),
                              screen_transform(cp),
                              screen_transform(dp),
                              last);
}

void render_blender_object(ta_parameter_writer& writer,
                           const mat4x4& screen_trans,
                           const object * object,
                           float mod)
{
  const mat4x4 trans = screen_trans
    * translate(object->location)
    * scale(object->scale)
    * rotate_quaternion(object->rotation);

  const mesh * mesh = object->mesh;

  vec3 position[mesh->position_length];
  for (int i = 0; i < mesh->position_length; i++) {
    position[i] = trans * mesh->position[i];
  }

  global_polygon(writer,
                 para_control::list_type::opaque,
                 mod);

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * polygon = &mesh->polygons[i];
    render_polygon(writer, polygon, position);
  }
}

void render_blender_object_modifier_volume(ta_parameter_writer& writer,
                                           const mat4x4& screen_trans,
                                           const object * object)
{
  const mat4x4 trans = screen_trans
    * translate(object->location)
    * scale(object->scale)
    * rotate_quaternion(object->rotation);

  const mesh * mesh = object->mesh;

  vec3 position[mesh->position_length];
  for (int i = 0; i < mesh->position_length; i++) {
    position[i] = trans * mesh->position[i];
  }

  global_polygon_modifier_volume(writer);

  for (int i = 0; i < mesh->polygons_length; i++) {
    bool last = i == (mesh->polygons_length - 1);
    const polygon * polygon = &mesh->polygons[i];
    render_polygon_modifier_volume(writer, polygon, position, last);
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  // opaque modifier
  {
    render_blender_object_modifier_volume(writer, screen_trans, &objects[0]);

    // end of opaque list
    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }

  // opaque
  {
    //render_blender_object(writer, screen_trans, &objects[0], 0.5f);
    render_blender_object(writer, screen_trans, &objects[1], 1.0f);

    // end of opaque list
    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

static inline mat4x4 update_analog(const mat4x4& screen_trans)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;
  float x = 0.05f * -x_;
  float y = 0.05f * y_;

  mat4x4 t = translate((vec3){0, 0, y});

  float theta = 0;
  if (l_ > 0.1f) {
    theta = -0.05f * l_;
  } else if (r_ > 0.1f) {
    theta = 0.05f * r_;
  }

  mat4x4 rx = rotate_x(x);

  mat4x4 ry = rotate_y(theta);

  //mat4x4 rz = rotate_z(y);

  return t * screen_trans * ry * rx;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

void main()
{
  serial::init(0);

  interrupt_init();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::intensity_volume_mode
                       | fpu_shad_scale::scale_factor_for_shadows(128);

  system.ISTNRM = 0xffffffff;
  system.ISTERR = 0xffffffff;
  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list;

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
  do_get_condition();

  mat4x4 screen_trans = {
    1, 0,  0, 0,
    0, 0, -1, 1,
    0, 1,  0, 3.6,
    0, 0,  0, 1,
  };

  int frame = 0;

  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    screen_trans = update_analog(screen_trans);

    writer.offset = 0;
    transfer_scene(writer, screen_trans);
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

    if (0 && (frame++ % 60) == 0) {
      const mat4x4& t = screen_trans;
      printf("\n");
      printf("%f %f %f %f\n", t[0][0], t[0][1], t[0][2], t[0][3]);
      printf("%f %f %f %f\n", t[1][0], t[1][1], t[1][2], t[1][3]);
      printf("%f %f %f %f\n", t[2][0], t[2][1], t[2][2], t[2][3]);
      printf("%f %f %f %f\n", t[3][0], t[3][1], t[3][2], t[3][3]);
    }
  }
}
