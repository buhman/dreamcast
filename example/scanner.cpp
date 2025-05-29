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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "model/scanner/Scanner.H"
#pragma GCC diagnostic pop

#include "model/scanner/Back.data.h"
#include "model/scanner/Bones.data.h"
#include "model/scanner/Dino.data.h"
#include "model/scanner/powervr.data.h"
#include "model/scanner/Scanner.data.h"
#include "model/scanner/Surface.data.h"

struct material {
  void * start;
  uint32_t size;
  uint32_t offset;
  int dimension;
};

const material materials[] = {
  [Mt_DINOSAUR] = // 131072 model/scanner/Dino.data
  {
    .start = (void *)&_binary_model_scanner_Dino_data_start,
    .size = (uint32_t)&_binary_model_scanner_Dino_data_size,
    .offset = 0,
    .dimension = 256,
  },
  [Mt_BONES] = // 131072 model/scanner/Bones.data
  {
    .start = (void *)&_binary_model_scanner_Bones_data_start,
    .size = (uint32_t)&_binary_model_scanner_Bones_data_size,
    .offset = 131072,
    .dimension = 256,
  },
  [Mt_SURFACE] = // 131072 model/scanner/Surface.data
  {
    .start = (void *)&_binary_model_scanner_Surface_data_start,
    .size = (uint32_t)&_binary_model_scanner_Surface_data_size,
    .offset = 262144,
    .dimension = 256,
  },
  [Mt_SCANNER] = // 131072 model/scanner/Scanner.data
  {
    .start = (void *)&_binary_model_scanner_Scanner_data_start,
    .size = (uint32_t)&_binary_model_scanner_Scanner_data_size,
    .offset = 393216,
    .dimension = 256,
  },
  [Mt_LOGO] = // 524288 model/scanner/powervr.data
  {
    .start = (void *)&_binary_model_scanner_powervr_data_start,
    .size = (uint32_t)&_binary_model_scanner_powervr_data_size,
    .offset = 524288,
    .dimension = 512,
  },
  [Mt_BACKGROUND] = // 131072 model/scanner/Back.data
  {
    .start = (void *)&_binary_model_scanner_Back_data_start,
    .size = (uint32_t)&_binary_model_scanner_Back_data_size,
    .offset = 1048576,
    .dimension = 256,
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
                           uint32_t nMaterial,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control::texture
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater_or_equal
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          | tsp_instruction_word::filter_mode::bilinear_filter
                                          ;

  const material& m = materials[nMaterial];

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_shading_instruction::decal
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::texture_u_size::from_int(m.dimension)
                                      | tsp_instruction_word::texture_v_size::from_int(m.dimension)
                                      ;

  uint32_t texture_address = texture_memory_alloc.texture.start + m.offset;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::twiddled
                                | texture_control_word::texture_address(texture_address / 8);

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

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480; // * 2.0;

  return {
    v.x / (1.f * v.z) * dim + 640 / 2.0f,
    v.y / (1.f * v.z) * dim + 480 / 2.0f,
    1 / v.z,
  };
}

static uint32_t random = 0x12345678;

uint32_t xorshift()
{
  uint32_t x = random;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return random = x;
}

#define fsrra(n) (1.0f / (sqrt(n)))

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

const vec3 _light = {10, 5, 10};

void transfer_mesh(ta_parameter_writer& writer, const mat4x4& screen_trans, const Struct_Mesh& mesh)
{
  vec3 position[mesh.nNumVertex];
  static_assert((sizeof (vec3)) == 3 * 4);
  static_assert((offsetof (vec3, x)) == 0);
  float intensity[mesh.nNumVertex];

  vec3 light = normal_multiply(screen_trans, _light);

  for (int i = 0; i < (int)mesh.nNumVertex; i++) {
    vec3 p = *(vec3 *)(&mesh.pVertex[i * 3]);
    vec3 pos = screen_trans * p;

    vec3 n = *(vec3 *)(&mesh.pNormals[i * 3]);
    vec3 normal = normal_multiply(screen_trans, n);

    vec3 light_dir = normalize(light - pos);
    float diffuse = max(dot(normal, light_dir), 0.0f);
    intensity[i] = 0.5 + 0.6 * diffuse;

    position[i] = screen_transform(pos);
  }

  int ix = 0;

  global_polygon_type_1(writer,
                        para_control::list_type::opaque,
                        mesh.nMaterial);

  for (int i = 0; i < (int)mesh.nNumStrips; i++) {
    int strip_length = mesh.pStripLength[i];

    for (int j = 0; j < (strip_length + 2); j++) {
      int vertex_ix = mesh.pStrips[ix];
      vec3 p = position[vertex_ix];
      float li = intensity[vertex_ix];
      vec2 t = *(vec2 *)(&mesh.pUV[vertex_ix * 2]);
      ix += 1;

      bool end_of_strip = (j == (strip_length + 1));
      writer.append<ta_vertex_parameter::polygon_type_7>() =
        ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(end_of_strip),
                                            p.x, p.y, p.z,
                                            t.x, t.y,
                                            li, 0);

    }
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  // opaque list
  {
    for (int i = 0; i < NUM_MESHES; i++) {
      transfer_mesh(writer, screen_trans, Mesh[i]);
    }

    writer.append<ta_global_parameter::end_of_list>() =
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

  float z = -0.5f * r_ + 0.5f * l_;

  return translate((vec3){0, 0, z}) *
    screen_trans *
    rotate_x(x) *
    rotate_y(y);
}

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

void transfer_scene_textures()
{
  for (uint32_t i = 0; i < (sizeof (materials)) / (sizeof (materials[0])); i++) {
    uint32_t offset = texture_memory_alloc.texture.start + materials[i].offset;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);

    transfer_ta_fifo_texture_memory_32byte(dst, materials[i].start, materials[i].size);
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0;
  system.LMMODE1 = 0; // 64-bit

  transfer_scene_textures();
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf1[1024 * 1024 * 2];
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

  transfer_textures();

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

  ta_parameter_writer tl_writer = ta_parameter_writer(ta_parameter_buf1, (sizeof (ta_parameter_buf1)));
  ta_parameter_writer sv_writer = ta_parameter_writer(ta_parameter_buf2, (sizeof (ta_parameter_buf2)));

  video_output::set_mode_vga();

  mat4x4 screen_trans = {
    1, 0, 0, 0,
    0, -1, 0, 0,
    0, 0, 1, 160,
    0, 0, 0, 1,
  };

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    screen_trans = update_analog(screen_trans);

    tl_writer.offset = 0;
    sv_writer.offset = 0;
    transfer_scene(tl_writer, screen_trans);

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
    ta_polygon_converter_writeback(tl_writer.buf, tl_writer.offset);
    ta_polygon_converter_transfer(tl_writer.buf, tl_writer.offset);

    while (next_frame == 0);
    next_frame = 0;
  }
}
