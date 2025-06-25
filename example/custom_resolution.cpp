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
#include "holly/texture_memory_alloc8.hpp"
#include "memorymap.hpp"
#include "color_format.hpp"

#include "math/float_types.hpp"

#include "sh7091/serial.hpp"
#include "printf/printf.h"

constexpr int div(int n, int d)
{
  return (n + 32 - 1) / 32;
}

const int framebuffer_width = 720;
const int framebuffer_height = 480;
const int tile_width = div(framebuffer_width, 32);
const int tile_height = div(framebuffer_height, 32);
const int bytes_per_pixel = 2;

const uint32_t ta_alloc
  = ta_alloc_ctrl::pt_opb::no_list
  | ta_alloc_ctrl::tm_opb::no_list
  | ta_alloc_ctrl::t_opb::no_list
  | ta_alloc_ctrl::om_opb::no_list
  | ta_alloc_ctrl::o_opb::_16x4byte;

const int ta_cont_count = 1;
const struct opb_size opb_size[ta_cont_count] = {
  {
    .opaque = 16 * 4,
    .opaque_modifier = 0,
    .translucent = 0,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};

void framebuffer_init()
{
  int x_size = framebuffer_width;
  int y_size = framebuffer_height;

  // write

  holly.FB_X_CLIP = fb_x_clip::fb_x_clip_max(x_size - 1)
                  | fb_x_clip::fb_x_clip_min(0);

  holly.FB_Y_CLIP = fb_y_clip::fb_y_clip_max(y_size - 1)
                  | fb_y_clip::fb_y_clip_min(0);

  // read

  holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                  | fb_r_size::fb_y_size(y_size - 1)
                  | fb_r_size::fb_x_size((x_size * bytes_per_pixel) / 4 - 1);
}

void scaler_init()
{
  holly.Y_COEFF = y_coeff::coefficient_1(0x80)
                | y_coeff::coefficient_0_2(0x40);

  // in 6.10 fixed point; 0x0400 is 1x vertical scale
  holly.SCALER_CTL = scaler_ctl::vertical_scale_factor(0x0400);

  holly.FB_BURSTCTRL = fb_burstctrl::wr_burst(0x09)
                     | fb_burstctrl::vid_lat(0x3f)
                     | fb_burstctrl::vid_burst(0x39);
}

void spg_set_mode()
{
  holly.SPG_CONTROL
    = spg_control::sync_direction::output;

  holly.SPG_LOAD
    = spg_load::vcount(525 - 1)    // number of lines per field
    | spg_load::hcount(858 - 1);   // number of video clock cycles per line

  holly.SPG_HBLANK
    = spg_hblank::hbend(117)       // H Blank ending position
    | spg_hblank::hbstart(837);    // H Blank starting position

  holly.SPG_VBLANK
    = spg_vblank::vbend(40)        // V Blank ending position
    | spg_vblank::vbstart(520);    // V Blank starting position

  holly.SPG_WIDTH
    = spg_width::eqwidth(16 - 1)   // Specify the equivalent pulse width (number of video clock cycles - 1)
    | spg_width::bpwidth(794 - 1)  // Specify the broad pulse width (number of video clock cycles - 1)
    | spg_width::vswidth(3)        // V Sync width (number of lines)
    | spg_width::hswidth(64 - 1);  // H Sync width (number of video clock cycles - 1)

  holly.VO_STARTX
    = vo_startx::horizontal_start_position(117);

  holly.VO_STARTY
    = vo_starty::vertical_start_position_on_field_2(40)
    | vo_starty::vertical_start_position_on_field_1(40);

  holly.VO_CONTROL
    = vo_control::pclk_delay(22);

  holly.SPG_HBLANK_INT
    = spg_hblank_int::line_comp_val(837);

  holly.SPG_VBLANK_INT
    = spg_vblank_int::vblank_out_interrupt_line_number(21)
    | spg_vblank_int::vblank_in_interrupt_line_number(520);
}

void core_param_init()
{
  uint32_t region_array_start = texture_memory_alloc.region_array.start;
  uint32_t isp_tsp_parameters_start = texture_memory_alloc.isp_tsp_parameters.start;
  uint32_t background_start = texture_memory_alloc.framebuffer[0].start;

  holly.REGION_BASE = region_array_start;
  holly.PARAM_BASE = isp_tsp_parameters_start;

  uint32_t background_offset = background_start - isp_tsp_parameters_start;

  holly.ISP_BACKGND_T
    = isp_backgnd_t::tag_address(background_offset / 4)
    | isp_backgnd_t::tag_offset(0)
    | isp_backgnd_t::skip(1);
  holly.ISP_BACKGND_D = _i(1.f/100000.f);

  holly.FB_W_CTRL
    = fb_w_ctrl::fb_dither
    | fb_w_ctrl::fb_packmode::_565_rgb_16bit;

  holly.FB_W_LINESTRIDE = (framebuffer_width * bytes_per_pixel) / 8;
}

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
  int r5 = r & 31;
  int g6 = g & 63;
  int b5 = b & 31;
  return (r5 << 11) | (g6 << 5) | (b5 << 0);
}

