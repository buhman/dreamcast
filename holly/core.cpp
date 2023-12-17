#include "../float_uint32.hpp"
#include "core_bits.hpp"
#include "../holly.hpp"
#include "../memorymap.hpp"
#include "../systembus.hpp"
#include "../systembus_bits.hpp"

#include "texture_memory_alloc.hpp"

#include "core.hpp"
#include "background.hpp"
#include "region_array.hpp"

void core_init()
{
  holly.ISP_FEED_CFG   = isp_feed_cfg::cache_size_for_translucency(0x200);

  holly.FPU_SHAD_SCALE = fpu_shad_scale::scale_factor_for_shadows(1);
  holly.FPU_CULL_VAL   = _i(1.f);
  holly.FPU_PERP_VAL   = _i(0.f);
  holly.SPAN_SORT_CFG  = 0; /*span_sort_cfg::span_sort_enable
                              | span_sort_cfg::offset_sort_enable;*/

  holly.FOG_COL_RAM    = fog_col_ram::red(127)
		       | fog_col_ram::green(127)
		       | fog_col_ram::blue(127);

  holly.FOG_COL_VERT   = fog_col_vert::red(127)
		       | fog_col_vert::green(127)
		       | fog_col_vert::blue(127);

  holly.FOG_CLAMP_MIN  = fog_clamp_min::alpha(0)
                       | fog_clamp_min::red(0)
                       | fog_clamp_min::green(0)
                       | fog_clamp_min::blue(0);

  holly.FOG_CLAMP_MAX  = fog_clamp_max::alpha(255)
                       | fog_clamp_max::red(255)
                       | fog_clamp_max::green(255)
                       | fog_clamp_max::blue(255);

  holly.HALF_OFFSET    = half_offset::tsp_texel_sampling_position::center
		       | half_offset::tsp_pixel_sampling_position::center
		       | half_offset::fpu_pixel_sampling_position::center;

  holly.FPU_PARAM_CFG  = fpu_param_cfg::region_header_type::type_2
		       | fpu_param_cfg::tsp_parameter_burst_threshold(31)
		       | fpu_param_cfg::isp_parameter_burst_threshold(31)
		       | fpu_param_cfg::pointer_burst_size(0x15) // must be less than opb size
		       | fpu_param_cfg::pointer_first_burst_size(7); // half of pointer burst size(?)
}

void core_start_render(uint32_t frame_ix, uint32_t num_frames)
{
  holly.REGION_BASE = (offsetof (struct texture_memory_alloc, region_array));
  holly.PARAM_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));

  holly.ISP_BACKGND_T = isp_backgnd_t::tag_address((offsetof (struct texture_memory_alloc, background)) / 4)
                      | isp_backgnd_t::tag_offset(0)
                      | isp_backgnd_t::skip(1);
  holly.ISP_BACKGND_D = _i(1.f/100000.f);

  holly.FB_W_CTRL = 1 << 3 | fb_w_ctrl::fb_packmode::_565_rgb_16bit;
  holly.FB_W_LINESTRIDE = (640 * 2) / 8;

  uint32_t w_fb = ((frame_ix + 0) & num_frames) * 0x00096000;
  holly.FB_W_SOF1 = (offsetof (struct texture_memory_alloc, framebuffer)) + w_fb;

  holly.STARTRENDER = 1;
}

static bool flycast_is_dumb = false;

#include "serial.hpp"

void core_wait_end_of_render_video(uint32_t frame_ix, uint32_t num_frames)
{
  if (!flycast_is_dumb) {
    flycast_is_dumb = true;
  } else {
    while ((system.ISTNRM & ISTNRM__END_OF_RENDER_TSP) == 0);
    system.ISTNRM = ISTNRM__END_OF_RENDER_TSP;
  }
  uint32_t r_fb = ((frame_ix + 1) & num_frames) * 0x00096000;
  holly.FB_R_SOF1 = (offsetof (struct texture_memory_alloc, framebuffer)) + r_fb;
}
