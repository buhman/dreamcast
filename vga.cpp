#include <cstdint>

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "holly/holly.hpp"
#include "holly/core_bits.hpp"
#include "aica.hpp"
#include "memorymap.hpp"

#include "vga.hpp"
#include "rgb.hpp"

/*
uint32_t get_cable_type()
{
  // set all pins to input
  sh7091.BSC.PCTRA = 0;

  // get cable type from pins 9 + 8
  return sh7091.BSC.PDTRA & PDTRA__MASK;
}
*/

void vga1()
{
  holly.FB_R_CTRL = holly.FB_R_CTRL & ~fb_r_ctrl::fb_enable;
  holly.VO_CONTROL |= vo_control::blank_video;

  holly.FB_R_SIZE = 0;

  holly.SPG_VBLANK_INT = spg_vblank_int::vblank_out_interrupt_line_number(0x0015)
                       | spg_vblank_int::vblank_in_interrupt_line_number(0x0208);

  holly.SPG_CONTROL = spg_control::sync_direction::output;

  constexpr uint32_t hbstart = 0x0345;
  holly.SPG_HBLANK = spg_hblank::hbend(0x007e) // default
                   | spg_hblank::hbstart(hbstart); // default

  holly.SPG_LOAD = spg_load::vcount(0x020c)  // default
                 | spg_load::hcount(0x0359); // non-default

  holly.SPG_VBLANK = spg_vblank::vbend(0x0028) // non-default
                   | spg_vblank::vbstart(0x0208); // non-default

  holly.SPG_WIDTH = spg_width::eqwidth(0x000f)
		  | spg_width::bpwidth(0x0319)
		  | spg_width::vswidth(0x0003)
                  | spg_width::hswidth(0x003f);

  constexpr uint32_t starty = 0x028;
  holly.VO_STARTX = vo_startx::horizontal_start_position(0x0a8);
  holly.VO_STARTY = vo_starty::vertical_start_position_on_field_2(starty)
                  | vo_starty::vertical_start_position_on_field_1(starty);

  holly.SPG_HBLANK_INT = spg_hblank_int::line_comp_val(hbstart);
}

void vga2()
{
  holly.FB_BURSTCTRL = fb_burstctrl::wr_burst(0x09)
                     | fb_burstctrl::vid_lat(0x3f)
                     | fb_burstctrl::vid_burst(0x39);

  constexpr uint32_t x_size = 640;
  constexpr uint32_t y_size = 480;

  holly.FB_X_CLIP = fb_x_clip::fb_x_clip_max(x_size - 1)
                  | fb_x_clip::fb_x_clip_min(0);

  holly.FB_Y_CLIP = fb_y_clip::fb_y_clip_max(y_size - 1)
                  | fb_y_clip::fb_y_clip_min(0);

  holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                  | fb_r_size::fb_y_size(y_size - 3)
                  | fb_r_size::fb_x_size((x_size * 16) / 32 - 1);

  holly.Y_COEFF = y_coeff::coefficient_1(0x80)
                | y_coeff::coefficient_0_2(0x40);

  // in 6.10 fixed point; 0x0400 is 1x vertical scale
  holly.SCALER_CTL = scaler_ctl::vertical_scale_factor(0x0400);

  holly.FB_W_SOF1 = fb_w_sof1::frame_buffer_write_address_frame_1(0);
  holly.FB_W_SOF2 = fb_w_sof2::frame_buffer_write_address_frame_2(0);
  holly.FB_R_SOF1 = fb_r_sof1::frame_buffer_read_address_frame_1(0);
  holly.FB_R_SOF2 = fb_r_sof2::frame_buffer_read_address_frame_2(0);

  holly.FB_R_CTRL = fb_r_ctrl::vclk_div::pclk_vclk_1
                  | fb_r_ctrl::fb_depth::_0565_rgb_16bit
                  | fb_r_ctrl::fb_enable;

#define DVE_OUTPUT_MODE (&aica[0x2c00])
#define DVE_OUTPUT_MODE__VGA (0b00 << 0)
  *DVE_OUTPUT_MODE = DVE_OUTPUT_MODE__VGA;
#undef DVE_OUTPUT_MODE
#undef DVE_OUTPUT_MODE__VGA
}

void v_sync_in()
{
  while (!spg_status::vsync(holly.SPG_STATUS)) {
    asm volatile ("nop");
  }
  while (spg_status::vsync(holly.SPG_STATUS)) {
    asm volatile ("nop");
  }
}

void v_sync_out()
{
  while (spg_status::vsync(holly.SPG_STATUS)) {
    asm volatile ("nop");
  }
  while (!spg_status::vsync(holly.SPG_STATUS)) {
    asm volatile ("nop");
  }
}

void vga()
{
  holly.SOFTRESET = softreset::sdram_if_soft_reset
		  | softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.TEXT_CONTROL = text_control::stride(3);

  vga1();
  vga2();

  holly.VO_BORDER_COL = vo_border_col::red(0x00)
                      | vo_border_col::green(0xff)
                      | vo_border_col::blue(0x00);
  holly.VO_CONTROL = vo_control::pclk_delay(0x16);

  holly.SDRAM_CFG = 0x15D1C951;
  holly.SDRAM_REFRESH = 0x00000020;

  v_sync_in();

  holly.SOFTRESET = 0;
}

void vga_fill_framebuffer()
{
  volatile uint16_t * vram = reinterpret_cast<volatile uint16_t *>(texture_memory32);
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {
      struct hsv hsv = {(y * 255) / 480, 255, 255};
      struct rgb rgb = hsv_to_rgb(hsv);
      vram[y * 640 + x] = ((rgb.r >> 3) << 11) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 0);
    }
  }
}
