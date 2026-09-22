#include "holly/holly.hpp"
#include "holly/holly_bits.hpp"

#include "holly/core/video_output.hpp"

// this entire file assumes VGA output

namespace holly::core::video_output {
  void framebuffer_init(struct framebuffer const& framebuffer)
  {
    int x_size = framebuffer.px_width;
    int y_size = framebuffer.px_height;

    // write

    holly.FB_X_CLIP = fb_x_clip::fb_x_clip_max(x_size - 1)
                    | fb_x_clip::fb_x_clip_min(0);

    holly.FB_Y_CLIP = fb_y_clip::fb_y_clip_max(y_size - 1)
                    | fb_y_clip::fb_y_clip_min(0);

    // read
    while (spg_status::vsync(holly.SPG_STATUS));
    while (!spg_status::vsync(holly.SPG_STATUS));

    holly.FB_R_SIZE = fb_r_size::fb_modulus(1)
                    | fb_r_size::fb_y_size(y_size - 1)
                    | fb_r_size::fb_x_size((x_size * framebuffer.bytes_per_pixel) / 4 - 1);

    uint32_t fb_depth = (framebuffer.bytes_per_pixel == 4)
      ? fb_r_ctrl::fb_depth::xrgb0888
      : fb_r_ctrl::fb_depth::rgb565;

    holly.FB_R_CTRL = fb_r_ctrl::vclk_div::pclk_vclk_1
                    | fb_depth
                    | fb_r_ctrl::fb_enable;

    uint32_t fb_packmode = (framebuffer.bytes_per_pixel == 4)
      ? fb_w_ctrl::fb_packmode::krgb0888
      : fb_w_ctrl::fb_packmode::rgb565;
    holly.FB_W_CTRL = fb_packmode;

    holly.FB_W_LINESTRIDE = (x_size * framebuffer.bytes_per_pixel) / 8;
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

  void spg_set_mode_640x480()
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
}
