#pragma once

namespace serial_load {

void init();
void recv(uint8_t c);

enum command {
  CMD_NONE,
  CMD_DATA, // DATA 0000 0000 {data}
  CMD_JUMP, // JUMP 0000
  CMD_RATE, // RATE 0000
};

struct state {
  union {
    uint8_t buf[12];
    struct {
      uint8_t cmd[4];
      uint32_t addr1;
      uint32_t addr2;
    };
  };
  uint32_t len;
  enum command command;
};

extern struct state state;

}
