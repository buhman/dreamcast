#pragma once

#include <cstdint>

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"

namespace maple {

template <typename C, typename R>
void init_host_command_all_ports(uint32_t * command_buf, uint32_t * receive_buf,
				 const typename C::data_fields& data_fields)
{
  using command_type = maple::host_command<typename C::data_fields>;
  using response_type = maple::command_response<typename R::data_fields>;

  auto host_command = reinterpret_cast<command_type *>(command_buf);
  auto response_command = reinterpret_cast<response_type *>(receive_buf);

  init_host_command((uint32_t*)&host_command[0], (uint32_t*)&response_command[0],
                    host_instruction::port_select::a, // destination_port
                    ap::de::device | ap::port_select::a, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag
  host_command[0].bus_data.data_fields = data_fields;

  init_host_command((uint32_t*)&host_command[1], (uint32_t*)&response_command[1],
                    host_instruction::port_select::b, // destination_port
                    ap::de::device | ap::port_select::b, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag
  host_command[1].bus_data.data_fields = data_fields;

  init_host_command((uint32_t*)&host_command[2], (uint32_t*)&response_command[2],
                    host_instruction::port_select::c, // destination_port
                    ap::de::device | ap::port_select::c, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag
  host_command[2].bus_data.data_fields = data_fields;

  init_host_command((uint32_t*)&host_command[3], (uint32_t*)&response_command[3],
                    host_instruction::port_select::d, // destination_port
                    ap::de::device | ap::port_select::d, C::command_code, (sizeof (typename C::data_fields)),
                    true); // end_flag
  host_command[3].bus_data.data_fields = data_fields;
}

template <typename C, typename R>
void init_host_command_all_ports(uint32_t * command_buf, uint32_t * receive_buf)
{
  using command_type = maple::host_command<typename C::data_fields>;
  using response_type = maple::command_response<typename R::data_fields>;

  auto host_command = reinterpret_cast<command_type *>(command_buf);
  auto response_command = reinterpret_cast<response_type *>(receive_buf);

  init_host_command((uint32_t*)&host_command[0], (uint32_t*)&response_command[0],
                    host_instruction::port_select::a, // destination_port
                    ap::de::device | ap::port_select::a, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag

  init_host_command((uint32_t*)&host_command[1], (uint32_t*)&response_command[1],
                    host_instruction::port_select::b, // destination_port
                    ap::de::device | ap::port_select::b, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag

  init_host_command((uint32_t*)&host_command[2], (uint32_t*)&response_command[2],
                    host_instruction::port_select::c, // destination_port
                    ap::de::device | ap::port_select::c, C::command_code, (sizeof (typename C::data_fields)),
                    false); // end_flag

  init_host_command((uint32_t*)&host_command[3], (uint32_t*)&response_command[3],
                    host_instruction::port_select::d, // destination_port
                    ap::de::device | ap::port_select::d, C::command_code, (sizeof (typename C::data_fields)),
                    true); // end_flag
}

}
