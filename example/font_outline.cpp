#include <cstdint>

#include "align.hpp"

#include "vga.hpp"
#include "holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/texture_memory_alloc.hpp"
#include "memorymap.hpp"
#include "holly/background.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "twiddle.hpp"
#include "serial.hpp"

#include "font/font.hpp"
#include "dejavusansmono.hpp"

#include "sperrypc.hpp"

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
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));

  uint32_t advance = 0; // in 26.6 fixed-point

  for (uint32_t string_ix = 0; string_ix < len; string_ix++) {
    char c = s[string_ix];
    auto& glyph = glyphs[c - first_char_code];
    if (glyph.bitmap.width == 0 || glyph.bitmap.height == 0) {
      advance += glyph.metrics.horiAdvance;
      continue;
    }

    auto polygon = global_polygon_type_0(texture_address);
    polygon.parameter_control_word = para_control::para_type::polygon_or_modifier_volume
				   | para_control::list_type::opaque
				   | obj_control::col_type::packed_color
				   | obj_control::texture;

    polygon.tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
				 | tsp_instruction_word::dst_alpha_instr::zero
				 | tsp_instruction_word::fog_control::no_fog
				 | tsp_instruction_word::texture_u_size::from_int(texture_width)
				 | tsp_instruction_word::texture_v_size::from_int(texture_height);

    polygon.texture_control_word = texture_control_word::pixel_format::_8bpp_palette
				 | texture_control_word::scan_order::twiddled
				 | texture_control_word::texture_address(texture_address / 8);
    parameter.append<global_polygon_type_0>() = polygon;

    for (uint32_t i = 0; i < strip_length; i++) {
      bool end_of_strip = i == strip_length - 1;

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

      parameter.append<vertex_polygon_type_3>() =
	vertex_polygon_type_3(x, y, z,
			      u, v,
			      0x00000000, // base_color
			      end_of_strip);
    }

    advance += glyph.metrics.horiAdvance;
  }

  return parameter.offset;
}

uint32_t transform2(ta_parameter_writer& parameter,
		    const uint32_t texture_width, uint32_t texture_height)
{
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));

  auto polygon = global_polygon_type_0(texture_address);
  polygon.parameter_control_word = para_control::para_type::polygon_or_modifier_volume
				 | para_control::list_type::opaque
				 | obj_control::col_type::packed_color
				 | obj_control::texture;

  polygon.tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
			       | tsp_instruction_word::dst_alpha_instr::zero
			       | tsp_instruction_word::fog_control::no_fog
			       | tsp_instruction_word::texture_u_size::from_int(texture_width)
			       | tsp_instruction_word::texture_v_size::from_int(texture_height);

  polygon.texture_control_word = texture_control_word::pixel_format::_8bpp_palette
			       | texture_control_word::scan_order::twiddled
			       | texture_control_word::texture_address(texture_address / 8);
  parameter.append<global_polygon_type_0>() = polygon;

  for (uint32_t i = 0; i < strip_length; i++) {
    bool end_of_strip = i == strip_length - 1;

    float x = strip_vertices[i].x;
    float y = strip_vertices[i].y;
    float z = strip_vertices[i].z;

    x *= 128.f;
    y *= 256.f;
    x += 50.f;
    y += 50.f;
    z = 1.f / (z + 9.f);

    float u = strip_vertices[i].u;
    float v = strip_vertices[i].v;

    parameter.append<vertex_polygon_type_3>() =
      vertex_polygon_type_3(x, y, z,
			    u, v,
			    0x00000000, // base_color
			    end_of_strip);
  }

  return parameter.offset;
}

void init_texture_memory(const struct opb_size& opb_size)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory32);

  background_parameter(mem->background, 0xff0000ff);

  region_array2(mem->region_array,
	        (offsetof (struct texture_memory_alloc, object_list)),
		640 / 32, // width
		480 / 32, // height
		opb_size
		);
}

void inflate_font(const uint32_t * src,
		  const uint32_t stride,
		  const uint32_t curve_end_ix)
{
  auto mem = reinterpret_cast<volatile texture_memory_alloc *>(texture_memory64);
  auto texture = reinterpret_cast<volatile uint32_t *>(mem->texture);

  twiddle::texture3<8, 8>(texture, reinterpret_cast<const uint8_t *>(src),
			  stride,
			  curve_end_ix);
}

template <int C>
void palette_data()
{
  static_assert(C >= 2);
  constexpr int increment = 256 / C;

  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::rgb565;

  // generate a palette with `C` shades of grey,
  // ranging in intensity from rgb565(0, 0, 0) to rgb565(31, 63, 31)
  for (int i = 0; i < 256; i += increment) {
    holly.PALETTE_RAM[i / increment] = ((i >> 3) << 11)
                                     | ((i >> 2) << 5)
                                     | ((i >> 3) << 0);
  }
}

uint32_t _ta_parameter_buf[((32 * 10 * 17) + 32) / 4];

void main()
{
  vga();

  auto font = reinterpret_cast<const struct font *>(&_binary_dejavusansmono_data_start);
  auto glyphs = reinterpret_cast<const struct glyph *>(&font[1]);
  auto texture = reinterpret_cast<const uint32_t *>(&glyphs[font->glyph_count]);

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
	       font->texture_stride,
	       font->max_z_curve_ix);
  palette_data<256>();

  // The address of `ta_parameter_buf` must be a multiple of 32 bytes.
  // This is mandatory for ch2-dma to the ta fifo polygon converter.
  uint32_t * ta_parameter_buf = align_32byte(_ta_parameter_buf);

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

  constexpr uint32_t tiles = (640 / 32) * (320 / 32);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  init_texture_memory(opb_size);

  uint32_t frame_ix = 0;
  constexpr uint32_t num_frames = 1;

  const char ana[18] = "A from ana i know";
  const char cabal[27] = "where is this secret cabal";

  while (true) {
    ta_polygon_converter_init(opb_size.total() * tiles, ta_alloc,
			      640, 480);

    auto parameter = ta_parameter_writer(ta_parameter_buf);

    /*
    transform2(parameter,
	       font->texture_width, font->texture_height);
    */

    transform(parameter,
	      font->texture_width, font->texture_height,
	      font->first_char_code,
	      glyphs,
	      ana, 17,
	      font->glyph_height * 0);

    transform(parameter,
	      font->texture_width, font->texture_height,
	      font->first_char_code,
	      glyphs,
	      cabal, 26,
	      font->glyph_height * 1);

    parameter.append<global_end_of_list>() = global_end_of_list();

    ta_polygon_converter_transfer(ta_parameter_buf, parameter.offset);
    ta_wait_opaque_list();

    core_start_render(frame_ix, num_frames);

    v_sync_out();
    v_sync_in();
    core_wait_end_of_render_video(frame_ix, num_frames);

    frame_ix++;
  }
}
