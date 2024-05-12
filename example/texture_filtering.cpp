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

#include "sh7091/serial.hpp"

#include "bbb.hpp"

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
  { -0.5f,  -0.5f,  0.f, 0.f, 0.f, 0xffffffff},
  {  0.5f,  -0.5f,  0.f, 1.f, 0.f, 0xffffffff},
  { -0.5f,   0.5f,  0.f, 0.f, 1.f, 0xffffffff},
  {  0.5f,   0.5f,  0.f, 1.f, 1.f, 0xffffffff},
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

namespace defaults {
//constexpr uint32_t parameter_control_word = para_control::para_type::sprite
  constexpr uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                            | obj_control::col_type::packed_color
                                            | obj_control::texture
    ;
    //| obj_control::_16bit_uv;

  constexpr uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                              | isp_tsp_instruction_word::culling_mode::no_culling;

  constexpr uint32_t tsp_instruction_word = tsp_instruction_word::src_select::primary_accumulation_buffer
                                          | tsp_instruction_word::dst_select::primary_accumulation_buffer
                                          | tsp_instruction_word::fog_control::no_fog
                                          | tsp_instruction_word::texture_u_size::_1024
                                          | tsp_instruction_word::texture_v_size::_1024;

  constexpr uint32_t texture_address = texture_memory_alloc::texture.start;
  constexpr uint32_t texture_control_word = texture_control_word::mip_mapped
                                          | texture_control_word::pixel_format::_565
                                          | texture_control_word::scan_order::twiddled
                                          | texture_control_word::texture_address(texture_address / 8);
}

void append_parameter(ta_parameter_writer& parameter,
		      const uint32_t parameter_control_word,
		      const uint32_t isp_tsp_instruction_word,
		      const uint32_t tsp_instruction_word,
		      const uint32_t texture_control_word)
{
  parameter.append<ta_global_parameter::polygon_type_0>() =
    ta_global_parameter::polygon_type_0(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        0, // data_size_for_sort_dma
                                        0  // next_address_for_sort_dma
                                        );

  /*
  constexpr uint32_t base_color = 0xffff0000;
  parameter.append<ta_global_parameter::sprite>() =
    ta_global_parameter::sprite(parameter_control_word,
                                isp_tsp_instruction_word,
                                tsp_instruction_word,
                                texture_control_word,
                                base_color,
                                0, // offset_color
                                0, // data_size_for_sort_dma
                                0); // next_address_for_sort_dma
  */
}

void append_point_sampled(ta_parameter_writer& parameter)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::point_sampled;

  const uint32_t texture_control_word = defaults::texture_control_word;

  append_parameter(parameter, parameter_control_word, isp_tsp_instruction_word, tsp_instruction_word, texture_control_word);
}

void append_bilinear(ta_parameter_writer& parameter)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::bilinear_filter;

  const uint32_t texture_control_word = defaults::texture_control_word;

  append_parameter(parameter, parameter_control_word, isp_tsp_instruction_word, tsp_instruction_word, texture_control_word);
}

void append_trilinear_pass_a(ta_parameter_writer& parameter)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::opaque;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::trilinear_pass_a;

  const uint32_t texture_control_word = defaults::texture_control_word;

  append_parameter(parameter, parameter_control_word, isp_tsp_instruction_word, tsp_instruction_word, texture_control_word);
}

void append_trilinear_pass_b(ta_parameter_writer& parameter)
{
  const uint32_t parameter_control_word = defaults::parameter_control_word
                                        | para_control::list_type::translucent;

  const uint32_t isp_tsp_instruction_word = defaults::isp_tsp_instruction_word;

  const uint32_t tsp_instruction_word = defaults::tsp_instruction_word
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::one
                                      | tsp_instruction_word::filter_mode::trilinear_pass_b;

  const uint32_t texture_control_word = defaults::texture_control_word;

  append_parameter(parameter, parameter_control_word, isp_tsp_instruction_word, tsp_instruction_word, texture_control_word);
}

enum class filter_type {
  point_sampled,
  bilinear,
  trilinear_pass_a,
  trilinear_pass_b,
};

