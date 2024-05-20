#pragma once

#include <cstdint>

#include "maple/maple_bus_ft6.hpp"
#include "gap_buffer.hpp"

void keyboard_do_get_condition(uint32_t * command_buf,
			       uint32_t * receive_buf,
			       ft6::data_transfer::data_format& data);

void keyboard_debug(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix);

void keyboard_update(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix, gap_buffer& gb);
