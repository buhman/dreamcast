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

#include "model/blender_export.h"
#include "model/bloom_scene/scene.h"
#include "model/bloom_scene/wood.data.h"
#include "model/bloom_scene/container2.data.h"

constexpr int bloom_width = 128;
constexpr int bloom_height = 96;
constexpr float bloom_u_size = 128;
constexpr float bloom_v_size = 128;

struct texture {
  const void * start;
  const uint32_t size;
  const int offset;
  const int width;
  const int height;
};

enum texture_e {
  TEX_WOOD,
  TEX_CONTAINER2,
};

const struct texture textures[] = {
  [TEX_WOOD] = {
    .start = (void *)&_binary_model_bloom_scene_wood_data_start,
    .size = (uint32_t)&_binary_model_bloom_scene_wood_data_size,
    .offset = bloom_width * bloom_height * 2,
    .width = 1024,
    .height = 1024,
  },
  [TEX_CONTAINER2] = {
    .start = (void *)&_binary_model_bloom_scene_container2_data_start,
    .size = (uint32_t)&_binary_model_bloom_scene_container2_data_size,
    .offset = bloom_width * bloom_height * 2 + (1024 * 1024 * 2),
    .width = 512,
    .height = 512,
  },
};

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

struct tile_param {
  int framebuffer_width;
  int framebuffer_height;
  int region_array_offset;
  consteval int tile_width() const {
    return ((uint32_t)framebuffer_width) >> 5;
  }
  consteval int tile_height() const {
    return ((uint32_t)framebuffer_height) >> 5;
  }
};

constexpr tile_param tile_param[2] = {
  {
    .framebuffer_width = bloom_width,
    .framebuffer_height = bloom_height,
    .region_array_offset = 0,
  },
  {
    .framebuffer_width = 640,
    .framebuffer_height = 480,
    .region_array_offset = (bloom_width / 32) * (bloom_height / 32) * (sizeof (struct region_array_entry)),
  }
};

constexpr int ta_cont_count = 2;

constexpr uint32_t ta_alloc[ta_cont_count] = {
  {
      ta_alloc_ctrl::pt_opb::no_list
    | ta_alloc_ctrl::tm_opb::no_list
    | ta_alloc_ctrl::t_opb::no_list
    | ta_alloc_ctrl::om_opb::no_list
    | ta_alloc_ctrl::o_opb::_32x4byte,
  },
  {
      ta_alloc_ctrl::pt_opb::no_list
    | ta_alloc_ctrl::tm_opb::no_list
    | ta_alloc_ctrl::t_opb::_8x4byte
    | ta_alloc_ctrl::om_opb::no_list
    | ta_alloc_ctrl::o_opb::_32x4byte,
  },
};

constexpr struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
    .translucent = 0,
    .translucent_modifier = 0,
    .punch_through = 0
  },
  {
    .opaque = 32 * 4,
    .opaque_modifier = 0,
        .translucent = 8 * 4,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};

static volatile int ta_in_use = 0;
static volatile int core_in_use = 0;
static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static volatile int render_step = 0;

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

    core_in_use = 0;
  }

  if (istnrm & istnrm::end_of_transferring_opaque_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

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
                           uint32_t depth_compare_mode = isp_tsp_instruction_word::depth_compare_mode::greater
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = depth_compare_mode
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

static inline float clamp(float f)
{
  if (f > 1.0)
    return 1.0;
  if (f < 0.0)
    return 0.0;
  return f;
}

void transfer_quad(ta_parameter_writer& writer,
                   vec3 ap,
                   vec3 bp,
                   vec3 cp,
                   vec3 dp,
                   vec3 ac,
                   vec3 bc,
                   vec3 cc,
                   vec3 dc
                   )
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  const float a = 1.0;

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        a, ac.x, ac.y, ac.z);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        a, bc.x, bc.y, bc.z);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        a, dc.x, dc.y, dc.z);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        a, cc.x, cc.y, cc.z);
}

