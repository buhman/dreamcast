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

#include "interrupt.hpp"

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"
#include "palette.hpp"
#include "printf/unparse.h"

#include "assert.h"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

struct polygon {
  int a, b, c, d;
};

struct mesh {
  const vec3 * position;
  const int position_length;
  const vec3 * normal;
  const int normal_length;
  const polygon * polygons;
  const int polygons_length;
  // vec2 * uv_layers[]; // support for multiple UV maps
  // int uv_layers_length;
};

struct transform {
  const vec3 scale;
  const vec4 rotation;
  const vec3 location;
};

struct object {
  const struct mesh * mesh;
  const transform transforms[26];
};

#include "model/door/door.h"

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

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

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control,
                           uint32_t texture_u_v_size,
                           uint32_t texture_control_word,
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

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_shading_instruction::decal
    //| tsp_instruction_word::src_alpha_instr::one
    //| tsp_instruction_word::dst_alpha_instr::zero
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

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               float li)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        li);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        li);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        li);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        li);
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

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

static inline float inverse_length(vec3 v)
{
  float f = dot(v, v);
  return _fsrra(f);
}

float light_intensity(vec3 l1, vec3 l2, vec3 n)
{
  float intensity = 0.2f;
  {
    float n_dot_l = dot(n, l1);
    if (n_dot_l > 0)
      intensity += 0.9f * n_dot_l * (inverse_length(n) * inverse_length(l1));
  }
  {
    float n_dot_l = dot(n, l2);
    if (n_dot_l > 0)
      intensity += 0.9f * n_dot_l * (inverse_length(n) * inverse_length(l2));
  }

  if (intensity > 1.0f)
    intensity = 1.0f;

  return intensity;
}

static inline vec3 normal_transform(const mat4x4& trans, vec3 normal)
{
  vec4 n = trans * (vec4){normal.x, normal.y, normal.z, 0.f}; // no translation component
  return {n.x, n.y, n.z};
}

const vec3 light_vec = {20, -20, -20};
const vec3 light_vec2 = {20, 20, 20};

mat4x4 scale(vec3 s)
{
  return {
    s.x,   0,   0, 0,
      0, s.y,   0, 0,
      0,   0, s.z, 0,
      0,   0,   0, 1,
  };
}

mat4x4 translate(vec3 t)
{
  return {
    1, 0, 0, t.x,
    0, 1, 0, t.y,
    0, 0, 1, t.z,
    0, 0, 0,   1,
  };
}

//mat4x4 rodrigues(vec4 r) __attribute__ ((optimize(1)));
mat4x4 rodrigues(vec4 r)
{
  const vec3 k = {r.y, r.z, r.w};
  const float t = r.x;

  const mat4x4 K = {
       0, -k.z,  k.y, 0,
     k.z,    0, -k.x, 0,
    -k.y,  k.x,    0, 0,
       0,    0,    0, 1,
  };

  const mat4x4 I = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };

  const mat4x4 R = I + (K * sin(t)) + ((K * K) * (1 - cos(t)));

  return R;
}

mat4x4 quaternion(vec4 r)
{
  return {
    1 - (2 * r.y * r.y) - (2 * r.z * r.z),      (2 * r.x * r.y) - (2 * r.z * r.w),      (2 * r.x * r.z) + (2 * r.y * r.w), 0,
        (2 * r.x * r.y) + (2 * r.z * r.w),  1 - (2 * r.x * r.x) - (2 * r.z * r.z),      (2 * r.y * r.z) - (2 * r.x * r.w), 0,
        (2 * r.x * r.z) - (2 * r.y * r.w),      (2 * r.y * r.z) + (2 * r.x * r.w),  1 - (2 * r.x * r.x) - (2 * r.y * r.y), 0,
    0, 0, 0, 1,
  };
}

constexpr int animation_frames = 26;
constexpr int ticks_per_animation_frame = 64;
constexpr float tick_div = 1.0f / (float)ticks_per_animation_frame;

