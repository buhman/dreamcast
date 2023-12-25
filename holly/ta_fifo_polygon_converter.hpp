#pragma once

#include <cstdint>

void ta_polygon_converter_init(uint32_t opb_total_size, // for one tile, for all render passes
			       uint32_t ta_alloc,
			       uint32_t tile_width,   // in tile units (e.g: (640 / 32))
			       uint32_t tile_height); // in tile units (e.g: (480 / 32))
void ta_polygon_converter_cont(uint32_t ol_base_offset,
			       uint32_t ta_alloc);
void ta_polygon_converter_transfer(volatile uint32_t * buf, uint32_t size);
void ta_wait_opaque_list();
void ta_wait_translucent_list();
void ta_wait_punch_through_list();
