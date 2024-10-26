#pragma once

#include "string.hpp"

namespace serial {

void reset_txrx();

void init(uint8_t bit_rate);

void character(const char c);

void string(const char * s);

void string(const uint8_t * s, uint32_t len);

void hexlify(const uint8_t n);

void hexlify(const uint8_t * s, uint32_t len);

void hexlify(const uint32_t * s, uint32_t len);

using hex = string::hex_type;
using dec = string::dec_type;

template <typename T, typename conv_type>
void integer(const T n, const char end, const uint32_t length);

template <typename T, typename conv_type = hex>
inline void integer(const T n, const char end)
{
  constexpr uint32_t length = (sizeof (T)) * 2;
  return integer<T, conv_type>(n, end, length);
}

template <typename T, typename conv_type = hex>
inline void integer(const T n)
{
  return integer<T, conv_type>(n, '\n');
}

}
