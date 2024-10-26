#pragma once

#include <stdint.h>

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

namespace command {
  constexpr uint32_t write = gen_cmd("WRTE");
  constexpr uint32_t read = gen_cmd("READ");
  constexpr uint32_t jump = gen_cmd("JUMP");
  constexpr uint32_t speed = gen_cmd("SPED");

  static_assert(write == 0x2cc46ed8);
  static_assert(read == 0xf18d57c7);
  static_assert(jump == 0xa6696f38);
  static_assert(speed == 0x27a7a9f4);
}

namespace reply {
  constexpr uint32_t write = gen_cmd("wrte");
  constexpr uint32_t read = gen_cmd("read");
  constexpr uint32_t jump = gen_cmd("jump");
  constexpr uint32_t speed = gen_cmd("sped");
  constexpr uint32_t write_crc = gen_cmd("crcW");
  constexpr uint32_t read_crc = gen_cmd("crcR");

  static_assert(write == 0x8c661aaa);
  static_assert(read  == 0x512f23b5);
  static_assert(jump  == 0x06cb1b4a);
  static_assert(speed == 0x8705dd86);
  static_assert(write_crc == 0x1cced074);
  static_assert(read_crc == 0xb9ce82f4);
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

constexpr union command_reply write_command(uint32_t dest, uint32_t size)
{
  return command_reply(command::write, dest, size);
}

constexpr union command_reply read_command(uint32_t dest, uint32_t size)
{
  return command_reply(command::read, dest, size);
}

constexpr union command_reply jump_command(uint32_t dest)
{
  return command_reply(command::jump, dest, 0);
}

constexpr union command_reply speed_command(uint32_t speed)
{
  return command_reply(command::speed, speed, 0);
}

constexpr union command_reply write_reply(uint32_t dest, uint32_t size)
{
  return command_reply(reply::write, dest, size);
}

constexpr union command_reply read_reply(uint32_t dest, uint32_t size)
{
  return command_reply(reply::read, dest, size);
}

constexpr union command_reply jump_reply(uint32_t dest)
{
  return command_reply(reply::jump, dest, 0);
}

constexpr union command_reply speed_reply(uint32_t speed)
{
  return command_reply(reply::speed, speed, 0);
}

constexpr union command_reply write_crc_reply(uint32_t crc)
{
  return command_reply(reply::write_crc, crc, 0);
}

constexpr union command_reply read_crc_reply(uint32_t crc)
{
  return command_reply(reply::read_crc, crc, 0);
}

}
