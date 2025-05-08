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

#include "texture/bump/bump.data.h"

#include "assert.h"

constexpr int font_base = 64 + (((0x7f - 0x20) + 1) * 8 * 16 / 2);

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

static ft0::data_transfer::data_format data[4];

static int k1 = 0;
static int k2 = 0;
static int k3 = 0;
static int q = 0;
static int pcw_offset = 0;

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
  | ta_alloc_ctrl::o_opb::_32x4byte
  ;

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

  if (istnrm & istnrm::end_of_transferring_translucent_list) {
    system.ISTNRM = istnrm::end_of_transferring_translucent_list;

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

const vec3 position[] = {
  { -0.5f,  -0.5f,  0.1f},
  { -0.5f,   0.5f,  0.1f},
  {  0.5f,   0.5f,  0.1f},
  {  0.5f,  -0.5f,  0.1f},
};

const vec2 texture[] = {
  {0.f, 0.f},
  {0.f, 1.f},
  {1.f, 1.f},
  {1.f, 0.f},
};

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control,
                           uint32_t tsp_instruction,
                           uint32_t texture_control_word,
                           const float a = 1.0f,
                           const float r = 1.0f,
                           const float g = 1.0f,
                           const float b = 1.0f
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::packed_color
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_shading_instruction::decal
                                      | tsp_instruction
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

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               vec2 at,
                               vec2 bt,
                               vec2 ct,
                               vec2 dt,
                               uint32_t base_color,
                               uint32_t offset_color)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        base_color,
                                        offset_color);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        base_color,
                                        offset_color);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        dt.x, dt.y,
                                        base_color,
                                        offset_color);

  writer.append<ta_vertex_parameter::polygon_type_3>() =
    ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        base_color,
                                        offset_color);
}

vec3 transform(vec3 v)
{
  return {
    v.x * 300 + 320,
    v.y * 300 + 240,
    v.z
  };
}

void transfer_mesh(ta_parameter_writer& writer)
{
  uint32_t control = para_control::list_type::translucent
                   | obj_control::texture
    | (pcw_offset ? obj_control::offset : 0)
    ;
  uint32_t tsp_instruction_word = 0
                                | tsp_instruction_word::src_alpha_instr::zero
                                | tsp_instruction_word::dst_alpha_instr::src_alpha
                                | tsp_instruction_word::texture_u_size::from_int(512)
                                | tsp_instruction_word::texture_v_size::from_int(512)
                                ;
  uint32_t texture_address = texture_memory_alloc.texture.start + font_base;
  uint32_t texture_control_word = texture_control_word::pixel_format::bump_map
                                      | texture_control_word::scan_order::non_twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  global_polygon_type_0(writer,
                        control,
                        tsp_instruction_word,
                        texture_control_word);


  uint32_t base_color = 0;
  uint32_t offset_color = 0
    | (k1 << 24)
    | (k2 << 16)
    | (k3 << 8)
    | (q << 0);

  render_quad(writer,
              transform(position[0]),
              transform(position[1]),
              transform(position[2]),
              transform(position[3]),
              texture[0],
              texture[1],
              texture[2],
              texture[3],
              base_color,
              offset_color);
}


void opaque_string(ta_parameter_writer& writer,
                   int x, int y,
                   char * s, int length)
{
  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start + 64,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + x * 8, // position x
                                16 + y * 16, // position y
                                s, length,
                                para_control::list_type::opaque);
}

