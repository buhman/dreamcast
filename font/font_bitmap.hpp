#pragma once

#include <cstdint>

#include "../holly/ta_parameter.hpp"

namespace font_bitmap {

uint32_t inflate(const uint32_t pitch,
		 const uint32_t width,
		 const uint32_t height,
                 const uint32_t texture_address,
		 const uint32_t texture_width,
		 const uint32_t texture_height,
		 const uint8_t * src);

void transform_string(ta_parameter_writer& parameter,
                      const uint32_t texture_address,
                      const uint32_t texture_width,
                      const uint32_t texture_height,
                      const uint32_t glyph_width,
                      const uint32_t glyph_height,
                      const int32_t position_x,
                      const int32_t position_y,
                      const char * s,
                      const int32_t len,
		      const uint32_t list_type
                      );

}