void transfer_quad_textured(ta_parameter_writer& writer,
                            vec3 ap,
                            vec3 bp,
                            vec3 cp,
                            vec3 dp,
                            vec2 at,
                            vec2 bt,
                            vec2 ct,
                            vec2 dt,
                            vec3 ac,
                            vec3 bc,
                            vec3 cc,
                            vec3 dc
                            )
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  const float a = 1.0;

  writer.append<ta_vertex_parameter::polygon_type_5>() =
    ta_vertex_parameter::polygon_type_5(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        a, ac.x, ac.y, ac.z,
                                        0, 0, 0, 0);

  writer.append<ta_vertex_parameter::polygon_type_5>() =
    ta_vertex_parameter::polygon_type_5(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        a, bc.x, bc.y, bc.z,
                                        0, 0, 0, 0);

  writer.append<ta_vertex_parameter::polygon_type_5>() =
    ta_vertex_parameter::polygon_type_5(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        a, dc.x, dc.y, dc.z,
                                        0, 0, 0, 0);

  writer.append<ta_vertex_parameter::polygon_type_5>() =
    ta_vertex_parameter::polygon_type_5(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        a, cc.x, cc.y, cc.z,
                                        0, 0, 0, 0);
}

vec3 screen_transform1(const vec3& v)
{
  float w2 = bloom_width / 2.0;
  float h2 = bloom_height / 2.0;
  float dim = w2;
  float iz = 1.0 / v.z;

  return {
    v.x * iz * dim + w2,
    v.y * iz * dim + h2,
    iz,
  };
}

vec3 screen_transform2(const vec3& v)
{
  float w2 = 640 / 2.0;
  float h2 = 480 / 2.0;
  float dim = w2;
  float iz = 1.0 / v.z;

  return {
    v.x * iz * dim + w2,
    v.y * iz * dim + h2,
    iz,
  };
}

template <vec3 (*FC)(const mat4x4& trans, const vec3& base_color, const vec3& position),
          vec3 (*FS)(const vec3& v) = screen_transform2>
void transfer_mesh(ta_parameter_writer& writer,
                   const mat4x4& trans,
                   const object * object,
                   vec3 base_color)
{
  uint32_t control = para_control::list_type::opaque;
  uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::texture_shading_instruction::decal
                                | tsp_instruction_word::fog_control::no_fog
    ;
  uint32_t texture_control_word = 0;

  global_polygon_type_0(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word);

  const mesh * mesh = object->mesh;

  vec3 position_cache[mesh->position_length];
  vec3 color_cache[mesh->position_length];

  mat4x4 trans_p = trans
    * translate(object->location)
    * rotate_quaternion(object->rotation)
    * scale(object->scale);

  for (int i = 0; i < mesh->position_length; i++) {
    vec3 p = trans_p * mesh->position[i];
    position_cache[i] = p;
    color_cache[i] = FC(trans, base_color, p);
  }

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    vec3 ap = FS(position_cache[p->a]);
    vec3 bp = FS(position_cache[p->b]);
    vec3 cp = FS(position_cache[p->c]);
    vec3 dp = FS(position_cache[p->d]);

    vec3 ac = color_cache[p->a];
    vec3 bc = color_cache[p->b];
    vec3 cc = color_cache[p->c];
    vec3 dc = color_cache[p->d];

    transfer_quad(writer, ap, bp, cp, dp, ac, bc, cc, dc);
  }
}

vec3 color_diffuse(const mat4x4& trans, const vec3& base_color, const vec3& normal, const vec3& vertex_position);
vec3 color_specular(const mat4x4& trans, const vec3& base_color, const vec3& normal, const vec3& vertex_position);

template <vec3 (*FC)(const mat4x4& trans, const vec3& base_color, const vec3& position),
          vec3 (*FS)(const vec3& v) = screen_transform2>
