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
#include "holly/texture_memory_alloc6.hpp"
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

#include "font/font_bitmap.hpp"
#include "font/verite_8x16/verite_8x16.data.h"
#include "palette.hpp"

#include "math/float_types.hpp"
#include "math/transform.hpp"

#include "interrupt.hpp"
#include "assert.h"

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
  | ta_alloc_ctrl::o_opb::_32x4byte
  ;

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

static volatile int next_frame = 0;
static volatile int framebuffer_ix = 0;
static volatile int next_frame_ix = 0;

static volatile int ta_ix = 0;

static inline void render()
{
  int core_ix = !ta_ix;

  uint32_t region_array_start = texture_memory_alloc.region_array[core_ix].start;
  uint32_t isp_tsp_parameters_start = texture_memory_alloc.isp_tsp_parameters[core_ix].start;

  uint32_t background_start = texture_memory_alloc.background[core_ix].start;
  uint32_t background_offset = background_start - isp_tsp_parameters_start;

  uint32_t bytes_per_pixel = 2;

  holly.REGION_BASE = region_array_start;
  holly.PARAM_BASE = isp_tsp_parameters_start;

  holly.ISP_BACKGND_T
    = isp_backgnd_t::tag_address(background_offset / 4)
    | isp_backgnd_t::tag_offset(0)
    | isp_backgnd_t::skip(1);
  holly.ISP_BACKGND_D = _i(1.f/100000.f);

  holly.FB_W_CTRL = fb_w_ctrl::fb_packmode::_565_rgb_16bit;
  holly.FB_W_LINESTRIDE = (framebuffer_width * bytes_per_pixel) / 8;

  holly.FB_R_CTRL
    = fb_r_ctrl::vclk_div::pclk_vclk_2
    | fb_r_ctrl::fb_strip_buf_en
    | fb_r_ctrl::fb_stripsize(0x02) // 64 lines
    | fb_r_ctrl::fb_depth::_565_rgb_16bit
    | fb_r_ctrl::fb_enable;

  holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[0].start;
  holly.FB_W_SOF1 = texture_memory_alloc.framebuffer[0].start;
  holly.FB_R_SOF2 = texture_memory_alloc.framebuffer[1].start;
  holly.FB_W_SOF2 = texture_memory_alloc.framebuffer[1].start;
  holly.STARTRENDER = 1;
}

static inline void pump_events(uint32_t istnrm)
{
  if (istnrm & istnrm::v_blank_in) {
    system.ISTNRM = istnrm::v_blank_in;

    next_frame = 1;

    render();
  }

  if (istnrm & istnrm::end_of_render_tsp) {
    system.ISTNRM = istnrm::end_of_render_tsp
                  | istnrm::end_of_render_isp
                  | istnrm::end_of_render_video;
  }

  if (istnrm & istnrm::end_of_transferring_opaque_list) {
    system.ISTNRM = istnrm::end_of_transferring_opaque_list;

    ta_ix = !ta_ix;
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
      serial::integer<uint32_t>(isterr);
      if (isterr & 1) {
        system.ISTERR = 1;
      }
      if (isterr & 2) {
        system.ISTERR = 2;
      }

      /*
      holly.SOFTRESET = softreset::pipeline_soft_reset
                      | softreset::ta_soft_reset;
      holly.SOFTRESET = 0;
      */
    }

    pump_events(istnrm);

    sr &= ~sh::sr::imask(15);
    asm volatile ("ldc %0,sr" : : "r" (sr));

    return;
  }

  serial::string("vbr600\n");
  interrupt_exception();
}

void opaque_string(ta_parameter_writer& writer,
                   int x, int y,
                   char * s, int length)
{
  font_bitmap::transform_string(writer,
                                texture_memory_alloc.texture.start,
                                8,  16, // texture
                                8,  16, // glyph
                                16 + x * 8, // position x
                                16 + y * 16, // position y
                                s, length,
                                para_control::list_type::opaque);
}

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480 / 2; // * 2.0;

  return {
    v.x / (1.f * v.z) * dim + 640 / 2.0f,
    v.y / (1.f * v.z) * dim + 480 / 2.0f,
    1 / v.z,
  };
}

void global_polygon_type_0(ta_parameter_writer& writer,
                           uint32_t para_control_obj_control
                           )
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | obj_control::col_type::floating_color
                                        | obj_control::gouraud
                                        | para_control_obj_control
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater_or_equal
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          | tsp_instruction_word::filter_mode::point_sampled
                                          ;

  uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                | tsp_instruction_word::src_alpha_instr::one
                                | tsp_instruction_word::dst_alpha_instr::zero
                                | tsp_instruction_word::texture_shading_instruction::decal
                                ;

  uint32_t texture_control_word = 0;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0,
                                        0);
}

#define fsrra(n) (1.0f / (sqrt(n)))

