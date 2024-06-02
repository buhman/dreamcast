#include "render.hpp"
#include "minmax.hpp"
#include "transform.hpp"

#include "holly/ta_global_parameter.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "sh7091/serial.hpp"

static inline uint32_t get_font_ix(const struct font * font,
				   char_type c)
{
  uint32_t ix;
  if (c >= font->first_char_code && c <= font->last_char_code) {
    ix = c - font->first_char_code;
  } else if (c == '\n') {
    ix = '#' - font->first_char_code;
  } else if ('%' <= font->last_char_code) {
    ix = '%' - font->first_char_code;
  } else {
    ix = 0;
  }
  return ix;
}

constexpr inline int32_t int_26_6(int32_t n)
{
  int32_t v = n >> 6;
//float d = n & 63;
//return v + (d / 64.f);
  return v;
}

cursor_advance render_primary_buffer(ta_parameter_writer& parameter,
				     const font * font,
				     const glyph * glyphs,
				     const gap_buffer& gb,
				     const viewport_window& window)
{
  int32_t first_line = min(-1, max(window.first_line, gb.line.length - 1));

  cursor_advance cursor = { 0 };
  int32_t h_advance = 0;
  int32_t v_advance = 0;

  int32_t init_i = first_line >= 0 ? gb.line.offsets[first_line] + 1 : 0;
  for (int32_t i = init_i; i <= gb.size; i++) {
    if (i == gb.gap_start) {
      uint32_t ix = get_font_ix(font, ' ');
      auto& glyph = glyphs[ix];

      cursor.x = h_advance + glyph.metrics.horiBearingX;
      cursor.y = v_advance - glyph.metrics.horiBearingY;

      cursor.width = glyph.metrics.horiAdvance;
      cursor.height = font->face_metrics.height;

      i = gb.gap_end;
    }

    if (i == gb.size)
      break;

    int32_t x = window.box.x0 + int_26_6(h_advance);
    int32_t y = window.box.y0 + int_26_6(v_advance);

    char_type c = gb.buf[i];
    uint32_t ix = get_font_ix(font, c);
    auto& glyph = glyphs[ix];
    if (x + int_26_6(glyph.metrics.horiAdvance) <= window.box.x1) {
      transform_glyph(parameter,
		      font->texture_width,
		      font->texture_height,
		      glyph,
		      x,
		      y
		      );
    }

    if (c == '\n') {
      h_advance = 0;
      v_advance += font->face_metrics.height;
      if (int_26_6(v_advance + font->face_metrics.height) > window.box.y1) {
	break;
      }
    } else {
      h_advance += glyph.metrics.horiAdvance;
    }
  }

  return cursor;
}

void render_cursor(ta_parameter_writer& parameter,
		   const cursor_advance& cursor,
		   const viewport_window& window)
{
  float x = window.box.x0 + int_26_6(cursor.x);
  float y = window.box.y0 + int_26_6(cursor.y);
  float width = int_26_6(cursor.width);
  float height = int_26_6(cursor.height);

  transform_sprite(parameter,
		   x, // x
		   y, // y
		   width,
		   height
		   );
}

void render(ta_parameter_writer& parameter,
	    const font * font,
	    const glyph * glyphs,
	    const gap_buffer& gb,
	    const viewport_window& window)
{
  cursor_advance cursor = render_primary_buffer(parameter, font, glyphs, gb, window);

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  ta_polygon_converter_transfer(parameter.buf, parameter.offset);
  ta_wait_opaque_list();

  parameter.offset = 0;

  /*
  render_cursor(parameter, cursor, window);

  parameter.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);

  ta_polygon_converter_transfer(parameter.buf, parameter.offset);
  serial::string("wait tl\n");
  ta_wait_translucent_list();
  serial::string("done tl\n");
  */
}