void transfer_mesh_textured(ta_parameter_writer& writer,
                            const mat4x4& trans,
                            const object * object,
                            vec3 base_color,
                            const texture * texture,
                            float uv_mul)
{
  uint32_t control = para_control::list_type::opaque
                   | obj_control::texture;
  uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::texture_shading_instruction::modulate
                                | tsp_instruction_word::texture_u_size::from_int(texture->width)
                                | tsp_instruction_word::texture_v_size::from_int(texture->height)
                                | tsp_instruction_word::filter_mode::bilinear_filter;
  uint32_t texture_address = texture_memory_alloc.texture.start + texture->offset;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::twiddled
                                | texture_control_word::texture_address(texture_address / 8);

  global_polygon_type_0(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word);

  const mesh * mesh = object->mesh;

  vec3 position_cache[mesh->position_length];
  vec3 color_cache[mesh->position_length];
  //vec3 normal_cache[mesh->normal_length];

  mat4x4 trans_p = trans
    * translate(object->location)
    * rotate_quaternion(object->rotation)
    * scale(object->scale);

  //mat4x4 trans_n = trans
  //* rotate_quaternion(object->rotation);

  for (int i = 0; i < mesh->position_length; i++) {
    vec3 p = trans_p * mesh->position[i];
    position_cache[i] = p;
    color_cache[i] = FC(trans, base_color, p);
    //normal_cache[i] = normal_multiply(trans_n, mesh->normal[i]);
  }

  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    //vec3 normal = normalize(normal_multiply(trans_n, mesh->polygon_normal[i]));

    vec3 ap = FS(position_cache[p->a]);
    vec3 bp = FS(position_cache[p->b]);
    vec3 cp = FS(position_cache[p->c]);
    vec3 dp = FS(position_cache[p->d]);

    vec3 ac = color_cache[p->a];// * color_diffuse(trans_p, base_color, normal, ap);
    vec3 bc = color_cache[p->b];// * color_diffuse(trans_p, base_color, normal, bp);
    vec3 cc = color_cache[p->c];// * color_diffuse(trans_p, base_color, normal, cp);
    vec3 dc = color_cache[p->d];// * color_diffuse(trans_p, base_color, normal, dp);
    //vec3 ac = color_specular(trans_p, base_color, normal, ap);
    //vec3 bc = color_specular(trans_p, base_color, normal, bp);
    //vec3 cc = color_specular(trans_p, base_color, normal, cp);
    //vec3 dc = color_specular(trans_p, base_color, normal, dp);

    vec2 at = mesh->uv_layers[0][i * 4 + 0] * uv_mul;
    vec2 bt = mesh->uv_layers[0][i * 4 + 1] * uv_mul;
    vec2 ct = mesh->uv_layers[0][i * 4 + 2] * uv_mul;
    vec2 dt = mesh->uv_layers[0][i * 4 + 3] * uv_mul;

    transfer_quad_textured(writer,
                           ap, bp, cp, dp,
                           at, bt, ct, dt,
                           ac, bc, cc, dc);
  }
}

constexpr vec2 plane[] = {
  {  0,  0},
  {  1,  0},
  {  1,  1},
  {  0,  1},
};

void transfer_ss_plane(ta_parameter_writer& writer)
{
  uint32_t control = para_control::list_type::translucent
                   | obj_control::texture;
  uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::one
                                | tsp_instruction_word::texture_shading_instruction::decal
                                | tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::texture_u_size::from_int(bloom_u_size)
                                | tsp_instruction_word::texture_v_size::from_int(bloom_v_size)
                                | tsp_instruction_word::filter_mode::bilinear_filter
    | tsp_instruction_word::clamp_uv::uv;
  const uint32_t texture_address = texture_memory_alloc.texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::non_twiddled
    //| texture_control_word::stride_select
                                      | texture_control_word::texture_address(texture_address / 8);

  global_polygon_type_0(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word,
                        isp_tsp_instruction_word::depth_compare_mode::always);

  constexpr vec3 size = {640, 480, 1};

  constexpr vec3 ap = (vec3){plane[0].x, plane[0].y, 0.1} * size;
  constexpr vec3 bp = (vec3){plane[1].x, plane[1].y, 0.1} * size;
  constexpr vec3 cp = (vec3){plane[2].x, plane[2].y, 0.1} * size;
  constexpr vec3 dp = (vec3){plane[3].x, plane[3].y, 0.1} * size;

  constexpr vec2 tscale = {
    (float)bloom_width / bloom_u_size,
    (float)bloom_height / bloom_v_size
  };

  constexpr vec2 at = plane[0] * tscale;
  constexpr vec2 bt = plane[1] * tscale;
  constexpr vec2 ct = plane[2] * tscale;
  constexpr vec2 dt = plane[3] * tscale;

  constexpr vec3 c = {1.0, 1.0, 1.0};

  transfer_quad_textured(writer,
                         ap, bp, cp, dp,
                         at, bt, ct, dt,
                         c, c, c, c);
}

const vec3 colors[] = {
  {0, 0, 1},
  {0, 1, 0},
  {1, 0, 0},
  {1, 1, 1},
};

