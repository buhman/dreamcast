#pragma once

#include <cstdint>

void ta_polygon_converter_init(uint32_t opb_total_size); // total OPB size for all render passes
void ta_polygon_converter_cont(uint32_t ol_base_offset);
void ta_polygon_converter_transfer(volatile uint32_t * buf, uint32_t size);
void ta_wait_opaque_list();
void ta_wait_translucent_list();
