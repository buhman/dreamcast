#pragma once

#include <cstdint>

namespace maple {

  struct protocol_header {
    uint8_t command_code;
    uint8_t destination_ap;
    uint8_t source_ap;
    uint8_t data_size;
  };

  template <typename T>
  struct host_command {
    uint32_t host_instruction;     // interpreted by the Maple DMA controller
    uint32_t receive_data_address; // interpreted by the Maple DMA controller
    protocol_header header;
    T data;
  };

  static_assert((sizeof (host_command<uint8_t[0]>)) == 12);
  static_assert((sizeof (host_command<uint8_t[0]>[4])) == 48);

  constexpr inline uint32_t align(uint32_t mem)
  {
    return (mem + 31) & ~31;
  }

  template <typename T>
  struct host_response {
    protocol_header header;
    T data;
    uint8_t _pad[align((sizeof (protocol_header)) + (sizeof (T))) - ((sizeof (protocol_header)) + (sizeof (T)))];
  };

  static_assert((sizeof (host_response<uint8_t[0]>)) == 32);
}