vec3 color_identity(const mat4x4& trans, const vec3& base_color, const vec3& position)
{
  return base_color;
}

vec3 color_diffuse(const mat4x4& trans, const vec3& base_color, const vec3& normal, const vec3& vertex_position)
{
  vec3 attenuation = {0, 0, 0};

  for (int i = 0; i < 4; i++) {
    const object * object = &objects[6 + i];
    vec3 light_position = trans * object->location;

    vec3 light_dir = normalize(light_position - vertex_position);

    float diffuse = max(dot(normal, light_dir), 0.0f);
    attenuation += colors[i] * diffuse;
  }

  return base_color * attenuation;
}

vec3 color_specular(const mat4x4& trans, const vec3& base_color, const vec3& normal, const vec3& vertex_position)
{
  vec3 attenuation = {0.1, 0.1, 0.1};

  for (int i = 0; i < 4; i++) {
    const object * object = &objects[6 + i];
    vec3 light_position = trans * object->location;

    vec3 light_dir = normalize(light_position - vertex_position);
    vec3 view_position = {0, 0, 0};
    vec3 view_dir = normalize(view_position - vertex_position);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular = __builtin_powf(max(dot(view_dir, reflect_dir), 0.0f), 64.0f);

    attenuation += colors[i] * specular;
  }

  return base_color * attenuation;
}

vec3 color_point(const mat4x4& trans, const vec3& base_color, const vec3& position)
{
  float constant = 1.0;
  float linear = 0.7;
  float quadratic = 1.8;

  vec3 attenuation = {0, 0, 0};

  for (int i = 0; i < 4; i++) {
    const object * object = &objects[6 + i];
    vec3 light_position = trans * object->location;

    float distance = magnitude(light_position - position);
    float intensity = 1.0 / (constant +
                             linear * distance +
                             quadratic * (distance * distance));
    attenuation += colors[i] * intensity;
  }

  return base_color * attenuation;
}

