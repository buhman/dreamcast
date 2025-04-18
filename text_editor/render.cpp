#include "render.hpp"
#include "minmax.hpp"
#include "transform.hpp"
#include "unparse.hpp"

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

cursor_advance render_primary_buffer(ta_parameter_writer& writer,
                                     const font * font,
                                     const glyph * glyphs,
                                     const gap_buffer& gb,
                                     const viewport_window& window)
{
  int32_t first_line = window.first_line;

  cursor_advance cursor = { 0 };
  cursor.x = -1;
  cursor.y = -1;
  int32_t h_advance = 0;
  int32_t v_advance = 0;

  const float r_texture_width = 1.0f / font->texture_width;
  const float r_texture_height = 1.0f / font->texture_height;

  int row = first_line;

  // line 0 is prior to the beginning of gb.line.offsets
  // gb.line.offsets is the index of the newline character
  int32_t init_i;
  if (first_line == 0) {
    init_i = 0;
  } else if ((first_line - 1) >= 0 && (first_line - 1) < gb.line.length) {
    init_i = gb.line.offsets[first_line - 1] + 1;
  } else {
    init_i = gb.size;
  }
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
      transform_glyph(writer,
                      r_texture_width,
                      r_texture_height,
		      glyph,
		      x,
		      y
		      );
    }

    if (c == '\n') {
      h_advance = 0;
      v_advance += font->face_metrics.height;
      row += 1;
      if (int_26_6(v_advance + font->face_metrics.height) > window.box.y1) {
	break;
      }
    } else {
      h_advance += glyph.metrics.horiAdvance;
    }
  }

  return cursor;
}

void render_text(ta_parameter_writer& writer,
                 const font * font,
                 const glyph * glyphs,
                 const char * buf,
                 int length,
                 int32_t x0,
                 int32_t y0)
{
  int32_t h_advance = 0;
  int32_t v_advance = 0;

  const float r_texture_width = 1.0f / font->texture_width;
  const float r_texture_height = 1.0f / font->texture_height;

  for (int i = 0; i < length; i++) {
    int32_t x = x0 + int_26_6(h_advance);
    int32_t y = y0 + int_26_6(v_advance);

    char_type c = buf[i];
    uint32_t ix = get_font_ix(font, c);
    auto& glyph = glyphs[ix];

    transform_glyph(writer,
                    r_texture_width,
                    r_texture_height,
                    glyph,
                    x,
                    y
                    );

    h_advance += glyph.metrics.horiAdvance;
  }
}

void render_cursor(ta_parameter_writer& writer,
		   const cursor_advance& cursor,
		   const viewport_window& window)
{
  float x = window.box.x0 + int_26_6(cursor.x);
  float y = window.box.y0 + int_26_6(cursor.y);
  float width = int_26_6(cursor.width);
  float height = int_26_6(cursor.height);

  transform_cursor(writer,
		   x, // x
		   y, // y
		   width,
		   height
		   );
}

static inline int copy_str(const char * s, char * d, int d_ix)
{
  while (*s != 0) {
    d[d_ix++] = *s++;
  }
  return d_ix;
}

void render_status(ta_parameter_writer& writer,
                   const font * font,
                   const glyph * glyphs,
                   const gap_buffer& gb,
                   const viewport_window& window)
{
  char buf[256];
  int ix = 0;

  ix = copy_str("(", buf, ix);
  ix += unparse_base10(&buf[ix], gb.line.gap, 0, 0);
  ix = copy_str(",", buf, ix);
  ix += unparse_base10(&buf[ix], gap_column_number(gb), 0, 0);
  ix = copy_str(")  ", buf, ix);
  ix += unparse_base10(&buf[ix], window.first_line, 0, 0);

  render_text(writer,
              font,
              glyphs,
              buf,
              ix,
              window.box.x0,
              480 - int_26_6(font->face_metrics.height) * 2);
}

void render(ta_parameter_writer& writer,
            const font * font,
            const glyph * glyphs,
            const gap_buffer& gb,
            const viewport_window& window)
{
  glyph_begin(writer,
              font->texture_width,
              font->texture_height);

  render_status(writer, font, glyphs, gb, window);

  cursor_advance cursor = render_primary_buffer(writer, font, glyphs, gb, window);

  render_cursor(writer, cursor, window);

  writer.append<ta_global_parameter::end_of_list>() = ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}
