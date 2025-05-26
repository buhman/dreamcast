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

#include "math/float_types.hpp"
#include "math/transform.hpp"

#include "interrupt.hpp"
#include "assert.h"

#include "model/blender_export.h"
#include "model/torus.h"

#include "shadow_volume.hpp"

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

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control,
                           bool always,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t depth_compare_mode = always
    ? isp_tsp_instruction_word::depth_compare_mode::always
    : isp_tsp_instruction_word::depth_compare_mode::greater_or_equal
    ;
  const uint32_t isp_tsp_instruction_word = depth_compare_mode
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_shading_instruction::decal
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      ;

  const uint32_t texture_control_word = 0;

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

void global_polygon_modifier_volume(ta_parameter_writer * writer)
{
   uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                   | para_control::list_type::opaque_modifier_volume
                                   ;

   uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::normal_polygon
                                     | isp_tsp_instruction_word::culling_mode::no_culling;

  writer->append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(parameter_control_word,
                                         isp_tsp_instruction_word);
}

void global_polygon_modifier_volume_last_in_volume(ta_parameter_writer * writer)
{
  const uint32_t last_parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                             | para_control::list_type::opaque_modifier_volume
                                             | obj_control::volume::modifier_volume::last_in_volume;

  const uint32_t last_isp_tsp_instruction_word = isp_tsp_instruction_word::volume_instruction::inside_last_polygon
                                               | isp_tsp_instruction_word::culling_mode::no_culling;

  writer->append<ta_global_parameter::modifier_volume>() =
    ta_global_parameter::modifier_volume(last_parameter_control_word,
                                         last_isp_tsp_instruction_word);
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
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

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

#define fsrra(n) (1.0f / (sqrt(n)))

void transfer_line(ta_parameter_writer& writer, vec3 p1, vec3 p2)
{
  float dy = p2.y - p1.y;
  float dx = p2.x - p1.x;
  float d = fsrra(dx * dx + dy * dy) * 0.7f;
  float dy1 = dy * d;
  float dx1 = dx * d;

  vec3 ap = { p1.x +  dy1, p1.y + -dx1, p1.z };
  vec3 bp = { p1.x + -dy1, p1.y +  dx1, p1.z };
  vec3 cp = { p2.x + -dy1, p2.y +  dx1, p2.z };
  vec3 dp = { p2.x +  dy1, p2.y + -dx1, p2.z };

  float li = 1.0f;

  render_quad(writer, ap, bp, cp, dp, li, li, li, li);
}

static ta_parameter_writer * _writer;
static ta_parameter_writer * _sv_writer;

void render_quad_sv(vec3 a,
                    vec3 b,
                    vec3 c,
                    vec3 d,
                    bool last_in_volume)
{
  float ai = 1.0f;
  float bi = 1.0f;
  float ci = 1.0f;
  float di = 1.0f;

  vec3 ap = screen_transform(a);
  vec3 bp = screen_transform(b);
  vec3 cp = screen_transform(c);
  vec3 dp = screen_transform(d);

  /*
  render_quad(*_writer,
              ap,
              bp,
              cp,
              dp,
              ai,
              bi,
              ci,
              di);
  */

  /*
  transfer_line(*_writer, ap, bp);
  transfer_line(*_writer, bp, cp);
  transfer_line(*_writer, cp, dp);
  transfer_line(*_writer, dp, ap);
  */

  /*
    A B   A B   B
    D C   D     D C
   */

  _sv_writer->append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         ap.x, ap.y, ap.z,
                                         bp.x, bp.y, bp.z,
                                         dp.x, dp.y, dp.z);

  if (last_in_volume) {
    global_polygon_modifier_volume_last_in_volume(_sv_writer);
  }

  _sv_writer->append<ta_vertex_parameter::modifier_volume>() =
    ta_vertex_parameter::modifier_volume(modifier_volume_vertex_parameter_control_word(),
                                         bp.x, bp.y, bp.z,
                                         cp.x, cp.y, cp.z,
                                         dp.x, dp.y, dp.z);
}

float _rotate_x = 0;

