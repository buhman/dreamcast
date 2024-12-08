#pragma once

#include <cstdint>

#include "holly/ta_parameter.hpp"

#include "font/font.h"
#include "gap_buffer.hpp"
#include "viewport_window.hpp"

struct cursor_advance {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

cursor_advance render_primary_buffer(ta_parameter_writer& parameter,
				     const font * font,
				     const glyph * glyphs,
				     const gap_buffer& gb,
				     const viewport_window& window);

void render(ta_parameter_writer& parameter,
	    const font * font,
	    const glyph * glyphs,
	    const gap_buffer& gb,
	    const viewport_window& window);
