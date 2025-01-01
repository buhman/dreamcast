#include <cstdint>

#include "sh7091/sh7091.hpp"
#include "sh7091/serial.hpp"
#include "aica/aica.hpp"
#include "dve.hpp"
#include "holly.hpp"
#include "holly/core_bits.hpp"

#include "video_output.hpp"
#include "video_output_mode.inc"

namespace video_output {

void set_framebuffer_resolution(const uint32_t x_size, const uint32_t y_size)
{
  holly.Y_COEFF = y_coeff::coefficient_1(0x80)
                | y_coeff::coefficient_0_2(0x40);

  // in 6.10 fixed point; 0x0400 is 1x vertical scale
  holly.SCALER_CTL = scaler_ctl::vertical_scale_factor(0x0400);

  holly.FB_BURSTCTRL = fb_burstctrl::wr_burst(0x09)
                     | fb_burstctrl::vid_lat(0x3f)
                     | fb_burstctrl::vid_burst(0x39);

  holly.FB_X_CLIP = fb_x_clip::fb_x_clip_max(x_size - 1)
                  | fb_x_clip::fb_x_clip_min(0);

  holly.FB_Y_CLIP = fb_y_clip::fb_y_clip_max(y_size - 1)
                  | fb_y_clip::fb_y_clip_min(0);

  holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                  | fb_r_size::fb_y_size(y_size - 3)
                  | fb_r_size::fb_x_size((x_size * 16) / 32 - 1);
}

void set_mode(const struct mode& mode)
{
  holly.FB_R_CTRL = mode.fb_r_ctrl
                  | fb_r_ctrl::fb_depth::_565_rgb_16bit
                  | fb_r_ctrl::fb_enable;

  holly.SPG_LOAD = mode.spg_load;
  holly.SPG_HBLANK = mode.spg_hblank;
  holly.SPG_VBLANK = mode.spg_vblank;
  holly.SPG_WIDTH = mode.spg_width;
  holly.SPG_CONTROL = mode.spg_control;

  holly.VO_STARTX = mode.vo_startx;
  holly.VO_STARTY = mode.vo_starty;
  holly.VO_CONTROL = mode.vo_control;

  holly.SPG_HBLANK_INT = mode.spg_hblank_int;
  holly.SPG_VBLANK_INT = mode.spg_vblank_int;
}

uint32_t get_cable_type()
{
  // set all pins to input
  sh7091.BSC.PCTRA = 0;

  // get cable type from pins 9 + 8
  return sh7091.BSC.PDTRA & pdtra::cable_type::bit_mask;
}

framebuffer_resolution set_mode_by_cable_type(uint32_t cable_type)
{
  switch (cable_type) {
  default: [[fallthrough]];
  case pdtra::cable_type::vga:
    set_mode(vga);
    set_framebuffer_resolution(640, 480);
    aica_sound.common.VREG(vreg::output_mode::vga);
    return {640, 480};
    break;
  case pdtra::cable_type::rgb:
    set_mode(ntsc_ni);
    set_framebuffer_resolution(320, 240);
    aica_sound.common.VREG(vreg::output_mode::rgb);
    return {320, 240};
    break;
  case pdtra::cable_type::cvbs_yc:
    set_mode(ntsc_ni);
    set_framebuffer_resolution(320, 240);
    aica_sound.common.VREG(vreg::output_mode::cvbs_yc);
    return {320, 240};
    break;
  }
}

void set_mode_vga()
{
  set_mode(vga);
  set_framebuffer_resolution(640, 480);
  aica_sound.common.VREG(vreg::output_mode::vga);
}

void reset_sdram()
{
  holly.SOFTRESET = softreset::sdram_if_soft_reset
		  | softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;

  holly.SDRAM_CFG = 0x15D1C951;
  holly.SDRAM_REFRESH = 0x00000020;

  holly.SOFTRESET = 0;
}

}
