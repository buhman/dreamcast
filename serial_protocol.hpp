#pragma once

#include <stdint.h>

#include "maple/maple_bus_commands.hpp"

#include "crc32.h"

namespace serial_load {

consteval uint32_t gen_cmd(const char * s)
{
  uint32_t x = 0
    | s[0] << 0
    | s[1] << 8
    | s[2] << 16
    | s[3] << 24;

  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;

  return x;
}

union command_reply {
  uint8_t u8[16];
  uint32_t u32[4];
  struct {
    uint32_t cmd;
    uint32_t arg[2];
    uint32_t crc;
  };
};

static inline uint32_t le_bswap(const uint32_t n)
{
  if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return n;
  else
    return __builtin_bswap32(n);
}

constexpr union command_reply command_reply(uint32_t cmd, uint32_t arg0, uint32_t arg1)
{
  union command_reply command = {
    .cmd = le_bswap(cmd),
    .arg = { le_bswap(arg0), le_bswap(arg1) },
    .crc = 0,
  };
  command.crc = le_bswap(crc32(command.u8, 12));
  return command;
}

namespace command {
  constexpr uint32_t _write = gen_cmd("WRTE");
  constexpr uint32_t _read = gen_cmd("READ");
  constexpr uint32_t _jump = gen_cmd("JUMP");
  constexpr uint32_t _speed = gen_cmd("SPED");

  constexpr uint32_t _maple_raw = gen_cmd("MPRW");

  static_assert(_write == 0x2cc46ed8);
  static_assert(_read  == 0xf18d57c7);
  static_assert(_jump  == 0xa6696f38);
  static_assert(_speed == 0x27a7a9f4);

  static_assert(_maple_raw  == 0xb62422e0);

  constexpr union command_reply write(uint32_t dest, uint32_t size)
  {
    return command_reply(_write, dest, size);
  }

  constexpr union command_reply read(uint32_t dest, uint32_t size)
  {
    return command_reply(_read, dest, size);
  }

  constexpr union command_reply jump(uint32_t dest)
  {
    return command_reply(_jump, dest, 0);
  }

  constexpr union command_reply speed(uint32_t speed)
  {
    return command_reply(_speed, speed, 0);
  }

  constexpr union command_reply maple_raw(uint32_t send_size, uint32_t recv_size)
  {
    return command_reply(_maple_raw, send_size, recv_size);
  }
}

namespace reply {
  constexpr uint32_t _write = gen_cmd("wrte");
  constexpr uint32_t _read = gen_cmd("read");
  constexpr uint32_t _jump = gen_cmd("jump");
  constexpr uint32_t _speed = gen_cmd("sped");

  constexpr uint32_t _maple_raw = gen_cmd("mprw");

  constexpr uint32_t _crc = gen_cmd("rcrc");

  static_assert(_write == 0x8c661aaa);
  static_assert(_read  == 0x512f23b5);
  static_assert(_jump  == 0x06cb1b4a);
  static_assert(_speed == 0x8705dd86);

  static_assert(_maple_raw  == 0x16865692);

  static_assert(_crc   == 0xcc9aab7c);

  constexpr union command_reply write(uint32_t dest, uint32_t size)
  {
    return command_reply(_write, dest, size);
  }

  constexpr union command_reply read(uint32_t dest, uint32_t size)
  {
    return command_reply(_read, dest, size);
  }

  constexpr union command_reply jump(uint32_t dest)
  {
    return command_reply(_jump, dest, 0);
  }

  constexpr union command_reply speed(uint32_t speed)
  {
    return command_reply(_speed, speed, 0);
  }

  constexpr union command_reply crc(uint32_t crc)
  {
    return command_reply(_crc, crc, 0);
  }

  constexpr union command_reply maple_raw(uint32_t send_size, uint32_t recv_size)
  {
    return command_reply(_maple_raw, send_size, recv_size);
  }
}

}
