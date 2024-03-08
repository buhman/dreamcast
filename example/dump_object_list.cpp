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

#include "sh7091/serial.hpp"

#include "macaw.hpp"

struct vertex {
  float x;
  float y;
  float z;
};

const struct vertex strip_vertices[3] = {
  // [ position       ]
  { -0.5f,   0.5f,  0.f }, // the first two base colors in a
  { -0.5f,  -0.5f,  0.f }, // non-Gouraud triangle strip are ignored
  {  0.5f,   0.5f,  0.f },
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

const struct vertex quad_verticies[4] = {
  { 200.f,  360.f, 0.1f },
  { 200.f,  120.f, 0.1f },
  { 440.f,  120.f, 0.1f },
  { 440.f,  360.f, 0.1f },
};

uint32_t transform(uint32_t * ta_parameter_buf,
                   const vertex * strip_vertices,
                   const uint32_t strip_length)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);

  const uint32_t parameter_control_word = para_control::para_type::sprite
//const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;

  const uint32_t texture_control_word = 0;

  constexpr uint32_t base_color = 0xffff0000;

  /*
  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;

    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;
    z = 1.f / (z + 10.f);

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_0>() =
      ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          base_color
                                          );
  }
  */

  parameter.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
				isp_tsp_instruction_word,
				tsp_instruction_word,
				texture_control_word,
				base_color,
				0, // offset_color
				0, // data_size_for_sort_dma
				0); // next_address_for_sort_dma

  parameter.append<ta_vertex_parameter::sprite_type_0>() =
    ta_vertex_parameter::sprite_type_0(para_control::para_type::vertex_parameter,
				       quad_verticies[0].x,
				       quad_verticies[0].y,
				       quad_verticies[0].z,
				       quad_verticies[1].x,
				       quad_verticies[1].y,
				       quad_verticies[1].z,
				       quad_verticies[2].x,
				       quad_verticies[2].y,
				       quad_verticies[2].z,
				       quad_verticies[3].x,
				       quad_verticies[3].y);

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  // zeroize
  for (uint32_t i = 0; i < 0x00100000 / 4; i++) {
    mem->object_list[i] = 0xeeeeeeee;
    mem->isp_tsp_parameters[i] = 0xeeeeeeee;
  }

  background_parameter(mem->background, 0xff222200);

  region_array2(mem->region_array,
                (offsetof (struct texture_memory_alloc, object_list)),
                640 / 32, // width
                480 / 32, // height
                opb_size
                );
}

uint32_t _ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4];

union u32_u8 {
  uint32_t u32;
  uint8_t u8[4];
};
static_assert((sizeof (union u32_u8)) == 4);

void dump()
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);


  constexpr uint32_t screen_ol_size = 8 * 4 * (640 / 32) * (480 / 32);
  for (uint32_t i = 0; i < (screen_ol_size + 0x100) / 4; i++) {
    union u32_u8 n;
    n.u32 = mem->object_list[i];

    if (((i * 4) & 0x1f) == 0)
      serial::character('\n');
    //if (((i * 4) & 0x3f) == 0)
    //  serial::character('\n');

    serial::integer<uint32_t>(n.u32, ' ');
  }

  serial::character('\n');
  serial::character('\n');
  serial::character('\n');

  for (uint32_t i = 0; i < (0x100) / 4; i++) {
    union u32_u8 n;
    n.u32 = mem->isp_tsp_parameters[i];

    if (((i * 4) & 0x1f) == 0)
      serial::character('\n');

    serial::integer<uint32_t>(n.u32, ' ');
  }
}

void main()
{
  video_output::set_mode_vga();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_8x4byte;

  constexpr struct opb_size opb_size = { .opaque = 8 * 4
                                       , .opaque_modifier = 0
                                       , .translucent = 0
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;
  constexpr uint32_t num_frames = 1;

  bool dumped = false;
  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);
    uint32_t ta_parameter_size = transform(ta_parameter_buf, strip_vertices, strip_length);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render(frame_ix, num_frames);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix, num_frames);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix += 1;

    if (frame_ix == 10 && dumped == false) {
      dump();
      dumped = true;
    }
  }
}
