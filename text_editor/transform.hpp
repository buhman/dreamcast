#pragma once

#include <cstdint>

#include "holly/ta_parameter.hpp"
#include "font/font.h"

void transform_cursor(ta_parameter_writer& writer,
		      const float origin_x,
		      const float origin_y,
		      const float width,
		      const float height);

void glyph_begin(ta_parameter_writer& writer,
                 const uint32_t texture_width, uint32_t texture_height);

void transform_glyph(ta_parameter_writer& writer,
		     const float r_texture_width,
                     const float r_texture_height,
		     const glyph& glyph,
		     const float origin_x,
		     const float origin_y);
