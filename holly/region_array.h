#pragma once

#include <cstdint>

void region_array(volatile uint32_t * buf,
		  uint32_t ol_base,
		  const uint32_t width,   // in tile units (1 tile unit = 32 pixels)
		  const uint32_t height); // in tile units (1 tile unit = 32 pixels)
