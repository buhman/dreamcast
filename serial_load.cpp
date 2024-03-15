#include <cstdint>

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "holly/holly.hpp"

enum load_command {
  CMD_NONE,
  CMD_DATA, // DATA 0000 0000 {data}
  CMD_JUMP, // JUMP 0000
  CMD_RATE, // RATE 0000
};

struct load_state {
  union {
    uint8_t buf[12];
    struct {
      uint8_t cmd[4];
      uint32_t addr1;
      uint32_t addr2;
    };
  };
  uint32_t len;
  enum load_command command;
};

static struct load_state state;

void move(void *dst, const void *src, size_t n)
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

void load_init()
{
  state.len = 0;
  state.command = CMD_NONE;
}

void debug(const char * s)
{
  char c;
  while ((sh7091.SCIF.SCFSR2 & scif::scfsr2::tdfe::bit_mask) == 0);
  while ((c = *s++)) {
    sh7091.SCIF.SCFTDR2 = (uint8_t)c;
  }
}

void jump_to_func(const uint32_t addr)
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

void load_recv(uint8_t c)
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
	    debug("data\n");
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
	    debug("jump\n");
	    state.command = CMD_JUMP;
	  }
	} else if (state.buf[0] == 'R' &&
		   state.buf[1] == 'A' &&
		   state.buf[2] == 'T' &&
		   state.buf[3] == 'E') {
	  if (state.len < 8) {
	    return;
	  } else {
	    debug("rate\n");
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
	  sh7091.SCIF.SCFTDR2 = c;

	  // write c to dest
	  *dest = c;
	  state.addr2++;

	  (*size)--;
	}
	if (*size == 0) {
	  state.len = 0;
	  state.command = CMD_NONE;
	  debug("next\n");
	}
	return;
	break;
      }
    case CMD_JUMP:
      // jump
      state.len = 0;
      state.command = CMD_NONE;
      debug("prejump\n");
      holly.VO_BORDER_COL = (31 << 11);
      jump_to_func(state.addr1);
      holly.VO_BORDER_COL = (63 << 5) | (31 << 0);
      debug("postjump\n");
      return;
      break;
    case CMD_RATE:
      state.len = 0;
      state.command = CMD_NONE;
      debug("prerate\n");
      serial::init(state.addr1 & 0xff);
      debug("postrate\n");
      return;
      break;
    }
  }
}
