#pragma once

#include <cstdint>

#include "serial_protocol.hpp"

namespace serial_load {

void init();
void recv(uint8_t c);
void tick();

enum struct fsm_state {
  idle,
  write,
  read,
  jump,
  speed,
};

struct state_arglen_reply {
  uint32_t command;
  uint32_t reply;
  enum fsm_state fsm_state;
};

struct incremental_crc {
  uint32_t value;
  uint32_t offset;
};

struct state {
  union command_reply buf;
  uint32_t len;
  enum fsm_state fsm_state;
  struct incremental_crc write_crc;
  struct incremental_crc read_crc;
};

extern struct state state;

}
