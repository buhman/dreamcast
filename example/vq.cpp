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
#include "twiddle.hpp"

#include "vq.hpp"

struct vertex {
  float x;
  float y;
  float u;
  float v;
};

const struct vertex strip_vertices[4] = {
  // [ position ]  [ uv   ]
  {   0.5f, -0.5f, 0.f, 0.f},
  {   0.5f,  0.5f, 1.f, 0.f},
  {  -0.5f, -0.5f, 0.f, 1.f},
  {  -0.5f,  0.5f, 1.f, 1.f},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

uint32_t transform(uint32_t * ta_parameter_buf,
                   const vertex * strip_vertices,
                   const uint32_t strip_length)
{
  auto parameter = ta_parameter_writer(ta_parameter_buf);
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(128)
                                      | tsp_instruction_word::texture_v_size::from_int(128);

  const uint32_t texture_address = texture_memory_alloc::texture.start;
  const uint32_t texture_control_word = texture_control_word::vq_compressed
                                      | texture_control_word::pixel_format::_565
    | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

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
    float y = -strip_vertices[i].y;
    float z = 0.1f;

    x *= 256.f;
    y *= 256.f;
    x += 320.f;
    y += 240.f;

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          strip_vertices[i].u,
                                          strip_vertices[i].v,
                                          0,
                                          0 // offset_color
                                          );
  }

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff00ff00);
}

void copy_vq_texture()
{
  uint32_t buf[(256 * 4 * 2 + (64 * 64)) / 4];

  //auto texture16 = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);
  auto texture16 = reinterpret_cast<volatile uint16_t *>(buf);

  uint16_t * _codes = &codes[0][0];
  for (int i = 0; i < 256 * 4; i++) {
    texture16[i] = _codes[i];
  }

  //auto texture8 = reinterpret_cast<volatile uint8_t *>(&texture16[256 * 4]);
  auto texture8 = reinterpret_cast<volatile uint8_t *>(&buf[256 * 4 * 2 / 4]);
  twiddle::texture(texture8, indexes, 64, 64);

  volatile uint32_t * tex = &texture_memory64[texture_memory_alloc::texture.start / 4];
  for (uint32_t i = 0; i < (sizeof (buf)) / 4; i++) {
    tex[i] = buf[i];
  }
}

uint32_t _ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4];

void main()
{
  video_output::set_mode_vga();
  copy_vq_texture();

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

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);
    uint32_t ta_parameter_size = transform(ta_parameter_buf, strip_vertices, strip_length);
    ta_polygon_converter_transfer(ta_parameter_buf, ta_parameter_size);
    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
