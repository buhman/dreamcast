#pragma once

#include <cstdint>

#include "maple/maple_bus_commands.hpp"
#include "serial_protocol.hpp"

namespace serial_load {

void init(uint32_t speed);
void recv(struct maple_poll_state& poll_state, uint8_t c);
void tick(struct maple_poll_state& poll_state);

enum struct fsm_state {
  idle,
  write,
  read,
  jump,
  speed,
  maple_raw__command,
  maple_raw__maple_dma,
  maple_raw__response,
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
  struct incremental_crc reply_crc;
  uint32_t speed;
};

extern struct state state;

struct lm {
  struct maple::device_id device_id;
};

struct maple_port {
  struct maple::device_id device_id;
  uint8_t ap__lm;
  struct lm lm[5];
};

enum struct step {
  IDLE = 0,
  DEVICE_STATUS,
  EXTENSION__DEVICE_STATUS,
  EXTENSION__DEVICE_STATUS__DEVICE_REPLY,
  RAW,
};

struct maple_command_parameters {
  uint8_t port;
  uint8_t lm;
};

struct maple_poll_state {
  uint32_t send_length;
  uint32_t recv_length;

  bool want_start;
  bool want_raw;
  enum step step;
  struct serial_load::maple_port port[4];
  struct maple_command_parameters command_parameters;
};

}

extern "C" uint32_t __send_buf __asm("__send_buf");
extern "C" uint32_t __recv_buf __asm("__recv_buf");
