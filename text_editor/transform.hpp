#pragma once

#include <cstdint>

#include "holly/ta_parameter.hpp"
#include "font/font.h"

void transform_sprite(ta_parameter_writer& parameter,
		      const float origin_x,
		      const float origin_y,
		      const float width,
		      const float height);

void transform_glyph(ta_parameter_writer& parameter,
		     const uint32_t texture_width, uint32_t texture_height,
		     const glyph& glyph,
		     const float origin_x,
		     const float origin_y);