void transfer_scene1(ta_parameter_writer& writer, const mat4x4& trans)
{
  for (int i = 0; i < 4; i++) {
    transfer_mesh<color_identity, screen_transform1>(writer, trans, &objects[6 + i], colors[i]);
  }

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void transfer_scene2(ta_parameter_writer& writer, const mat4x4& trans)
{
  transfer_ss_plane(writer);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  transfer_mesh_textured<color_point>(writer, trans, &objects[0], colors[3],
                                      &textures[0],
                                      2);
  for (int i = 1; i < 6; i++) {
    transfer_mesh_textured<color_point>(writer, trans, &objects[i], colors[3],
                                        &textures[1],
                                        1);
  }

  for (int i = 0; i < 4; i++) {
    transfer_mesh<color_identity>(writer, trans, &objects[6 + i], colors[i]);
  }

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void update_analog(mat4x4& screen)
{
  const float l_ = static_cast<float>(data[0].analog_coordinate_axis[0]) * (1.f / 255.f);
  const float r_ = static_cast<float>(data[0].analog_coordinate_axis[1]) * (1.f / 255.f);

  const float x_ = static_cast<float>(data[0].analog_coordinate_axis[2] - 0x80) / 127.f;
  const float y_ = static_cast<float>(data[0].analog_coordinate_axis[3] - 0x80) / 127.f;

  float yt = -0.05f * x_;
  float xt = 0.05f * y_;

  mat4x4 ry = rotate_z(yt);
  mat4x4 rx = rotate_x(xt);

  screen = screen * ry * rx;
}

void ch1_dma_transfer(void * source, void * destination, uint32_t transfers)
{
  using namespace dmac;

  volatile uint32_t _dummy = sh7091.DMAC.CHCR1;
  (void)_dummy;

  sh7091.DMAC.CHCR1 = 0;

  assert((((uint32_t)source) & 0b11111) == 0);
  assert((((uint32_t)destination) & 0b11111) == 0);
  sh7091.DMAC.SAR1 = (uint32_t)source;
  sh7091.DMAC.DAR1 = (uint32_t)destination;
  sh7091.DMAC.DMATCR1 = transfers & 0x00ff'ffff;

  sh7091.DMAC.CHCR1 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0100) /* auto request, external address space → external address space */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                  //| chcr::tm::cycle_steal_mode /* transmit mode */
                    | chcr::ts::_32_byte           /* transfer size */
                  //| chcr::ie::interrupt_request_generated
                    | chcr::de::channel_operation_enabled;

  for (uint32_t i = 0; i < transfers; i++) {
    asm volatile ("ocbp @%0"
		  :                          // output
		  : "r" (((uint32_t)destination) + (32 * i)) // input
		  );
  }

  // wait for DMA completion
  while ((sh7091.DMAC.CHCR1 & dmac::chcr::te::transfers_completed) == 0);
}

void ch2_dma_transfer(void * source, void * destination, uint32_t transfers)
{
  using namespace dmac;

  assert((((uint32_t)source) & 0b11111) == 0);
  assert((((uint32_t)destination) & 0b11111) == 0);

  for (uint32_t i = 0; i < transfers; i++) {
    asm volatile ("ocbwb @%0"
		  :                          // output
		  : "r" (((uint32_t)source) + (32 * i)) // input
		  );
  }

  // this dummy read appears to be required on real hardware.
  volatile uint32_t _dummy = sh7091.DMAC.CHCR2;
  (void)_dummy;

  /* start a new CH2-DMA transfer from "system memory" to "TA FIFO polygon converter" */
  sh7091.DMAC.CHCR2 = 0; /* disable DMA channel */

  sh7091.DMAC.SAR2 = (uint32_t)source;  /* start address, must be aligned to a CHCHR__TS-sized (32-byte) boundary */
  sh7091.DMAC.DMATCR2 = dmatcr::transfer_count(transfers); /* transfer count, in CHCHR__TS-sized (32-byte) units */
  sh7091.DMAC.CHCR2 = chcr::dm::destination_address_incremented
                    | chcr::sm::source_address_incremented
                    | chcr::rs::resource_select(0b0010) /* external request, single address mode;
					                   external address space → external device */
                    | chcr::tm::cycle_burst_mode /* transmit mode */
                    | chcr::ts::_32_byte         /* transfer size */
                    | chcr::de::channel_operation_enabled;

  system.C2DSTAT = c2dstat::texture_memory_start_address((uint32_t)destination); /* CH2-DMA destination address */
  system.C2DLEN  = c2dlen::transfer_length(transfers * 32);         /* CH2-DMA length (must be a multiple of 32) */
  system.C2DST   = 1;          /* CH2-DMA start (an 'external' request from SH7091's perspective) */

  // wait for ch2-dma completion
  while ((system.ISTNRM & istnrm::end_of_dma_ch2_dma) == 0);
  // reset ch2-dma interrupt status
  system.ISTNRM = istnrm::end_of_dma_ch2_dma;
}

static void dma_init()
{
  using namespace dmac;

  sh7091.DMAC.CHCR0 = 0;
  sh7091.DMAC.CHCR1 = 0;
  sh7091.DMAC.CHCR2 = 0;
  sh7091.DMAC.CHCR3 = 0;
  sh7091.DMAC.DMAOR = dmaor::ddt::on_demand_data_transfer_mode       /* on-demand data transfer mode */
                    | dmaor::pr::ch2_ch0_ch1_ch3                     /* priority mode; CH2 > CH0 > CH1 > CH3 */
                    | dmaor::dme::operation_enabled_on_all_channels; /* DMAC master enable */

}

void gauss_rgb565(uint16_t const * const src, uint16_t * const dst);

void transfer_ta_fifo_texture_memory_32byte(void * dst, const void * src, int length)
{
  assert((((int)dst) & 31) == 0);
  assert((((int)length) & 31) == 0);

  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffe0) / 4];
  const uint32_t * src32 = reinterpret_cast<const uint32_t *>(src);

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