void transfer_mesh(ta_parameter_writer& writer, const mat4x4& trans, const object * object, int animation_tick)
{
  const mesh * mesh = object->mesh;
  uint32_t control = para_control::list_type::opaque;
  uint32_t texture_uv_size = 0;
  uint32_t texture_control_word = 0;

  global_polygon_type_1(writer,
                        control,
                        texture_uv_size,
                        texture_control_word);

  vec3 position_cache[mesh->position_length];
  vec3 normal_cache[mesh->normal_length];

  int frame_ix0 = animation_tick / ticks_per_animation_frame;
  int frame_ix1 = frame_ix0 + 1;
  if (frame_ix1 >= animation_frames)
    frame_ix1 = 0;

  float lerp = (float)(animation_tick - (frame_ix0 * ticks_per_animation_frame)) * tick_div;

  const transform& t0 = object->transforms[frame_ix0];
  const transform& t1 = object->transforms[frame_ix1];

  vec3 location = t0.location + ((t1.location - t0.location) * lerp);
  vec4 rotation = t0.rotation + ((t1.rotation - t0.rotation) * lerp);
  vec3 _scale = t0.scale + ((t1.scale - t0.scale) * lerp);

  mat4x4 trans1 = trans
    * translate(location)
    * quaternion(rotation)
    * scale(_scale);

  for (int i = 0; i < mesh->position_length; i++) {
    position_cache[i] = trans1 * mesh->position[i];
    normal_cache[i] = normal_transform(trans * quaternion(rotation), mesh->normal[i]);
  }

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    vec3 ap = screen_transform(position_cache[p->a]);
    vec3 bp = screen_transform(position_cache[p->b]);
    vec3 cp = screen_transform(position_cache[p->c]);
    vec3 dp = screen_transform(position_cache[p->d]);

    float li = light_intensity(light_vec, light_vec2, normal_cache[p->a]);
    assert(li > 0);

    render_quad(writer, ap, bp, cp, dp, li);
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& trans, int animation_tick)
{
  // opaque list
  {
    transfer_mesh(writer, trans, &objects[0], animation_tick);
    transfer_mesh(writer, trans, &objects[1], animation_tick);
    transfer_mesh(writer, trans, &objects[2], animation_tick);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

void update_analog(mat4x4& screen, mat4x4& model)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;

  float yt = -0.05f * x_;
  float xt = 0.05f * y_;

  float x = 0;
  if (ra && !la) x = -0.05;
  if (la && !ra) x =  0.05;

  float y = 0;
  float z = -0.05f * r_ + 0.05f * l_;

  mat4x4 t = {
    1, 0, 0, x,
    0, 1, 0, z,
    0, 0, 1, y,
    0, 0, 0, 1,
  };

  mat4x4 rx = {
    1, 0, 0, 0,
    0, cos(xt), -sin(xt), 0,
    0, sin(xt), cos(xt), 0,
    0, 0, 0, 1,
  };

  mat4x4 ry = {
     cos(yt), 0, sin(yt), 0,
    0, 1, 0, 0,
    -sin(yt), 0, cos(yt), 0,
    0, 0, 0, 1,
  };

  screen = screen * t;
  model = model * ry * rx;
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

    serial::string((uint8_t *)s, offset);
    serial::character('\n');
  }
  serial::character('\n');
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 3];

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
    0, 1, 0, 7,
    0, 0, 0, 1,
  };

  mat4x4 model_trans = {
    0.805, -0.577,  0.136, 0,
    0.592,  0.773, -0.224, 0,
    0.024,  0.262,  0.964, 0,
    0, 0, 0, 1,
  };

  do_get_condition();
  int animation_tick = 0;
  while (1) {
    if (0 && animation_tick == 0) {
      serial::string("screen:\n");
      render_matrix(writer, screen_trans);
      serial::string("model:\n");
      render_matrix(writer, model_trans);
    }

    maple::dma_wait_complete();
    do_get_condition();
    writer.offset = 0;

    update_analog(screen_trans, model_trans);
    transfer_scene(writer, screen_trans * model_trans, animation_tick);

    // increment tick
    animation_tick += 1;
    if (animation_tick >= animation_frames * ticks_per_animation_frame)
      animation_tick = 0;

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

    while (next_frame)
    next_frame = 0;
  }
}
