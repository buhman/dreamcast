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

constexpr inline uint32_t ap_lm_bus(const uint32_t lm)
{
  switch (lm) {
  default: [[fallthrough]];
  case 0: return ap::lm_bus::_0;
  case 1: return ap::lm_bus::_1;
  case 2: return ap::lm_bus::_2;
  case 3: return ap::lm_bus::_3;
  case 4: return ap::lm_bus::_4;
  }
}

constexpr inline uint32_t ap_lm_bus_int(const uint32_t lm_bus)
{
  switch (lm_bus) {
  default: [[fallthrough]];
  case ap::lm_bus::_0: return 0;
  case ap::lm_bus::_1: return 1;
  case ap::lm_bus::_2: return 2;
  case ap::lm_bus::_3: return 3;
  case ap::lm_bus::_4: return 4;
  }
}