void test_pattern()
{
  uint16_t * framebuffer = (uint16_t *)&texture_memory32[texture_memory_alloc.framebuffer[0].start / 4];

  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 720; x++) {
      if (x == 0 || y == 0 || y == 479)
        framebuffer[y * 720 + x] = rgb565(31, 0, 0);
      else if (x == 719)
        framebuffer[y * 720 + x] = rgb565(0, 0, 31);
      else
        framebuffer[y * 720 + x] = rgb565(0, 31 * (x & 1), 0);
    }
  }

  holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[0].start;
}

struct vertex {
  vec3 position;
  unsigned int base_color;
};

// screen space coordinates
const struct vertex triangle_vertices[] = {
  { {320.000f + 40,   50.f, 0.1f}, 0xffff0000 },
  { {539.393f + 40,  430.f, 0.1f}, 0xff00ff00 },
  { {100.607f + 40,  430.f, 0.1f}, 0xff0000ff },
};

static inline void global_polygon_type_0(ta_parameter_writer& writer)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::gouraud;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  writer.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        0,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
}

static inline void vertex_polygon_type_0(ta_parameter_writer& writer, const vertex& v, bool end_of_strip)
{
  writer.append<ta_vertex_parameter::polygon_type_0>() =
    ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
                                        v.position.x, v.position.y, v.position.z,
                                        v.base_color);
}

void transfer_triangle(ta_parameter_writer& writer)
{
  global_polygon_type_0(writer);

  vertex_polygon_type_0(writer, triangle_vertices[0], false);
  vertex_polygon_type_0(writer, triangle_vertices[1], false);
  vertex_polygon_type_0(writer, triangle_vertices[2], true);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void main()
{
  serial::init(0);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  holly.FB_R_CTRL = fb_r_ctrl::vclk_div::pclk_vclk_1
                  | fb_r_ctrl::fb_depth::_565_rgb_16bit
                  | fb_r_ctrl::fb_enable;

  scaler_init();
  spg_set_mode();
  framebuffer_init();
  core_init();

  holly.VO_BORDER_COL = 0x00ff00ff;

  region_array_multipass(tile_width,
                         tile_height,
                         opb_size,
                         ta_cont_count,
                         texture_memory_alloc.region_array.start,
                         texture_memory_alloc.object_list.start);

  background_parameter2(texture_memory_alloc.framebuffer[0].start,
                        0xff800080);

  core_param_init();

  static uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024 * 1];
  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  transfer_triangle(writer);

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
  ta_wait_opaque_list();

  //int framebuffer_size = framebuffer_width * framebuffer_height * bytes_per_pixel;
  //int framebuffer_address = (0x10000000 - (framebuffer_size / 2));
  //int framebuffer_address = texture_memory_alloc.framebuffer[1].start;
  //holly.FB_W_SOF1 = framebuffer_address & 0x1ffffff;
  //holly.STARTRENDER = 1;

  int framebuffer_address = texture_memory_alloc.framebuffer[0].start;
  test_pattern();

  holly.FB_R_SOF1 = framebuffer_address & 0x0ffffff;

  printf("fb_w_sof1 0x%08x\n", holly.FB_W_SOF1);
  printf("fb_r_sof1 0x%08x\n", holly.FB_R_SOF1);

  while (1) {
  }
}
