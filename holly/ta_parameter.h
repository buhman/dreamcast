#pragma once

#include <cstdint>

void vertex(volatile uint32_t * buf,
	    const float x,
	    const float y,
	    const float z,
	    const uint32_t base_color,
	    bool end_of_strip
	    );

void triangle(volatile uint32_t * buf);

void end_of_list(volatile uint32_t * buf);
