#include <cstdint>

#include "align.hpp"

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
#include "holly/video_output.hpp"
#include "memorymap.hpp"

#include "texture/macaw/macaw.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t color;
};

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv coordinates       ]  [color   ]
  { -0.5f,   0.5f,  0.f, 0.f        , 127.f/128.f, 0xa0ff0000}, // the first two base colors in a
  { -0.5f,  -0.5f,  0.f, 0.f        , 0.f        , 0xa000ff00}, // non-Gouraud triangle strip are ignored
  {  0.5f,   0.5f,  0.f, 127.f/128.f, 127.f/128.f, 0xa00000ff},
  {  0.5f,  -0.5f,  0.f, 127.f/128.f, 0.f        , 0xa0ff00ff},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

void transform(ta_parameter_writer& writer,
               const vertex * strip_vertices,
               const uint32_t strip_length,
               const uint32_t render_pass)
{
  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
					  | isp_tsp_instruction_word::culling_mode::no_culling;

  if (render_pass == 0) {
    // textured
    const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
					  | para_control::list_type::opaque
					  | obj_control::col_type::packed_color
					  | obj_control::texture;

    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
					| tsp_instruction_word::dst_alpha_instr::zero
					| tsp_instruction_word::fog_control::no_fog
					| tsp_instruction_word::texture_u_size::from_int(128)
					| tsp_instruction_word::texture_v_size::from_int(128);

    const uint32_t texture_address = texture_memory_alloc::texture.start;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_565
					| texture_control_word::scan_order::non_twiddled
					| texture_control_word::texture_address(texture_address / 8);

    writer.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  texture_control_word,
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
  } else {
    // untextured
    const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
					  | para_control::list_type::translucent
					  | obj_control::col_type::packed_color
                                          | obj_control::gouraud;

    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
					| tsp_instruction_word::dst_alpha_instr::src_alpha
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::use_alpha;

    const uint32_t texture_control_word = 0;

    writer.append<ta_global_parameter::polygon_type_0>() =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  texture_control_word,
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
  }

  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float x1;

    x1 = x * __builtin_cosf(theta) - z * __builtin_sinf(theta);
    z  = x * __builtin_sinf(theta) + z * __builtin_cosf(theta);
    x  = x1;
    x *= 640.f;
    y *= 640.f;
    x += 320.f;
    y += 240.f;
    z = 1.f / (z + 10.f);

    writer.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
					  x, y, z,
					  strip_vertices[i].u,
					  strip_vertices[i].v,
					  strip_vertices[i].color,
					  0 // offset color
					  );
  }

  writer.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void init_texture_memory(const struct opb_size * opb_size, uint32_t render_passes)
{
  region_array_multipass(640 / 32, // tile_width
			 480 / 32, // tile_height
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array.start,
			 texture_memory_alloc::object_list.start);

  background_parameter(0xff00ff00);
}

uint32_t ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4] __attribute__((aligned(32)));

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

void main()
{
  video_output::set_mode_vga();
  copy_macaw_texture();

  constexpr uint32_t ta_alloc[2] =
    { ta_alloc_ctrl::pt_opb::no_list
    | ta_alloc_ctrl::tm_opb::no_list
    | ta_alloc_ctrl::t_opb::no_list
    | ta_alloc_ctrl::om_opb::no_list
    | ta_alloc_ctrl::o_opb::_16x4byte

    , ta_alloc_ctrl::pt_opb::no_list
    | ta_alloc_ctrl::tm_opb::no_list
    | ta_alloc_ctrl::t_opb::_16x4byte
    | ta_alloc_ctrl::om_opb::no_list
    | ta_alloc_ctrl::o_opb::no_list
    };

  constexpr struct opb_size opb_size[2] = {
    { .opaque = 16 * 4
    , .opaque_modifier = 0
    , .translucent = 0
    , .translucent_modifier = 0
    , .punch_through = 0
    },
    { .opaque = 0
    , .opaque_modifier = 0
    , .translucent = 16 * 4
    , .translucent_modifier = 0
    , .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  constexpr uint32_t render_passes = 2;
  init_texture_memory(opb_size, render_passes);

  constexpr uint32_t tiles = (640 / 32) * (480 / 32);

  uint32_t frame_ix = 0;

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf, (sizeof (ta_parameter_buf)));

  while (true) {
    // first render pass
    ta_polygon_converter_init(opb_size[0].total() + opb_size[1].total(),
			      ta_alloc[0],
			      640 / 32,
			      480 / 32);
    writer.offset = 0;
    transform(writer, strip_vertices, strip_length, 0);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_list();

    // second render pass
    ta_polygon_converter_cont(opb_size[0].total() * tiles,
			      ta_alloc[1]);
    writer.offset = 0;
    transform(writer, strip_vertices, strip_length, 1);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_translucent_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    theta += half_degree;
    frame_ix = (frame_ix + 1) & 1;
  }
}
