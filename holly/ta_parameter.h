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

void textured_vertex(volatile uint32_t * buf,
		     const float x,
		     const float y,
		     const float z,
		     const float u,
		     const float v,
		     const uint32_t base_color,
		     const uint32_t offset_color,
		     bool end_of_strip
		     );

void textured_triangle(volatile uint32_t * buf,
		       uint32_t texture_address
		       );

void end_of_list(volatile uint32_t * buf);
