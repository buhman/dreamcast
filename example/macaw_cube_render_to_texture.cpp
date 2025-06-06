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

#include "texture/macaw/macaw.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

vertex cube_faces[][4] = {
  { // front
    { -1.0f, -1.0f,  1.0f, 0.f, 0.f},
    { -1.0f,  1.0f,  1.0f, 0.f, 1.f},
    {  1.0f, -1.0f,  1.0f, 1.f, 0.f},
    {  1.0f,  1.0f,  1.0f, 1.f, 1.f},
  },
  { // back
    { -1.0f, -1.0f, -1.0f, 1.f, 0.f},
    { -1.0f,  1.0f, -1.0f, 1.f, 1.f},
    {  1.0f, -1.0f, -1.0f, 0.f, 0.f},
    {  1.0f,  1.0f, -1.0f, 0.f, 1.f},
  },
  { // right side
    {  1.0f, -1.0f,  1.0f, 0.f, 0.f},
    {  1.0f,  1.0f,  1.0f, 0.f, 1.f},
    {  1.0f, -1.0f, -1.0f, 1.f, 0.f},
    {  1.0f,  1.0f, -1.0f, 1.f, 1.f},
  },
  { // left side
    {  -1.0f, -1.0f,  1.0f, 1.f, 0.f},
    {  -1.0f,  1.0f,  1.0f, 1.f, 1.f},
    {  -1.0f, -1.0f, -1.0f, 0.f, 0.f},
    {  -1.0f,  1.0f, -1.0f, 0.f, 1.f},
  },
};
constexpr uint32_t num_faces = (sizeof (cube_faces)) / (sizeof (cube_faces[0]));

constexpr uint32_t color = 0xffff00ff;

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

void transform(ta_parameter_writer& parameter,
	       const vertex * strip_vertices,
	       const uint32_t strip_length,
	       const uint32_t screen_width,
	       const uint32_t screen_height,
	       const uint32_t texture_address,
	       const uint32_t texture_width)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                      | texture_control_word::scan_order::non_twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  uint32_t tsp_instruction_word = 0;
  switch (texture_width) {
  case 32:
    tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			 | tsp_instruction_word::dst_alpha_instr::zero
			 | tsp_instruction_word::fog_control::no_fog
			 | tsp_instruction_word::texture_u_size::from_int(32)
			 | tsp_instruction_word::texture_v_size::from_int(32)
			 | tsp_instruction_word::use_alpha;
    break;
  case 64:
    tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			 | tsp_instruction_word::dst_alpha_instr::zero
			 | tsp_instruction_word::fog_control::no_fog
			 | tsp_instruction_word::texture_u_size::from_int(64)
			 | tsp_instruction_word::texture_v_size::from_int(64)
			 | tsp_instruction_word::use_alpha;
    break;
  case 128:
    tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			 | tsp_instruction_word::dst_alpha_instr::zero
			 | tsp_instruction_word::fog_control::no_fog
			 | tsp_instruction_word::texture_u_size::from_int(128)
			 | tsp_instruction_word::texture_v_size::from_int(128)
			 | tsp_instruction_word::use_alpha;
    break;
  case 256:
    tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			 | tsp_instruction_word::dst_alpha_instr::zero
			 | tsp_instruction_word::fog_control::no_fog
			 | tsp_instruction_word::texture_u_size::from_int(256)
			 | tsp_instruction_word::texture_v_size::from_int(256)
			 | tsp_instruction_word::use_alpha;
    break;
  case 512:
    tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			 | tsp_instruction_word::dst_alpha_instr::zero
			 | tsp_instruction_word::fog_control::no_fog
			 | tsp_instruction_word::texture_u_size::from_int(512)
			 | tsp_instruction_word::texture_v_size::from_int(512)
			 | tsp_instruction_word::use_alpha;
    break;
  default: break;
  }

  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float t;

    t  = y * __builtin_cosf(theta) - z * __builtin_sinf(theta);
    z  = y * __builtin_sinf(theta) + z * __builtin_cosf(theta);
    y  = t;

    float theta2 = 3.14 * __builtin_sinf(theta / 2);

    t  = x * __builtin_cosf(theta2) - z * __builtin_sinf(theta2);
    z  = x * __builtin_sinf(theta2) + z * __builtin_cosf(theta2);
    x  = t;

    z += 3;

    // perspective
    x = x / z;
    y = y / z;

    x *= static_cast<float>(screen_height) * 0.7;
    y *= static_cast<float>(screen_height) * 0.7;
    x += static_cast<float>(screen_width  / 2);
    y += static_cast<float>(screen_height / 2);

    z = 1 / z;

    parameter.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          strip_vertices[i].u,
                                          strip_vertices[i].v,
                                          0, // base_color
                                          0  // offset_color
                                          );
  }
}

void copy_macaw_texture()
{
  auto src = reinterpret_cast<const uint8_t *>(&_binary_texture_macaw_macaw_data_start);
  auto size  = reinterpret_cast<const uint32_t>(&_binary_texture_macaw_macaw_data_size);
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

void render(const uint32_t width, const uint32_t height,
	    const uint32_t texture_address,
	    const uint32_t texture_width,
	    uint32_t * ta_parameter_buf)
{
  ta_polygon_converter_init(opb_size.total(),
			    ta_alloc,
			    width / 32,
			    height / 32);

  auto parameter = ta_parameter_writer(ta_parameter_buf);
  for (uint32_t i = 0; i < num_faces; i++) {
    transform(parameter, cube_faces[i], 4,
	      width, height,
	      texture_address,
	      texture_width);
  }
  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
  ta_wait_opaque_list();
}

uint32_t _ta_parameter_buf[((32 * (5 * 6 + 1)) + 32) / 4];

void main()
{
  video_output::set_mode_vga();
  copy_macaw_texture();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  uint32_t frame_ix = 0;

  constexpr uint32_t texture_buffer_size = 256;

  while (1) {
    background_parameter(0xff2288aa);
    region_array2(texture_buffer_size / 32, // width
		  texture_buffer_size / 32, // height
		  opb_size
		  );

    constexpr uint32_t macaw = texture_memory_alloc::texture.start;
    constexpr uint32_t texture_buffer = texture_memory_alloc::texture.start + 128 * 128 * 2;

    render(texture_buffer_size, texture_buffer_size,
	   macaw,
	   128,
	   ta_parameter_buf);
    core_start_render(0x100'0000 | texture_buffer, // 64-bit area
		      texture_buffer_size // frame_width
                      );
    core_wait_end_of_render_video();

    background_parameter(0xff220000);
    region_array2(640 / 32, // width
		  480 / 32, // height
		  opb_size
		  );

    render(640, 480,
	   texture_buffer,
	   texture_buffer_size,
	   ta_parameter_buf);
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
