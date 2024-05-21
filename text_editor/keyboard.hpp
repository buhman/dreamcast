#pragma once

#include <cstdint>

#include "maple/maple_bus_ft6.hpp"
#include "gap_buffer.hpp"

void keyboard_do_get_condition(ft6::data_transfer::data_format& data);

void keyboard_debug(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix);

void keyboard_update(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix, gap_buffer& gb);
