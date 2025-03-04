#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tuple>

#include "maple/maple.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"

namespace maple {

template <uint32_t base_address = 0>
struct host_command_writer {
  uint8_t * const send_buf;
  uint8_t * const recv_buf;
  uint32_t send_offset;
  uint32_t recv_offset;
  uint32_t last_send_offset;

  constexpr host_command_writer(uint8_t * const send_buf,
				uint8_t * const recv_buf)
    : send_buf(send_buf), recv_buf(recv_buf), send_offset(0), recv_offset(0), last_send_offset(0)
  { }

  template <typename C, typename R>
  constexpr inline std::tuple<maple::host_command<typename C::data_fields> *,
			      maple::host_response<typename R::data_fields> *>
  append_command(uint32_t host_port_select,
		 uint32_t destination_ap,
		 bool end_flag,
		 uint32_t send_trailing = 0,
		 uint32_t recv_trailing = 0)
  {
    using command_type = maple::host_command<typename C::data_fields>;
    using response_type = maple::host_response<typename R::data_fields>;
    const uint32_t data_size = (sizeof (typename C::data_fields)) + send_trailing;

    static_assert((sizeof (command_type)) % 4 == 0);
    static_assert((sizeof (response_type)) % 4 == 0);

    auto host_command = reinterpret_cast<command_type *>(&send_buf[send_offset]);
    auto host_response = reinterpret_cast<response_type *>(&recv_buf[recv_offset]);

    host_command->host_instruction = (end_flag ? host_instruction::end_flag : 0)
                                   | (host_port_select & host_instruction::port_select::bit_mask)
                                   | host_instruction::transfer_length(data_size / 4);

    uint32_t host_response_address;
    if constexpr (base_address != 0) {
      host_response_address = base_address + recv_offset;
    } else {
      host_response_address = reinterpret_cast<uint32_t>(host_response);
    }
    host_command->receive_data_storage_address = receive_data_storage_address::address(host_response_address);

    host_command->bus_data.command_code = C::command_code;
    host_command->bus_data.destination_ap = destination_ap;

    host_command->bus_data.source_ap = destination_ap & ap::port_select::bit_mask;
    host_command->bus_data.data_size = data_size / 4;

    last_send_offset = send_offset;
    send_offset += (sizeof (command_type)) + send_trailing;
    recv_offset += (sizeof (response_type)) + recv_trailing;

    return {host_command, host_response};
  }

  template <typename C, typename R>
  constexpr inline std::tuple<maple::host_command<typename C::data_fields> *,
			      maple::host_response<typename R::data_fields> *>
  append_command_all_ports(bool set_end_flag = true)
  {
    auto ret = append_command<C, R>(host_instruction::port_select::a, ap::de::device | ap::port_select::a, false);
    append_command<C, R>(host_instruction::port_select::b, ap::de::device | ap::port_select::b, false);
    append_command<C, R>(host_instruction::port_select::c, ap::de::device | ap::port_select::c, false);
    append_command<C, R>(host_instruction::port_select::d, ap::de::device | ap::port_select::d, set_end_flag);
    return ret;
  }

  void set_end_flag()
  {
    using host_command_type = maple::host_command<uint8_t[0]>;
    auto host_command = reinterpret_cast<host_command_type *>(&send_buf[last_send_offset]);
    host_command->host_instruction |= host_instruction::end_flag;
  }

  uint32_t reset()
  {
    const uint32_t old_recv_offset = recv_offset;
    // reset writer
    send_offset = 0;
    recv_offset = 0;
    return old_recv_offset;
  }
};

}