void transfer_mesh(ta_parameter_writer& writer,
                   const mat4x4& screen_trans,
                   const object * object,
                   const vec3 light,
                   const bool cast_shadow,
                   const bool receive_shadow,
                   const bool diffuse,
                   vec3 color)
{
  const mesh * mesh = object->mesh;

  vec3 position[mesh->position_length];
  vec3 polygon_normal[mesh->polygon_normal_length];
  assert(mesh->polygon_normal_length == mesh->polygons_length);

  mat4x4 trans = screen_trans
    * translate(object->location)
    * rotate_x(_rotate_x)
    * rotate_quaternion(object->rotation)
    * scale(object->scale);

  for (int i = 0; i < mesh->position_length; i++) {
    position[i] = trans * mesh->position[i];
  }
  for (int i = 0; i < mesh->polygon_normal_length; i++) {
    polygon_normal[i] = normalize(normal_multiply(trans, mesh->polygon_normal[i]));
  }

  bool always = false;
  uint32_t shadow = receive_shadow ? obj_control::shadow : 0;
  uint32_t control = para_control::list_type::opaque | shadow;
  global_polygon_type_1(writer,
                        control,
                        always,
                        1.0f,
                        color.x, color.y, color.z);

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    vec3 ap = screen_transform(position[p->a]);
    vec3 bp = screen_transform(position[p->b]);
    vec3 cp = screen_transform(position[p->c]);
    vec3 dp = screen_transform(position[p->d]);

    float li = 1.0f;
    if (diffuse) {
      vec3 light_dir = normalize(light - position[p->a]);
      float diffuse = max(dot(polygon_normal[i], light_dir), 0.0f);
      li = 0.5 + 0.6 * diffuse;
    }

    render_quad(writer, ap, bp, cp, dp, li, li, li, li);
  }

  if (cast_shadow) {
    global_polygon_modifier_volume(_sv_writer);

    global_polygon_type_1(writer,
                          control,
                          always,
                          1, 1, 0.5, 0.5);

    shadow_volume_mesh(light, position, polygon_normal, mesh, render_quad_sv);
  }
}

mat4x4 light_trans = mat4x4();
float _torus_rx = 0;

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  light_trans = rotate_z(0.01f) * light_trans;
  vec3 light = screen_trans * light_trans * objects[0].location;

  // opaque list
  {
    _rotate_x = 0;
    transfer_mesh(writer, screen_trans * light_trans, &objects[0], light,
                  false, // cast shadow
                  false, // receive shadow
                  false, // diffuse
                  (vec3){0.9, 0.9, 0.9}
                  );

    _rotate_x = 0;
    transfer_mesh(writer, screen_trans, &objects[1], light,
                  false, // cast shadow
                  true,  // receive shadow
                  true, // diffuse
                  (vec3){0.5, 0.9, 0.5}
                  );

    _rotate_x = _torus_rx;
    _torus_rx += 0.001f;
    transfer_mesh(writer, screen_trans, &objects[2], light,
                  true,   // cast shadow
                  false,  // receive shadow
                  true,   // diffuse
                  (vec3){1, 0, 1}
                  );

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    _sv_writer->append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

mat4x4 update_analog(mat4x4& screen_trans)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  float y = -0.05f * x_;
  float x = 0.05f * y_;

  float z = -0.05f * r_ + 0.05f * l_;

  return translate((vec3){0, 0, z}) *
    screen_trans *
    rotate_x(x) *
    rotate_z(y);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf1[1024 * 1024];
uint8_t __attribute__((aligned(32))) ta_parameter_buf2[1024 * 1024];

int main()
{
  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  serial::init(0);

  interrupt_init();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::intensity_volume_mode
                       | fpu_shad_scale::scale_factor_for_shadows(128);

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

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf1, (sizeof (ta_parameter_buf1)));
  ta_parameter_writer sv_writer = ta_parameter_writer(ta_parameter_buf2, (sizeof (ta_parameter_buf2)));
  _writer = &writer;
  _sv_writer = &sv_writer;

  video_output::set_mode_vga();

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 7,
    0, 0, 0, 1,
  };

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    screen_trans = update_analog(screen_trans);

    writer.offset = 0;
    sv_writer.offset = 0;
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
    ta_polygon_converter_writeback(sv_writer.buf, sv_writer.offset);
    ta_polygon_converter_transfer(sv_writer.buf, sv_writer.offset);
    ta_wait_opaque_modifier_volume_list();
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);

    while (next_frame == 0);
    next_frame = 0;
  }
}
