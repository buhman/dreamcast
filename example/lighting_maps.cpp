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

#include "interrupt.hpp"

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"
#include "palette.hpp"
#include "printf/unparse.h"

#include "texture/container/container2.data.h"
#include "texture/container/container2_specular.data.h"

#include "assert.h"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#include "model/blender_export.h"
#include "model/grid.h"

const uint32_t texture_size = 512 * 512 * 2;
const uint32_t diffuse_texture_offset = texture_size * 0;
const uint32_t specular_texture_offset = texture_size * 1;

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
                            | ta_alloc_ctrl::t_opb::_32x4byte
                            | ta_alloc_ctrl::om_opb::no_list
                            | ta_alloc_ctrl::o_opb::_32x4byte;

constexpr int ta_cont_count = 1;
constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
    .translucent = 32 * 4,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};

static volatile int ta_in_use = 0;
static volatile int core_in_use = 0;
static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static ta_parameter_writer * _writer_tl;

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
    //serial::string("istnrm op\n");
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

    assert(ta_in_use);
    ta_polygon_converter_writeback(_writer_tl->buf, _writer_tl->offset);
    ta_polygon_converter_transfer(_writer_tl->buf, _writer_tl->offset);
  }

  if (istnrm & istnrm::end_of_transferring_translucent_list) {
    //serial::string("istnrm tl\n");
    system.ISTNRM = istnrm::end_of_transferring_translucent_list;

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

void global_polygon_type_1(ta_parameter_writer& writer,
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
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
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
                               vec2 at,
                               vec2 bt,
                               vec2 ct,
                               vec2 dt,
                               float ai,
                               float bi,
                               float ci,
                               float di)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        ai,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        bi,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        di,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        ci,
                                        0);
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

void transfer_mesh(ta_parameter_writer& writer_op,
                   ta_parameter_writer& writer_tl,
                   const mat4x4& trans,
                   const mesh * mesh)
{
  {
    uint32_t control = para_control::list_type::opaque
                     | obj_control::texture;
    uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                  | tsp_instruction_word::dst_alpha_instr::zero
                                  | tsp_instruction_word::fog_control::no_fog
                                  | tsp_instruction_word::texture_shading_instruction::modulate
                                  | tsp_instruction_word::texture_u_size::from_int(512)
                                  | tsp_instruction_word::texture_v_size::from_int(512);
    uint32_t texture_address = texture_memory_alloc.texture.start + diffuse_texture_offset;
    uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                  | texture_control_word::scan_order::twiddled
                                  | texture_control_word::texture_address(texture_address / 8);
    global_polygon_type_1(writer_op,
                          control,
                          tsp_instruction_word,
                          texture_control_word);
  }
  {
    uint32_t control = para_control::list_type::translucent
                     | obj_control::texture;
    uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                  | tsp_instruction_word::dst_alpha_instr::one
                                  | tsp_instruction_word::fog_control::no_fog
                                  | tsp_instruction_word::texture_shading_instruction::modulate
                                  | tsp_instruction_word::texture_u_size::from_int(512)
                                  | tsp_instruction_word::texture_v_size::from_int(512);
    uint32_t texture_address = texture_memory_alloc.texture.start + specular_texture_offset;
    uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                  | texture_control_word::scan_order::twiddled
                                  | texture_control_word::texture_address(texture_address / 8);
    global_polygon_type_1(writer_tl,
                          control,
                          tsp_instruction_word,
                          texture_control_word);
  }

  for (int i = 0; i < mesh->polygons_length; i++) {
    const struct polygon * polygon = &mesh->polygons[i];

    vec3 ap = trans * mesh->position[polygon->a];
    vec3 bp = trans * mesh->position[polygon->b];
    vec3 cp = trans * mesh->position[polygon->c];
    vec3 dp = trans * mesh->position[polygon->d];

    vec3 light_pos = {1.2f, 1.0f, -2.0f};
    vec3 n = normal_multiply(trans, mesh->polygon_normal[i]);

    vec3 a_light_dir = normalize(light_pos - ap);
    vec3 b_light_dir = normalize(light_pos - bp);
    vec3 c_light_dir = normalize(light_pos - cp);
    vec3 d_light_dir = normalize(light_pos - dp);
    float a_diffuse = max(dot(n, a_light_dir), 0.0f);
    float b_diffuse = max(dot(n, b_light_dir), 0.0f);
    float c_diffuse = max(dot(n, c_light_dir), 0.0f);
    float d_diffuse = max(dot(n, d_light_dir), 0.0f);

    vec3 view_pos = {0, 0, 0};
    vec3 a_view_dir = normalize(view_pos - ap);
    vec3 a_reflect_dir = reflect(-a_light_dir, n);
    float a_spec = __builtin_powf(max(dot(a_view_dir, a_reflect_dir), 0.0f), 64.0f);

    vec3 b_view_dir = normalize(view_pos - bp);
    vec3 b_reflect_dir = reflect(-b_light_dir, n);
    float b_spec = __builtin_powf(max(dot(b_view_dir, b_reflect_dir), 0.0f), 64.0f);

    vec3 c_view_dir = normalize(view_pos - cp);
    vec3 c_reflect_dir = reflect(-c_light_dir, n);
    float c_spec = __builtin_powf(max(dot(c_view_dir, c_reflect_dir), 0.0f), 64.0f);

    vec3 d_view_dir = normalize(view_pos - dp);
    vec3 d_reflect_dir = reflect(-d_light_dir, n);
    float d_spec = __builtin_powf(max(dot(d_view_dir, d_reflect_dir), 0.0f), 64.0f);

    float ai = a_diffuse * 0.7;
    float bi = b_diffuse * 0.7;
    float ci = c_diffuse * 0.7;
    float di = d_diffuse * 0.7;

    vec2 at = mesh->uv_layers[0][i * 4 + 0];
    vec2 bt = mesh->uv_layers[0][i * 4 + 1];
    vec2 ct = mesh->uv_layers[0][i * 4 + 2];
    vec2 dt = mesh->uv_layers[0][i * 4 + 3];

    vec3 aps = screen_transform(ap);
    vec3 bps = screen_transform(bp);
    vec3 cps = screen_transform(cp);
    vec3 dps = screen_transform(dp);

    render_quad(writer_op,
                aps,
                bps,
                cps,
                dps,
                at,
                bt,
                ct,
                dt,
                ai,
                bi,
                ci,
                di);

    render_quad(writer_tl,
                aps,
                bps,
                cps,
                dps,
                at,
                bt,
                ct,
                dt,
                a_spec,
                b_spec,
                c_spec,
                d_spec);
  }
}