static inline void render_quad(ta_parameter_writer& writer,
                               vec3 ap,
                               vec3 bp,
                               vec3 cp,
                               vec3 dp,
                               vec3 ac,
                               vec3 bc,
                               vec3 cc,
                               vec3 dc)
{
  if (ap.z < 0 || bp.z < 0 || cp.z < 0 || dp.z < 0)
    return;

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        1, ac.r, ac.g, ac.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        1, bc.r, bc.g, bc.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(false),
                                        dp.x, dp.y, dp.z,
                                        1, dc.r, dc.g, dc.b);

  writer.append<ta_vertex_parameter::polygon_type_1>() =
    ta_vertex_parameter::polygon_type_1(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        1, cc.r, cc.g, cc.b);
}

void transfer_line(ta_parameter_writer& writer, vec3 p1, vec3 p2, vec3 color)
{
  float dy = p2.y - p1.y;
  float dx = p2.x - p1.x;
  float d = fsrra(dx * dx + dy * dy) * 0.7f;
  float dy1 = dy * d;
  float dx1 = dx * d;

  vec3 ap = { p1.x +  dy1, p1.y + -dx1, p1.z }; // -> ^
  vec3 bp = { p1.x + -dy1, p1.y +  dx1, p1.z }; // <- v
  vec3 cp = { p2.x + -dy1, p2.y +  dx1, p2.z }; // <- v
  vec3 dp = { p2.x +  dy1, p2.y + -dx1, p2.z }; // -> ^

  render_quad(writer, ap, bp, cp, dp, color, color, color, color);
}

void transfer_line2(ta_parameter_writer& writer, vec2 p1, vec2 p2, vec3 color)
{
  transfer_line(writer, {p1.x, p1.y, 0.1}, {p2.x, p2.y, 0.1}, color);
}

static int tick = 0;

void transfer_scene(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer,
                        para_control::list_type::opaque);

  transfer_line2(writer, {  1,   1}, {639,   1}, {1, 0, 0});
  transfer_line2(writer, {639,   1}, {639, 479}, {1, 0, 0});
  transfer_line2(writer, {639, 479}, {  1, 479}, {1, 0, 0});
  transfer_line2(writer, {  1, 479}, {  1,   1}, {1, 0, 0});

  vec3 a = {-128, -128, 0.1};
  vec3 b = { 128, -128, 0.1};
  vec3 c = { 128,  128, 0.1};
  vec3 d = {-128,  128, 0.1};

  mat4x4 r = translate((vec3){320, 240, 0}) * rotate_z(((float)tick) * 0.01f);

  render_quad(writer,
              r * a,
              r * b,
              r * c,
              r * d,
              {0, 1, 1},
              {1, 0, 1},
              {1, 1, 0},
              {0, 0, 0});

  opaque_string(writer,
                0, 0,
                (char *)"hello", 5);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void transfer_font()
{
  const uint8_t * src = reinterpret_cast<const uint8_t *>(&_binary_font_verite_8x16_verite_8x16_data_start);
  const uint32_t texture_address = texture_memory_alloc.texture.start;
  font_bitmap::inflate(1,  // pitch
                       8,  // width
                       16, // height
                       texture_address,
                       8,  // texture_width
                       16, // texture_height
                       src);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf1[1024 * 1024];

int main()
{
  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  serial::init(0);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  transfer_font();
  palette_data<3>();

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array[0].start,
                         texture_memory_alloc.object_list[0].start);

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array[1].start,
                         texture_memory_alloc.object_list[1].start);

  background_parameter2(texture_memory_alloc.background[0].start,
                        0xff202040);

  background_parameter2(texture_memory_alloc.background[1].start,
                        0xff202040);

  ta_parameter_writer op_writer = ta_parameter_writer(ta_parameter_buf1, (sizeof (ta_parameter_buf1)));

  video_output::set_mode_vga();

  op_writer.offset = 0;
  transfer_scene(op_writer);
  for (int i = 0; i < 2; i++) {
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[i].start,
                               texture_memory_alloc.isp_tsp_parameters[i].end,
                               texture_memory_alloc.object_list[i].start,
                               texture_memory_alloc.object_list[i].end,
                               opb_size[0].total(),
                               ta_alloc,
                               tile_width,
                               tile_height);
    ta_polygon_converter_writeback(op_writer.buf, op_writer.offset);
    ta_polygon_converter_transfer(op_writer.buf, op_writer.offset);
    ta_wait_opaque_list();
  }

  interrupt_init();

  system.IML6NRM = istnrm::end_of_render_tsp
                 | istnrm::v_blank_in
                 | istnrm::end_of_transferring_opaque_list;

  do_get_condition();
  while (1) {
    maple::dma_wait_complete();
    do_get_condition();

    op_writer.offset = 0;
    transfer_scene(op_writer);
    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta_ix].start,
                               texture_memory_alloc.isp_tsp_parameters[ta_ix].end,
                               texture_memory_alloc.object_list[ta_ix].start,
                               texture_memory_alloc.object_list[ta_ix].end,
                               opb_size[0].total(),
                               ta_alloc,
                               tile_width,
                               tile_height);
    ta_polygon_converter_writeback(op_writer.buf, op_writer.offset);
    ta_polygon_converter_transfer(op_writer.buf, op_writer.offset);

    while (next_frame == 0);
    next_frame = 0;

    tick += 1;
  }
}