void transfer_bloom_scene_textures()
{
  for (int i = 0; i < 2; i++) {
    uint32_t offset = texture_memory_alloc.texture.start + textures[i].offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    transfer_ta_fifo_texture_memory_32byte(dst, textures[i].start, textures[i].size);
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0;
  system.LMMODE1 = 0; // 64-bit

  transfer_bloom_scene_textures();
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf1[1024 * 1024];
uint8_t __attribute__((aligned(32))) ta_parameter_buf2[1024 * 1024];

int main()
{
  serial::init(0);

  interrupt_init();
  dma_init();
  transfer_textures();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;
  holly.TEXT_CONTROL = text_control::stride(5);

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list;

  region_array_multipass(tile_param[0].tile_width(),
                         tile_param[0].tile_height(),
                         &opb_size[0],
                         1,
                         texture_memory_alloc.region_array.start + tile_param[0].region_array_offset,
                         texture_memory_alloc.object_list.start);

  region_array_multipass(tile_param[1].tile_width(),
                         tile_param[1].tile_height(),
                         &opb_size[1],
                         1,
                         texture_memory_alloc.region_array.start + tile_param[1].region_array_offset,
                         texture_memory_alloc.object_list.start,
                         REGION_ARRAY__PRE_SORT);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff000000);
  background_parameter2(texture_memory_alloc.background[1].start,
                        0xff000000);

  ta_parameter_writer writer1 = ta_parameter_writer(ta_parameter_buf1, (sizeof (ta_parameter_buf1)));
  ta_parameter_writer writer2 = ta_parameter_writer(ta_parameter_buf2, (sizeof (ta_parameter_buf2)));

  video_output::set_mode_vga();

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, 1, 0, 3,
    0, 0, 0, 1,
  };

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    update_analog(screen_trans);

    writer1.offset = 0;
    transfer_scene1(writer1, screen_trans);
    writer2.offset = 0;
    transfer_scene2(writer2, screen_trans);

    if (1) { // ta 0
      assert(!ta_in_use); ta_in_use = 1;
      ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters.start,
                                 texture_memory_alloc.isp_tsp_parameters.end,
                                 texture_memory_alloc.object_list.start,
                                 texture_memory_alloc.object_list.end,
                                 opb_size[0].total(),
                                 ta_alloc[0],
                                 tile_param[0].tile_width(),
                                 tile_param[0].tile_height());
      ta_polygon_converter_writeback(writer1.buf, writer1.offset);
      ta_polygon_converter_transfer(writer1.buf, writer1.offset);

      while (ta_in_use);
    }

    if (1) { // core 0
      assert(!core_in_use); core_in_use = 1;

      uint32_t region_array_start = texture_memory_alloc.region_array.start
                                  + tile_param[0].region_array_offset;
      uint32_t framebuffer_start = 0x100'0000 | texture_memory_alloc.texture.start;
      core_start_render2(region_array_start,
                         texture_memory_alloc.isp_tsp_parameters.start,
                         texture_memory_alloc.background[0].start,
                         framebuffer_start,
                         tile_param[0].framebuffer_width
                         );

      while (core_in_use);
    }

    { // gauss
      static uint16_t  input[bloom_width * bloom_height] __attribute__((aligned(32)));
      static uint16_t output[bloom_width * bloom_height] __attribute__((aligned(32)));
      static_assert((sizeof (input)) == (sizeof (output)));

      uint32_t transfers = (sizeof (input)) / 32;
      void * texture = (void *)&texture_memory64[texture_memory_alloc.texture.start / 4];
      ch1_dma_transfer(texture, input, transfers);

      gauss_rgb565(input, output);

      ch1_dma_transfer(output, texture, transfers);
    }

    { // ta 1
      assert(!ta_in_use); ta_in_use = 1;

      ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters.start,
                                 texture_memory_alloc.isp_tsp_parameters.end,
                                 texture_memory_alloc.object_list.start,
                                 texture_memory_alloc.object_list.end,
                                 opb_size[1].total(),
                                 ta_alloc[1],
                                 tile_param[1].tile_width(),
                                 tile_param[1].tile_height());
      ta_polygon_converter_writeback(writer2.buf, writer2.offset);
      ta_polygon_converter_transfer(writer2.buf, writer2.offset);

      while (ta_in_use);
    }

    { // core 1
      assert(!core_in_use); core_in_use = 1;

      uint32_t region_array_start = texture_memory_alloc.region_array.start
                                  + tile_param[1].region_array_offset;
      uint32_t framebuffer_start = texture_memory_alloc.framebuffer[framebuffer_ix].start;
      core_start_render2(region_array_start,
                         texture_memory_alloc.isp_tsp_parameters.start,
                         texture_memory_alloc.background[1].start,
                         framebuffer_start,
                         tile_param[1].framebuffer_width);

      while (core_in_use);
    }

    {
      next_frame_ix = framebuffer_ix;
      framebuffer_ix += 1;
      if (framebuffer_ix >= 3) framebuffer_ix = 0;
    }

    while (next_frame == 0);
    next_frame = 0;
  }
}
