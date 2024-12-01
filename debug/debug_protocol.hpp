#pragma once

#include <stdint.h>
#include <stddef.h>

namespace debug_protocol {

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

template <typename T>
struct frame {
  uint32_t command;
  uint32_t sequence;
  T data;
  uint32_t crc;
};

static_assert((sizeof (frame<char[0]>)) == 4 * 3);
static_assert((offsetof (frame<char[0]>, crc)) == 4 * 2);

struct register_state {
  uint32_t r[16];
  uint32_t sr;
  uint32_t gbr;
  uint32_t mach;
  uint32_t macl;
  uint32_t pr;
  uint32_t pc;
};

struct read_u32 {
  uint32_t start;
  uint32_t length;
};

namespace command {
  constexpr uint32_t _register_state = gen_cmd("REGS");
  constexpr uint32_t _read_u32 = gen_cmd("RU32");
}

namespace reply {
  constexpr uint32_t _register_state = gen_cmd("regs");
  constexpr uint32_t _read_u32 = gen_cmd("ru32");

  constexpr uint32_t _acknowledge = gen_cmd("ackn");
}

}
