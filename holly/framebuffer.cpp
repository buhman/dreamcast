#include "holly/holly.hpp"
#include "holly/core_bits.hpp"

#include "framebuffer.hpp"

namespace framebuffer {

  struct framebuffer framebuffer;

  void init(int px_width,
            int px_height,
            uint32_t framebuffer_start)
  {
    framebuffer.px_width = px_width;
    framebuffer.px_height = px_height;

    int x_size = framebuffer.px_width;
    int y_size = framebuffer.px_height;

    // write

    holly.FB_X_CLIP
      = fb_x_clip::fb_x_clip_max(x_size - 1)
      | fb_x_clip::fb_x_clip_min(0);

    holly.FB_Y_CLIP
      = fb_y_clip::fb_y_clip_max(y_size - 1)
      | fb_y_clip::fb_y_clip_min(0);

    holly.FB_R_SIZE
      = fb_r_size::fb_modulus(1)
      | fb_r_size::fb_y_size(y_size - 1)
      | fb_r_size::fb_x_size((x_size * bytes_per_pixel) / 4 - 1);

    holly.FB_R_SOF1
      = framebuffer_start;

    uint32_t vclk_div = (px_width >= 640) ? fb_r_ctrl::vclk_div::pclk_vclk_1
                                          : fb_r_ctrl::vclk_div::pclk_vclk_2;
    holly.FB_R_CTRL
      = vclk_div
      | fb_r_ctrl::fb_depth::_565_rgb_16bit
      | fb_r_ctrl::fb_enable;
  }

  void scaler_init()
  {
    holly.Y_COEFF
      = y_coeff::coefficient_1(0x80)
      | y_coeff::coefficient_0_2(0x40);

    // in 6.10 fixed point; 0x0400 is 1x vertical scale
    holly.SCALER_CTL = scaler_ctl::vertical_scale_factor(0x0400);

    holly.FB_BURSTCTRL
      = fb_burstctrl::wr_burst(0x09)
      | fb_burstctrl::vid_lat(0x3f)
      | fb_burstctrl::vid_burst(0x39);
  }

  void spg_set_mode_720x480_vga()
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

  void spg_set_mode_640x480_vga()
  {
    holly.SPG_CONTROL
      = spg_control::sync_direction::output;

    holly.SPG_LOAD
      = spg_load::vcount(525 - 1)    // number of lines per field
      | spg_load::hcount(858 - 1);   // number of video clock cycles per line

    holly.SPG_HBLANK
      = spg_hblank::hbend(126)       // H Blank ending position
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
      = vo_startx::horizontal_start_position(168);

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

  void spg_set_mode_320x240_ntsc_ni()
  {
    holly.SPG_CONTROL
      = spg_control::sync_direction::output
      | spg_control::ntsc;

    holly.SPG_LOAD
      = spg_load::vcount(263 - 1)
      | spg_load::hcount(858 - 1);

    holly.SPG_HBLANK
      = spg_hblank::hbend(126)
      | spg_hblank::hbstart(837);

    holly.SPG_VBLANK
      = spg_vblank::vbend(18)
      | spg_vblank::vbstart(258);

    holly.SPG_WIDTH
      = spg_width::eqwidth(16 - 1)
      | spg_width::bpwidth(794 - 1)
      | spg_width::vswidth(3)
      | spg_width::hswidth(64 - 1);

    holly.VO_STARTX
      = vo_startx::horizontal_start_position(164);

    holly.VO_STARTY
      = vo_starty::vertical_start_position_on_field_2(18)
      | vo_starty::vertical_start_position_on_field_1(17);

    holly.VO_CONTROL
      = vo_control::pclk_delay(22)
      | vo_control::pixel_double;

    holly.SPG_HBLANK_INT
      = spg_hblank_int::line_comp_val(837);

    holly.SPG_VBLANK_INT
      = spg_vblank_int::vblank_out_interrupt_line_number(21)
      | spg_vblank_int::vblank_in_interrupt_line_number(258);
  }
}
