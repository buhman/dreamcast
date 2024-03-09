#include "float_uint32.hpp"
#include "memorymap.hpp"
#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "texture_memory_alloc.hpp"
#include "holly.hpp"
#include "core.hpp"
#include "core_bits.hpp"
#include "background.hpp"
#include "region_array.hpp"

void core_init()
{
  holly.ISP_FEED_CFG   = isp_feed_cfg::cache_size_for_translucency(0x200)
                       | isp_feed_cfg::punch_through_chunk_size(0x040);

  holly.FPU_SHAD_SCALE = fpu_shad_scale::simple_shadow_enable::intensity_volume_mode
                       | fpu_shad_scale::scale_factor_for_shadows(127);
  holly.FPU_CULL_VAL   = _i(1.f);
  holly.FPU_PERP_VAL   = _i(0.f);
  holly.SPAN_SORT_CFG  = span_sort_cfg::span_sort_enable
                       | span_sort_cfg::offset_sort_enable;

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
		       | fpu_param_cfg::pointer_burst_size(15) // must be less than opb size
		       | fpu_param_cfg::pointer_first_burst_size(7); // half of pointer burst size(?)
}

void core_start_render(uint32_t frame_address,
		       uint32_t frame_width,      // in pixels
		       uint32_t frame_size,       // in bytes
		       uint32_t frame_ix, uint32_t num_frames)
{
  holly.REGION_BASE = (offsetof (struct texture_memory_alloc, region_array));
  holly.PARAM_BASE = (offsetof (struct texture_memory_alloc, isp_tsp_parameters));

  holly.ISP_BACKGND_T = isp_backgnd_t::tag_address((offsetof (struct texture_memory_alloc, background)) / 4)
                      | isp_backgnd_t::tag_offset(0)
                      | isp_backgnd_t::skip(1);
  holly.ISP_BACKGND_D = _i(1.f/100000.f);

  holly.FB_W_CTRL = fb_w_ctrl::fb_dither | fb_w_ctrl::fb_packmode::_565_rgb_16bit;
  constexpr uint32_t bytes_per_pixel = 2;
  holly.FB_W_LINESTRIDE = (frame_width * bytes_per_pixel) / 8;

  const uint32_t w_fb = (frame_ix & num_frames) * frame_size;
  holly.FB_W_SOF1 = frame_address + w_fb;

  holly.STARTRENDER = 1;
}

void core_start_render(uint32_t frame_ix, uint32_t num_frames)
{
  core_start_render((offsetof (struct texture_memory_alloc, framebuffer)),
		    640,        // frame_width
		    0x00096000, // frame_size
		    frame_ix, num_frames);
}

void core_wait_end_of_render_video()
{
  /*
    "Furthermore, it is strongly recommended that the End of ISP and End of Video interrupts
    be cleared at the same time in order to make debugging easier when an error occurs."
  */
  while ((system.ISTNRM & istnrm::end_of_render_tsp) == 0);
  system.ISTNRM = istnrm::end_of_render_tsp
		| istnrm::end_of_render_isp
		| istnrm::end_of_render_video;
}

void core_flip(uint32_t frame_ix, uint32_t num_frames)
{
  uint32_t r_fb = (frame_ix & num_frames) * 0x00096000;
  holly.FB_R_SOF1 = (offsetof (struct texture_memory_alloc, framebuffer)) + r_fb;
}
