#include <cstdint>

#include "align.hpp"
#include "holly/video_output.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "memorymap.hpp"

#include "sh7091/store_queue.hpp"

#include "texture/poppies/poppies.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t color;
};

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv   ]
  { -0.5f,   0.5f,  0.f, 0.f, 1.f},
  { -0.5f,  -0.5f,  0.f, 0.f, 0.f},
  {  0.5f,   0.5f,  0.f, 1.f, 1.f},
  {  0.5f,  -0.5f,  0.f, 1.f, 0.f},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

void transform_start(uint32_t texture_address, const uint32_t texture_width, uint32_t texture_height)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(texture_width)
                                      | tsp_instruction_word::texture_v_size::from_int(texture_height)
    //| tsp_instruction_word::filter_mode::bilinear_filter
    //| tsp_instruction_word::clamp_uv::uv;
    ;

  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::non_twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void transform_quad_xy(float width, float height)
{
  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;

    x += 0.5;
    y += 0.5;
    x *= width;
    y *= height;

    z = 1.f / (z + 10.f);

    float u = strip_vertices[i].u;
    float v = strip_vertices[i].v;

    bool end_of_strip = i == strip_length - 1;
    *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          u, v,
                                          0, // base_color
                                          0  // offset_color
                                          );
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }
}

void transform_quad_uv(float texture_width, float texture_height,
                       float width, float height)
{
  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;

    x += 0.5;
    y += 0.5;
    x *= texture_width;
    y *= texture_height;
    x += (640 - texture_width) / 2;
    y += (480 - texture_height) / 2;

    z = 1.f / (z + 10.f);

    float u = strip_vertices[i].u;
    float v = strip_vertices[i].v;

    u *= width / texture_width;
    v *= height / texture_height;

    bool end_of_strip = i == strip_length - 1;
    *reinterpret_cast<ta_vertex_parameter::polygon_type_3 *>(store_queue) =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          u, v,
                                          0, // base_color
                                          0  // offset_color
                                          );
    sq_transfer_32byte(ta_fifo_polygon_converter);
  }
}

void transfer_scene_xy(float width, float height)
{
  constexpr uint32_t texture_width = 512;
  constexpr uint32_t texture_height = 256;

  const uint32_t texture_address = texture_memory_alloc::texture.start;
  transform_start(texture_address, texture_width, texture_height);

  transform_quad_xy(width, height);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void transfer_scene_uv(uint32_t texture_address, float width, float height)
{
  constexpr uint32_t texture_width = 512;
  constexpr uint32_t texture_height = 256;

  transform_start(texture_address, texture_width, texture_height);

  transform_quad_uv(texture_width, texture_height,
                    width, height);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void copy_poppies_texture()
{
  auto src = reinterpret_cast<const uint8_t *>(&_binary_texture_poppies_poppies_data_start);
  auto size  = reinterpret_cast<const uint32_t>(&_binary_texture_poppies_poppies_data_size);
  auto texture = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);
  for (uint32_t px = 0; px < size / 3; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    texture[px] = rgb565;
  }
}

constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
			    | ta_alloc_ctrl::tm_opb::no_list
			    | ta_alloc_ctrl::t_opb::no_list
			    | ta_alloc_ctrl::om_opb::no_list
			    | ta_alloc_ctrl::o_opb::_16x4byte;

constexpr struct opb_size opb_size = { .opaque = 16 * 4
				     , .opaque_modifier = 0
				     , .translucent = 0
				     , .translucent_modifier = 0
				     , .punch_through = 0
				     };

void render_xy(const uint32_t width, const uint32_t height)
{
  ta_polygon_converter_init(opb_size.total(),
			    ta_alloc,
			    512 / 32,
			    256 / 32);

  transfer_scene_xy(width, height);

  ta_wait_opaque_list();
}

void render_uv(const uint32_t texture_address, const uint32_t width, const uint32_t height)
{
  ta_polygon_converter_init(opb_size.total(),
			    ta_alloc,
			    640 / 32,
			    480 / 32);

  transfer_scene_uv(texture_address, width, height);

  ta_wait_opaque_list();
}

void main()
{
  video_output::set_mode_vga();
  copy_poppies_texture();

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  uint32_t frame_ix = 0;

  constexpr uint32_t texture_buffer_width = 512;
  constexpr uint32_t texture_buffer_height = 256;

  float theta = 0;
  //int step = 600;
  constexpr float half_degree = 0.01745329f / 2.f;

  while (1) {
    float sin = (__builtin_sinf(theta) + 1.0) / 2.f * (31.f/32.f) + 1.f/32.f;
    //const int steps = 600;
    //float triangle = 4.f / steps * __builtin_fabsf(((step - steps/4) % steps) - (steps / 2.f)) - 1.f;
    //float sin = __builtin_fabsf(triangle) * 15.f/16.f + 1.f/16.f;
    float width = 512.f * sin;
    float height = 256.f * sin;
    //step += 1;

    background_parameter(0xff2288aa);
    region_array2(texture_buffer_width / 32, // width
		  texture_buffer_height / 32, // height
		  opb_size
		  );

    constexpr uint32_t texture_buffer = texture_memory_alloc::texture.start + 512 * 256 * 2;

    render_xy(width, height);
    core_start_render(0x100'0000 | texture_buffer, // 64-bit area
		      texture_buffer_width // frame_width
                      );
    core_wait_end_of_render_video();

    background_parameter(0xff220000);
    region_array2(640 / 32, // width
		  480 / 32, // height
		  opb_size
		  );

    render_uv(texture_buffer, width, height);
    core_start_render(texture_memory_alloc::framebuffer[frame_ix].start,  // 32-bit area
		      640 // frame_width
                      );
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    theta += half_degree;
    frame_ix = (frame_ix + 1) & 1;
  }
}
