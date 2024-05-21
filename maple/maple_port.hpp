#pragma once

#include "maple_bus_bits.hpp"

constexpr inline uint32_t ap_port_select(const uint32_t port)
{
  switch (port) {
  default: [[fallthrough]];
  case 0: return ap::port_select::a;
  case 1: return ap::port_select::b;
  case 2: return ap::port_select::c;
  case 3: return ap::port_select::d;
  }
}

constexpr inline uint32_t host_instruction_port_select(const uint32_t port)
{
  switch (port) {
  default: [[fallthrough]];
  case 0: return host_instruction::port_select::a;
  case 1: return host_instruction::port_select::b;
  case 2: return host_instruction::port_select::c;
  case 3: return host_instruction::port_select::d;
  }
}
