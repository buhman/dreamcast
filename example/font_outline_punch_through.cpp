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
#include "memorymap.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"
#include "twiddle.hpp"
#include "palette.hpp"

#include "font/font.h"
#include "font/dejavusansmono/dejavusansmono_mono.data.h"

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

const struct vertex strip_vertices[4] = {
  // [ position       ]  [ uv coordinates ]
  { 0.f,  1.f,  0.f, 0.f, 1.f, },
  { 0.f,  0.f,  0.f, 0.f, 0.f, },
  { 1.f,  1.f,  0.f, 1.f, 1.f, },
  { 1.f,  0.f,  0.f, 1.f, 0.f, },
};
constexpr uint32_t strip_length = (sizeof (strip_vertices)) / (sizeof (struct vertex));

uint32_t transform(ta_parameter_writer& parameter,
                   const uint32_t texture_width, uint32_t texture_height,
                   const uint32_t first_char_code,
                   const glyph * glyphs,
                   const char * s, const uint32_t len,
                   const uint32_t y_offset)
{
  uint32_t advance = 0; // in 26.6 fixed-point

  for (uint32_t string_ix = 0; string_ix < len; string_ix++) {
    char c = s[string_ix];
    auto& glyph = glyphs[c - first_char_code];
    if (glyph.bitmap.width == 0 || glyph.bitmap.height == 0) {
      advance += glyph.metrics.horiAdvance;
      continue;
    }

    const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                          | para_control::list_type::punch_through
                                          | obj_control::col_type::packed_color
                                          | obj_control::texture;

    const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                            | isp_tsp_instruction_word::culling_mode::no_culling;

    const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
                                        | tsp_instruction_word::dst_alpha_instr::zero
                                        | tsp_instruction_word::fog_control::no_fog
                                        | tsp_instruction_word::texture_u_size::from_int(texture_width)
                                        | tsp_instruction_word::texture_v_size::from_int(texture_height);

    const uint32_t texture_address = texture_memory_alloc::texture.start;
    const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
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
      float y = strip_vertices[i].y;
      float z = strip_vertices[i].z;

      x *= glyph.bitmap.width;
      y *= glyph.bitmap.height;
      x += 100.f + ((advance + glyph.metrics.horiBearingX) >> 6);
      y += 200.f - ((glyph.metrics.horiBearingY) >> 6);
      y += y_offset >> 6;
      z = 1.f / (z + 10.f);

      float u = strip_vertices[i].u;
      float v = strip_vertices[i].v;
      u *= glyph.bitmap.width;
      v *= glyph.bitmap.height;
      u += glyph.bitmap.x;
      v += glyph.bitmap.y;
      u = u / static_cast<float>(texture_width);
      v = v / static_cast<float>(texture_height);

      bool end_of_strip = i == strip_length - 1;
      parameter.append<ta_vertex_parameter::polygon_type_3>() =
        ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                            x, y, z,
                                            u, v,
                                            0, // base_color
                                            0  // offset_color
                                            );
    }

    advance += glyph.metrics.horiAdvance;
  }

  return parameter.offset;
}

uint32_t transform2(ta_parameter_writer& parameter,
                    const uint32_t texture_width, uint32_t texture_height)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::src_alpha
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::texture_u_size::from_int(texture_width)
                                      | tsp_instruction_word::texture_v_size::from_int(texture_height);

  const uint32_t texture_address = texture_memory_alloc::texture.start;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
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
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;

    x *= static_cast<float>(texture_width);
    y *= static_cast<float>(texture_height);
    x += 50.f;
    y += 50.f;
    z = 1.f / (z + 9.f);

    float u = strip_vertices[i].u;
    float v = strip_vertices[i].v;

    bool end_of_strip = i == strip_length - 1;
    parameter.append<ta_vertex_parameter::polygon_type_3>() =
      ta_vertex_parameter::polygon_type_3(polygon_vertex_parameter_control_word(end_of_strip),
                                          x, y, z,
                                          u, v,
                                          0, // base_color
                                          0  // offset_color
                                          );
  }

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size)
{
  region_array2(640 / 32, // width
                480 / 32, // height
                opb_size
                );
  background_parameter(0xff0000ff);
}

void inflate_font(const uint8_t * src,
                  const uint32_t width,
                  const uint32_t height)
{
  auto texture = reinterpret_cast<volatile uint32_t *>(&texture_memory64[texture_memory_alloc::texture.start / 4]);

  const uint32_t bits_per_pixel = 1;
  const uint32_t pixels_per_byte = 8;
  const uint32_t stride = width / pixels_per_byte;
  const uint32_t mask = (1 << bits_per_pixel) - 1;

  const uint32_t size = width * height;

  uint8_t temp1[size];
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      const uint32_t texture_ix = y * stride + x / pixels_per_byte;
      const uint32_t texture_ix_mod = x % pixels_per_byte;
      const uint32_t value = (src[texture_ix] >> (bits_per_pixel * texture_ix_mod)) & mask;
      temp1[y * width + x] = value;
    }
  }

  uint8_t temp[size] __attribute__((aligned(4)));

  twiddle::texture(temp,
		   temp1,
		   width,
		   height);

  for (uint32_t i = 0; i < size / 4; i++) {
    texture[i] = reinterpret_cast<uint32_t *>(temp)[i];
  }
}

uint32_t _ta_parameter_buf[((32 * 10 * 17) + 32) / 4];

void main()
{
  video_output::set_mode_vga();

  auto font = reinterpret_cast<const struct font *>(&_binary_font_dejavusansmono_dejavusansmono_mono_data_start);
  auto glyphs = reinterpret_cast<const struct glyph *>(&font[1]);
  auto texture = reinterpret_cast<const uint8_t *>(&glyphs[font->glyph_count]);

  /*
  serial::integer<uint32_t>(font->first_char_code);
  serial::integer<uint32_t>(font->glyph_count);
  serial::integer<uint32_t>(font->glyph_height);
  serial::integer<uint32_t>(font->texture_stride);
  serial::integer<uint32_t>(font->texture_width);
  serial::integer<uint32_t>(font->texture_height);
  serial::character('\n');
  serial::integer<uint32_t>(((uint32_t)glyphs) - ((uint32_t)font));
  serial::integer<uint32_t>(((uint32_t)texture) - ((uint32_t)font));
  */

  inflate_font(texture,
               font->texture_width,
	       font->texture_height);
  palette_data<2>();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::_16x4byte
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
                                       , .translucent = 0
                                       , .translucent_modifier = 0
                                       , .punch_through = 16 * 4
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  holly.PT_ALPHA_REF = 0x1;
  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;

  const char ana[18] = "A from ana i know";
  const char cabal[27] = "where is this secret cabal";

  while (true) {
    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    auto parameter = ta_parameter_writer(ta_parameter_buf);

    transform2(parameter,
               font->texture_width, font->texture_height);

    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    transform(parameter,
              font->texture_width, font->texture_height,
              font->first_char_code,
              glyphs,
              ana, 17,
              font->face_metrics.height * 0);

    transform(parameter,
              font->texture_width, font->texture_height,
              font->first_char_code,
              glyphs,
              cabal, 26,
              font->face_metrics.height * 1);

    parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_punch_through_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