void transfer_scene(ta_parameter_writer& writer_op,
                    ta_parameter_writer& writer_tl,
                    const mat4x4& screen_trans)
{
  // opaque list
  {
    for (uint32_t i = 0; i < (sizeof (objects)) / (sizeof (objects[0])); i++) {
      mat4x4 trans = screen_trans
        * translate(objects[i].location)
        * rotate_quaternion(objects[i].rotation)
        ;

      transfer_mesh(writer_op, writer_tl, trans, objects[i].mesh);
    }

    writer_op.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    writer_tl.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

mat4x4 update_analog()
{
  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  float yt = -0.05f * x_;
  float xt = 0.05f * y_;

  return rotate_x(yt) * rotate_z(xt);
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  assert((((int)dst) & 31) == 0);
  assert((((int)length) & 31) == 0);

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

void transfer_container_textures()
{
  {
    uint32_t offset = texture_memory_alloc.texture.start + diffuse_texture_offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(&_binary_texture_container_container2_data_start);
    uint32_t size = reinterpret_cast<uint32_t>(&_binary_texture_container_container2_data_size);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }

  {
    uint32_t offset = texture_memory_alloc.texture.start + specular_texture_offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = reinterpret_cast<void *>(&_binary_texture_container_container2_specular_data_start);
    uint32_t size = reinterpret_cast<uint32_t>(&_binary_texture_container_container2_specular_data_size);
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  transfer_container_textures();
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf_op[1024 * 1024 / 2];
uint8_t __attribute__((aligned(32))) ta_parameter_buf_tl[1024 * 1024 / 2];

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
  transfer_textures();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list
                 | istnrm::end_of_transferring_translucent_list;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff202040);

  ta_parameter_writer writer_op = ta_parameter_writer(ta_parameter_buf_op, (sizeof (ta_parameter_buf_op)));
  ta_parameter_writer writer_tl = ta_parameter_writer(ta_parameter_buf_tl, (sizeof (ta_parameter_buf_tl)));
  _writer_tl = &writer_tl;

  video_output::set_mode_vga();

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 4,
    0, 0, 0, 1,
  };

  do_get_condition();
  while (1) {
    screen_trans = screen_trans * rotate_x(0.005f) * rotate_y(0.005f);

    maple::dma_wait_complete();
    do_get_condition();

    writer_op.offset = 0;
    writer_tl.offset = 0;
    transfer_scene(writer_op, writer_tl, screen_trans);

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
    ta_polygon_converter_writeback(writer_op.buf, writer_op.offset);
    ta_polygon_converter_transfer(writer_op.buf, writer_op.offset);

    while (next_frame == 0);
    next_frame = 0;
  }
}
