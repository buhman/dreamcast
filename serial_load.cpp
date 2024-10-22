#include <cstdint>

#include "sh7091/serial.hpp"
#include "serial_load.hpp"

namespace serial_load {

struct state state;

static void move(void *dst, const void *src, uint32_t n)
{
  uint8_t * d = reinterpret_cast<uint8_t *>(dst);
  const uint8_t * s = reinterpret_cast<const uint8_t *>(src);

  if (d==s) return;
  if (d<s) {
    for (; n; n--) *d++ = *s++;
  } else {
    while (n) n--, d[n] = s[n];
  }
}

void init()
{
  state.len = 0;
  state.command = CMD_NONE;
}

static void jump_to_func(const uint32_t addr)
{
  serial::string("jump to: ");
  serial::integer<uint32_t>(addr);
  // save our stack
  asm volatile ("ldc r15, gbr; "
		"mov #0, r15; "
		"jsr @%0; "
		"nop; "
		"stc gbr, r15; "
                :
                : "r"(addr) /* input */
	        /* clobbered register */
                : "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","macl","mach","gbr","pr"
                );
  // restore our stack
}

void recv(uint8_t c)
{
  while (1) {
    switch (state.command) {
    case CMD_NONE:
      state.buf[state.len++] = c;
      if (state.len >= 4) {
	if (state.buf[0] == 'D' &&
	    state.buf[1] == 'A' &&
	    state.buf[2] == 'T' &&
	    state.buf[3] == 'A') {
	  if (state.len < 12) {
	    return;
	  } else {
	    serial::string("data");
	    state.command = CMD_DATA;
	    return;
	  }
	} else if (state.buf[0] == 'J' &&
		   state.buf[1] == 'U' &&
		   state.buf[2] == 'M' &&
		   state.buf[3] == 'P') {
	  if (state.len < 8) {
	    return;
	  } else {
	    serial::string("jump");
	    state.command = CMD_JUMP;
	  }
	} else if (state.buf[0] == 'R' &&
		   state.buf[1] == 'A' &&
		   state.buf[2] == 'T' &&
		   state.buf[3] == 'E') {
	  if (state.len < 8) {
	    return;
	  } else {
	    serial::string("rate");
	    state.command = CMD_RATE;
	  }
	} else {
	  move(&state.buf[0], &state.buf[1], state.len - 1);
	  state.len -= 1;
	}
      } else {
	return;
      }
      break;
    case CMD_DATA:
      {
	uint32_t * size = &state.addr1;
	uint8_t * dest = reinterpret_cast<uint8_t *>(state.addr2);
	if (*size > 0) {
	  serial::character(c);

	  // write c to dest
	  *dest = c;
	  state.addr2++;

	  (*size)--;
	}
	if (*size == 0) {
	  state.len = 0;
	  state.command = CMD_NONE;
	  serial::string("next");
	}
	return;
	break;
      }
    case CMD_JUMP:
      // jump
      state.len = 0;
      state.command = CMD_NONE;
      jump_to_func(state.addr1);
      return;
      break;
    case CMD_RATE:
      state.len = 0;
      state.command = CMD_NONE;
      serial::init(state.addr1 & 0xff);
      return;
      break;
    }
  }
}

}
