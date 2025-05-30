#pragma once

#include <cstdint>

void background_parameter_textured(const uint32_t background_start,
                                   const int texture_u_size,
                                   const int texture_v_size,
                                   const uint32_t texture_address);
void background_parameter2(const uint32_t background_start,
			   const uint32_t color);
void background_parameter(const uint32_t color);
