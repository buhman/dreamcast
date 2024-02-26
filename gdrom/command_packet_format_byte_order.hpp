#include <cstdint>

namespace gdrom_command_packet_format {
  template <int N>
  constexpr void byte_order(uint8_t * buf, const uint32_t n);

  template <>
  constexpr void byte_order<2>(uint8_t * buf, const uint32_t n)
  {
    buf[0] = n >> 8;
    buf[1] = n >> 0;
  }

  template <>
  constexpr void byte_order<3>(uint8_t * buf, const uint32_t n)
  {
    buf[0] = n >> 16;
    buf[1] = n >> 8;
    buf[1] = n >> 0;
  }
}
