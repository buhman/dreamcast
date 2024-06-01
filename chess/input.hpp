#pragma once

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_ft0.hpp"
#include "maple/maple_bus_ft9.hpp"

namespace input {

struct port_state {
  uint32_t function_type;
  uint32_t next_function_type;
  maple::host_response<maple::data_transfer<ft0::data_transfer::data_format>::data_fields> * host_response_data_transfer_ft0;
  maple::host_response<maple::data_transfer<ft9::data_transfer::data_format>::data_fields> * host_response_data_transfer_ft9;
};

struct input_state {
  port_state port[4];
};

extern input_state state;

void state_update(uint32_t * send_buf, uint32_t * recv_buf);
void state_init();

}