void render_k1k2k3q(ta_parameter_writer& writer)
{
  char __attribute__((aligned(4))) s[64];
  for (uint32_t i = 0; i < (sizeof (s)) / 4; i++)
    reinterpret_cast<uint32_t *>(s)[i] = 0x20202020;

  int offset;
  offset = unparse_base10(s, k1, 7, ' ');
  s[0] = 'k';
  s[1] = '1';
  s[2] = ':';
  opaque_string(writer, 1, 1, s, offset);

  offset = unparse_base10(s, k2, 7, ' ');
  s[0] = 'k';
  s[1] = '2';
  s[2] = ':';
  opaque_string(writer, 1, 2, s, offset);

  offset = unparse_base10(s, k3, 7, ' ');
  s[0] = 'k';
  s[1] = '3';
  s[2] = ':';
  opaque_string(writer, 1, 3, s, offset);

  offset = unparse_base10(s, q, 7, ' ');
  s[0] = ' ';
  s[1] = 'q';
  s[2] = ':';
  opaque_string(writer, 1, 4, s, offset);

  offset = unparse_base10(&s[1], pcw_offset, 7, ' ');
  s[0] = 'o';
  s[1] = 'f';
  s[2] = 'f';
  s[3] = ':';
  opaque_string(writer, 0, 5, s, offset + 1);
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& trans, int animation_tick)
{
  // opaque list
  if (1) {
    render_k1k2k3q(writer);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
  // translucent list
  if (1) {
    transfer_mesh(writer);

    writer.append<ta_global_parameter::end_of_list>() =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  }
}

void update_analog(mat4x4& screen)
{
  int ra = ft0::data_transfer::digital_button::ra(data[0].digital_button) == 0;
  int la = ft0::data_transfer::digital_button::la(data[0].digital_button) == 0;
  int da = ft0::data_transfer::digital_button::da(data[0].digital_button) == 0;
  int ua = ft0::data_transfer::digital_button::ua(data[0].digital_button) == 0;

  int db_x = ft0::data_transfer::digital_button::x(data[0].digital_button) == 0;
  int db_y = ft0::data_transfer::digital_button::y(data[0].digital_button) == 0;
  int db_a = ft0::data_transfer::digital_button::a(data[0].digital_button) == 0;
  int db_b = ft0::data_transfer::digital_button::b(data[0].digital_button) == 0;

  int start = ft0::data_transfer::digital_button::start(data[0].digital_button) == 0;
  static int last_start = 0;

  if (db_x) {
    k1 -= 1;
    if (k1 < 0) k1 = 0;
  }
  if (db_y) {
    k1 += 1;
    if (k1 > 255) k1 = 255;
  }
  if (db_a) {
    k2 -= 1;
    if (k2 < 0) k2 = 0;
  }
  if (db_b) {
    k2 += 1;
    if (k2 > 255) k2 = 255;
  }

  if (da) {
    k3 -= 1;
    if (k3 < 0) k3 = 0;
  }
  if (ua) {
    k3 += 1;
    if (k3 > 255) k3 = 255;
  }
  if (la) {
    q -= 1;
    if (q < 0) q = 0;
  }
  if (ra) {
    q += 1;
    if (q > 255) q = 255;
  }

  if (start && (start != last_start)) {
    pcw_offset = !pcw_offset;
  }
  last_start = start;
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

void transfer_font()
{
  const uint8_t * src = reinterpret_cast<const uint8_t *>(&_binary_font_verite_8x16_verite_8x16_data_start);
  const uint32_t texture_address = texture_memory_alloc.texture.start + 64;
  font_bitmap::inflate(1,  // pitch
                       8,  // width
                       16, // height
                       texture_address,
                       8,  // texture_width
                       16, // texture_height
                       src);
}

void transfer_bump_texture()
{
  uint32_t offset = texture_memory_alloc.texture.start + font_base;
  void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
  void * src = reinterpret_cast<void *>(&_binary_texture_bump_bump_data_start);
  uint32_t size = reinterpret_cast<uint32_t>(&_binary_texture_bump_bump_data_size);
  printf("src %x dst %x size %x\n", (uint32_t)src, (uint32_t)dst, (uint32_t)size);
  transfer_ta_fifo_texture_memory_32byte(dst, src, size);
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  transfer_font();
  transfer_bump_texture();
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
  transfer_textures();
  palette_data<3>();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::parameter_selection_volume_mode;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xffffffff);

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

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_translucent_list;

  while (1) {
    maple::dma_wait_complete();
    do_get_condition();
    writer.offset = 0;

    update_analog(screen_trans);
    transfer_scene(writer, screen_trans * model_trans, animation_tick);

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
