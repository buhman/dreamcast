#pragma once

#include <cstdint>

void ta_polygon_converter_init();
void ta_polygon_converter_transfer(volatile uint32_t * buf, uint32_t size);
void ta_wait_opaque_list();
