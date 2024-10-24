namespace video_output {
  const struct mode vga = {
    .fb_r_ctrl      = fb_r_ctrl::vclk_div::pclk_vclk_1
                    ,
    .spg_load       = spg_load::vcount(0x20c)
                    | spg_load::hcount(0x359)
                    ,
    .spg_hblank     = spg_hblank::hbend(0x07e)
                    | spg_hblank::hbstart(0x345)
                    ,
    .spg_vblank     = spg_vblank::vbend(0x028)
                    | spg_vblank::vbstart(0x208)
                    ,
    .spg_width      = spg_width::eqwidth(0x00f)
                    | spg_width::bpwidth(0x319)
                    | spg_width::vswidth(0x3)
                    | spg_width::hswidth(0x3f)
                    ,
    .spg_control    = spg_control::sync_direction::output
                    ,
    .vo_startx      = vo_startx::horizontal_start_position(0x0a8)
                    ,
    .vo_starty      = vo_starty::vertical_start_position_on_field_2(0x028)
                    | vo_starty::vertical_start_position_on_field_1(0x028)
                    ,
    .vo_control     = vo_control::pclk_delay(0x16)
                    ,
    .spg_hblank_int = spg_hblank_int::line_comp_val(0x345)
                    ,
    .spg_vblank_int = spg_vblank_int::vblank_out_interrupt_line_number(0x015)
                    | spg_vblank_int::vblank_in_interrupt_line_number(0x208)
                    ,
  };
  const struct mode ntsc_ni = {
    .fb_r_ctrl      = fb_r_ctrl::vclk_div::pclk_vclk_2
                    ,
    .spg_load       = spg_load::vcount(0x106)
                    | spg_load::hcount(0x359)
                    ,
    .spg_hblank     = spg_hblank::hbend(0x07e)
                    | spg_hblank::hbstart(0x345)
                    ,
    .spg_vblank     = spg_vblank::vbend(0x012)
                    | spg_vblank::vbstart(0x102)
                    ,
    .spg_width      = spg_width::eqwidth(0x00f)
                    | spg_width::bpwidth(0x319)
                    | spg_width::vswidth(0x3)
                    | spg_width::hswidth(0x3f)
                    ,
    .spg_control    = spg_control::sync_direction::output
                    | spg_control::ntsc
                    ,
    .vo_startx      = vo_startx::horizontal_start_position(0x0a4)
                    ,
    .vo_starty      = vo_starty::vertical_start_position_on_field_2(0x012)
                    | vo_starty::vertical_start_position_on_field_1(0x011)
                    ,
    .vo_control     = vo_control::pclk_delay(0x16)
                    | vo_control::pixel_double
                    ,
    .spg_hblank_int = spg_hblank_int::line_comp_val(0x345)
                    ,
    .spg_vblank_int = spg_vblank_int::vblank_out_interrupt_line_number(0x015)
                    | spg_vblank_int::vblank_in_interrupt_line_number(0x102)
                    ,
  };
  const struct mode ntsc_i = {
    .fb_r_ctrl      = fb_r_ctrl::vclk_div::pclk_vclk_2
                    ,
    .spg_load       = spg_load::vcount(0x20c)
                    | spg_load::hcount(0x359)
                    ,
    .spg_hblank     = spg_hblank::hbend(0x07e)
                    | spg_hblank::hbstart(0x345)
                    ,
    .spg_vblank     = spg_vblank::vbend(0x024)
                    | spg_vblank::vbstart(0x204)
                    ,
    .spg_width      = spg_width::eqwidth(0x01f)
                    | spg_width::bpwidth(0x16c)
                    | spg_width::vswidth(0x6)
                    | spg_width::hswidth(0x3f)
                    ,
    .spg_control    = spg_control::sync_direction::output
                    | spg_control::ntsc
                    | spg_control::interlace
                    ,
    .vo_startx      = vo_startx::horizontal_start_position(0x0a4)
                    ,
    .vo_starty      = vo_starty::vertical_start_position_on_field_2(0x012)
                    | vo_starty::vertical_start_position_on_field_1(0x012)
                    ,
    .vo_control     = vo_control::pclk_delay(0x16)
                    | vo_control::pixel_double
                    ,
    .spg_hblank_int = spg_hblank_int::line_comp_val(0x345)
                    ,
    .spg_vblank_int = spg_vblank_int::vblank_out_interrupt_line_number(0x015)
                    | spg_vblank_int::vblank_in_interrupt_line_number(0x102)
                    ,
  };
  const struct mode pal_ni = {
    .fb_r_ctrl      = fb_r_ctrl::vclk_div::pclk_vclk_2
                    ,
    .spg_load       = spg_load::vcount(0x138)
                    | spg_load::hcount(0x35f)
                    ,
    .spg_hblank     = spg_hblank::hbend(0x08d)
                    | spg_hblank::hbstart(0x34b)
                    ,
    .spg_vblank     = spg_vblank::vbend(0x016)
                    | spg_vblank::vbstart(0x134)
                    ,
    .spg_width      = spg_width::eqwidth(0x0f)
                    | spg_width::bpwidth(0x31f)
                    | spg_width::vswidth(0x3)
                    | spg_width::hswidth(0x3f)
                    ,
    .spg_control    = spg_control::sync_direction::output
                    | spg_control::pal
                    ,
    .vo_startx      = vo_startx::horizontal_start_position(0x0ae)
                    ,
    .vo_starty      = vo_starty::vertical_start_position_on_field_2(0x02e)
                    | vo_starty::vertical_start_position_on_field_1(0x02e)
                    ,
    .vo_control     = vo_control::pclk_delay(0x16)
                    | vo_control::pixel_double
                    ,
    .spg_hblank_int = spg_hblank_int::line_comp_val(0x34b)
                    ,
    .spg_vblank_int = spg_vblank_int::vblank_out_interrupt_line_number(0x015)
                    | spg_vblank_int::vblank_in_interrupt_line_number(0x134)
                    ,
  };
  const struct mode pal_i = {
    .fb_r_ctrl      = fb_r_ctrl::vclk_div::pclk_vclk_2
                    ,
    .spg_load       = spg_load::vcount(0x270)
                    | spg_load::hcount(0x35f)
                    ,
    .spg_hblank     = spg_hblank::hbend(0x08d)
                    | spg_hblank::hbstart(0x34b)
                    ,
    .spg_vblank     = spg_vblank::vbend(0x02c)
                    | spg_vblank::vbstart(0x26c)
                    ,
    .spg_width      = spg_width::eqwidth(0x01f)
                    | spg_width::bpwidth(0x16a)
                    | spg_width::vswidth(0x5)
                    | spg_width::hswidth(0x3f)
                    ,
    .spg_control    = spg_control::sync_direction::output
                    | spg_control::pal
                    | spg_control::interlace
                    ,
    .vo_startx      = vo_startx::horizontal_start_position(0x0ae)
                    ,
    .vo_starty      = vo_starty::vertical_start_position_on_field_2(0x02e)
                    | vo_starty::vertical_start_position_on_field_1(0x02d)
                    ,
    .vo_control     = vo_control::pclk_delay(0x16)
                    | vo_control::pixel_double
                    ,
    .spg_hblank_int = spg_hblank_int::line_comp_val(0x34b)
                    ,
    .spg_vblank_int = spg_vblank_int::vblank_out_interrupt_line_number(0x015)
                    | spg_vblank_int::vblank_in_interrupt_line_number(0x134)
                    ,
  };
}