void transform(ta_parameter_writer& parameter,
               const vertex * strip_vertices,
               const uint32_t strip_length,
               enum filter_type filter
               )
{
  switch (filter) {
  case filter_type::point_sampled:    append_point_sampled(parameter);    break;
  case filter_type::bilinear:         append_bilinear(parameter);         break;
  case filter_type::trilinear_pass_a: append_trilinear_pass_a(parameter); break;
  case filter_type::trilinear_pass_b: append_trilinear_pass_b(parameter); break;
  }

  //struct vertex v[strip_length];

  for (uint32_t i = 0; i < strip_length; i++) {
    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;
    float x1;

    x1 = x * __builtin_cosf(theta / 5.f) - z * __builtin_sinf(theta / 5.f);
    z  = x * __builtin_sinf(theta / 5.f) + z * __builtin_cosf(theta / 5.f);
    x  = x1;

    x *= 1024.f * (__builtin_cosf(theta / 10.f) + 1.0f);
    y *= 1024.f * (__builtin_cosf(theta / 10.f) + 1.0f);
    x += 320.f;
    y += 240.f;
    z = 1.f / (z + 10.f);

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          strip_vertices[i].u,
                                          strip_vertices[i].v,
                                          strip_vertices[i].color,
                                          0 // offset_color
                                          );
    /*
    v[i].x = x;
    v[i].y = y;
    v[i].z = z;
    v[i].u = strip_vertices[i].u;
    v[i].v = strip_vertices[i].v;
    */
  }

  /*
  parameter.append<ta_vertex_parameter::sprite_type_1>() =
    ta_vertex_parameter::sprite_type_1(para_control::para_type::vertex_parameter,
				       v[0].x,
				       v[0].y,
				       v[0].z,
				       v[1].x,
				       v[1].y,
				       v[1].z,
				       v[2].x,
				       v[2].y,
				       v[2].z,
				       v[3].x,
				       v[3].y,
				       uv_16bit(v[0].u, v[0].v),
				       uv_16bit(v[1].u, v[1].v),
				       uv_16bit(v[2].u, v[2].v));
  */

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
		480 / 32, // height
		opb_size
		);

  background_parameter(0xff00ff00);
}

uint8_t const * const mips[] = {
  reinterpret_cast<uint8_t *>(&_binary_bbb1_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb2_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb4_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb8_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb16_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb32_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb64_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb128_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb256_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb512_data_start),
  reinterpret_cast<uint8_t *>(&_binary_bbb1024_data_start)
};

void _copy_bbb_texture(uint32_t dst_offset, uint8_t const * const src, uint32_t mip)
{
  auto area  = mip * mip;
  uint16_t temp[area];
  for (uint32_t px = 0; px < area; px++) {
    uint8_t r = src[px * 3 + 0];
    uint8_t g = src[px * 3 + 1];
    uint8_t b = src[px * 3 + 2];

    uint16_t rgb565 = ((r / 8) << 11) | ((g / 4) << 5) | ((b / 8) << 0);
    temp[px] = rgb565;
  }

  auto texture = reinterpret_cast<volatile uint16_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);
  auto dst = &texture[dst_offset / 2];
  twiddle::texture(dst, temp, mip, mip);
}

void copy_bbb_texture()
{
  uint32_t dst_offset = 6;
  uint32_t ix = 0;
  for (uint32_t mip = 1; mip <= 1024; mip *= 2) {
    _copy_bbb_texture(dst_offset, mips[ix], mip);
    ix += 1;
    dst_offset += mip * mip * 2;
  }
}

uint32_t _ta_parameter_buf[((32 * (strip_length + 2)) + 32) / 4];

void main()
{
  video_output::set_mode_vga();
  holly.TEXT_CONTROL = text_control::bank_bit(0);

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
    //| ta_alloc_ctrl::t_opb::_16x4byte
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte
                              ;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
                                       //, .translucent = 16 * 4
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);
  copy_bbb_texture();

  uint32_t frame_ix = 0;

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    {
      auto parameter = ta_parameter_writer(ta_parameter_buf);
      transform(parameter, strip_vertices, strip_length, filter_type::trilinear_pass_a);
      ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
      ta_wait_opaque_list();
    }

    /*
    {
      auto parameter = ta_parameter_writer(ta_parameter_buf);
      parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
      //transform(parameter, strip_vertices, strip_length, filter_type::trilinear_pass_b);
      ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
      ta_wait_translucent_list();
    }
    */

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    theta += half_degree;
    frame_ix = (frame_ix + 1) & 1;
  }
}
