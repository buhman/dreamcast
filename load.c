#include <stdint.h>

#include "sh7091.h"
#include "holly.h"

enum load_command {
  CMD_NONE,
  CMD_DATA, // DATA 0000 0000 {data}
  CMD_JUMP, // JUMP 0000
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

void move(void *dest, const void *src, size_t n)
{
  uint8_t *d = dest;
  const uint8_t *s = src;

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
  while ((c = *s++)) {
    SH7091.SCIF.SCFTDR2 = (uint8_t)c;
  }
}

void load_recv(uint8_t c)
{
  if (state.command == CMD_NONE)
    state.buf[state.len++] = c;

  while (1) {
    switch (state.command) {
    case CMD_NONE:
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
	} else {
	  move(&state.buf[0], &state.buf[4], state.len - 4);
	  state.len -= 4;
	}
      } else {
	return;
      }
      break;
    case CMD_DATA:
      {
	uint32_t * size = &state.addr1;
	uint8_t * dest = (uint8_t *)state.addr2;
	if (size > 0) {
	  SH7091.SCIF.SCFTDR2 = c;

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
	case CMD_JUMP:
	  // jump
	  state.len = 0;
	  state.command = CMD_NONE;
	  debug("prejump\n");
	  HOLLY.VO_BORDER_COL = (31 << 11);
	  void (*fptr)(void) = (void (*)(void))state.addr1;
	  HOLLY.VO_BORDER_COL = (63 << 5) | (31 << 0);
	  fptr();
	  debug("postjump\n");
	  return;
	  break;
      }
    }
  }
}
