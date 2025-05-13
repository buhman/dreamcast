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

#include "interrupt.hpp"
#include "assert.h"

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

#include "md5/md5mesh.h"
#include "md5/md5anim.h"
#include "model/boblamp/boblamp_mesh.h"
#include "model/boblamp/boblamp_anim.h"

static int joint_ix_sel = 0;

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
                            | ta_alloc_ctrl::om_opb::no_list
                            | ta_alloc_ctrl::o_opb::_32x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
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

    assert(ta_in_use);
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

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control,
                           uint32_t tsp_instruction_word,
                           uint32_t texture_control_word,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0,
                                        0
                                        );
}

void transfer_triangle(ta_parameter_writer& writer,
                       vec3 ap,
                       vec3 bp,
                       vec3 cp,
                       vec3 ac,
                       vec3 bc,
                       vec3 cc
                       )
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        1.0,
                                        ac.x, ac.y, ac.z);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        1.0,
                                        bc.x, bc.y, bc.z);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        1.0,
                                        cc.x, cc.y, cc.z);
}

vec4 quaternion_normalize(vec4 q)
{
  float mag = magnitude(q);
  if (mag > 0.0f) {
    return q * (1.0f / mag);
  } else {
    return q;
  }
}

vec4 quaternion_mul_quaternion(vec4 a, vec4 b)
{
  return (vec4){
    (a.x * b.w) + (a.w * b.x) + (a.y * b.z) - (a.z * b.y),
    (a.y * b.w) + (a.w * b.y) + (a.z * b.x) - (a.x * b.z),
    (a.z * b.w) + (a.w * b.z) + (a.x * b.y) - (a.y * b.x),
    (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
  };
}

vec4 quaternion_mul_vec4(vec4 q, vec3 v)
{
  return (vec4){
      (q.w * v.x) + (q.y * v.z) - (q.z * v.y),
      (q.w * v.y) + (q.z * v.x) - (q.x * v.z),
      (q.w * v.z) + (q.x * v.y) - (q.y * v.x),
    - (q.x * v.x) - (q.y * v.y) - (q.z * v.z),
  };
}

vec3 quaternion_rotate_point(vec4 q, vec3 v)
{
  vec4 neg = {
    -q.x,
    -q.y,
    -q.z,
     q.w,
  };

  vec4 qv = quaternion_mul_quaternion(quaternion_mul_vec4(q, v),
                                      quaternion_normalize(neg));
  return (vec3){qv.x, qv.y, qv.z};
}

vec3 vertex_weights(const md5_mesh_joint * joints,
                    const md5_mesh_mesh * mesh,
                    const md5_mesh_vert * v)
{
  const md5_mesh_weight * weights = &mesh->weights[v->weight_index];

  vec3 sum = {0, 0, 0};

  for (int i = 0; i < v->weight_elem; i++) {
    const md5_mesh_weight * weight = &weights[i];
    const md5_mesh_joint * joint = &joints[weight->joint_index];
    vec3 rv = quaternion_rotate_point(joint->orient, weight->pos);
    sum += (joint->pos + rv) * weight->weight_value;
  }

  return sum;
}

vec3 vertex_weight_color(const md5_mesh_joint * joints,
                         const md5_mesh_mesh * mesh,
                         const md5_mesh_vert * v)
{
  const md5_mesh_weight * weights = &mesh->weights[v->weight_index];

  for (int i = 0; i < v->weight_elem; i++) {
    const md5_mesh_weight * weight = &weights[i];
    if (weight->joint_index == joint_ix_sel) {
      return {weight->weight_value, 1.0f - weight->weight_value, 0.0};
    }
  }

  return {0.0, 0.0, 1.0};
}

static inline vec3 screen_transform(vec3 v)
{
  float x2 = 640 / 2.0;
  float y2 = 480 / 2.0;
  float iz = 1.0f / v.z;

  return {
    v.x * iz * y2 + x2,
    v.y * iz * y2 + y2,
    iz,
  };
}

float light_intensity(vec3 l1, vec3 n)
{
  float intensity = 0.2f;
  {
    float n_dot_l = dot(n, l1);
    if (n_dot_l > 0)
      intensity += 0.9f * n_dot_l * (inverse_length(n) * inverse_length(l1));
  }

  if (intensity > 1.0f)
    intensity = 1.0f;

  return intensity;
}

void transfer_mesh(ta_parameter_writer& writer,
                   const mat4x4& screen_trans,
                   const md5_mesh_joint * joints,
                   const md5_mesh_mesh * mesh)
{
  const vec3 light_pos = {1.2f, 1.0f, -2.0f};

  for (int i = 0; i < mesh->num_tris; i++) {
    const md5_mesh_tri * tri = &mesh->tris[i];
    const md5_mesh_vert * av = &mesh->verts[tri->vert_index.a];
    const md5_mesh_vert * bv = &mesh->verts[tri->vert_index.b];
    const md5_mesh_vert * cv = &mesh->verts[tri->vert_index.c];

    vec3 ap = screen_trans * vertex_weights(joints, mesh, av);
    vec3 bp = screen_trans * vertex_weights(joints, mesh, bv);
    vec3 cp = screen_trans * vertex_weights(joints, mesh, cv);

    vec3 n = -normalize(cross(bp - ap, cp - ap));

    vec3 a_light_dir = normalize(light_pos - ap);
    vec3 b_light_dir = normalize(light_pos - bp);
    vec3 c_light_dir = normalize(light_pos - cp);

    float a_diffuse = max(dot(n, a_light_dir), 0.0f);
    float b_diffuse = max(dot(n, b_light_dir), 0.0f);
    float c_diffuse = max(dot(n, c_light_dir), 0.0f);

    vec3 ac = vertex_weight_color(joints, mesh, av);
    vec3 bc = vertex_weight_color(joints, mesh, bv);
    vec3 cc = vertex_weight_color(joints, mesh, cv);

    transfer_triangle(writer,
                      screen_transform(ap),
                      screen_transform(bp),
                      screen_transform(cp),
                      ac * a_diffuse,
                      bc * b_diffuse,
                      cc * c_diffuse
                      );
  }
}

void transfer_scene(ta_parameter_writer& writer,
                    const mat4x4& screen_trans)
{
  uint32_t control = para_control::list_type::opaque;
  uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::texture_shading_instruction::decal;
  uint32_t texture_control_word = 0;
  global_polygon_type_0(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word);

  for (int i = 0; i < boblamp_mesh.num_meshes; i++) {
    transfer_mesh(writer, screen_trans, boblamp_mesh.joints, &boblamp_mesh.meshes[i]);
  }

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void update_maple(struct md5_mesh * m)
{
  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;

  static int last_ra = 0;
  static int last_la = 0;

  if (ra && last_ra == 0) {
    joint_ix_sel += 1;
    printf("joint_ix_sel: %d\n", joint_ix_sel);
    if (joint_ix_sel >= m->num_joints)
      joint_ix_sel = 0;
  }
  if (la && last_la == 0) {
    joint_ix_sel -= 1;
    printf("joint_ix_sel: %d\n", joint_ix_sel);
    if (joint_ix_sel < 0)
      joint_ix_sel = m->num_joints - 1;
  }

  last_ra = ra;
  last_la = la;
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

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

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

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

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 40,
    0, 0, 0, 1,
  };

  screen_trans = translate((vec3){0, 30, 0}) * screen_trans;

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    update_maple(&boblamp_mesh);

    screen_trans = screen_trans * rotate_z(0.01f);

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
  }
}
