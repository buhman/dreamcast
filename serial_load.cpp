#include <cstdint>

#include "sh7091/serial.hpp"
#include "sh7091/serial_dma.hpp"

#include "serial_load.hpp"
#include "crc32.h"

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

void init(uint32_t speed)
{
  state.len = 0;
  state.fsm_state = fsm_state::idle;
  state.speed = speed & 0xff;
  sh7091.DMAC.CHCR1 = 0;
  serial::init(state.speed);
}

void jump_to_func(const uint32_t addr)
{
  serial::string("jump to: ");
  serial::integer<uint32_t>(addr);
  // save our stack
  asm volatile ("mov.l r14,@-r15; "
		"ldc r15, gbr; "
		"jsr @%0; "
		"mov #0, r15; "
		"stc gbr, r15; "
		"mov.l @r15+,r14; "
                :
                : "r"(addr) /* input */
	        /* clobbered register */
                : "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","macl","mach","gbr","pr"
                );
  // restore our stack
}

static inline void prestart_write()
{
  uint32_t dest = state.buf.arg[0];
  uint32_t size = state.buf.arg[1];
  serial::recv_dma(dest - 1, size + 1);
  state.write_crc.value = 0xffffffff;
  state.write_crc.offset = dest;
}

static inline void prestart_read()
{
  uint32_t src = state.buf.arg[0];
  uint32_t size = state.buf.arg[1];
  serial::send_dma(src, size);
  state.read_crc.value = 0xffffffff;
  state.read_crc.offset = src;
}

struct state_arglen_reply command_list[] = {
  {command::_write, reply::_write, fsm_state::write},
  {command::_read , reply::_read , fsm_state::read },
  {command::_jump , reply::_jump , fsm_state::jump },
  {command::_speed, reply::_speed, fsm_state::speed},
};
constexpr uint32_t command_list_length = (sizeof (command_list)) / (sizeof (command_list[0]));

void recv(uint8_t c)
{
  state.buf.u8[state.len++] = c;
  switch (state.fsm_state) {
  case fsm_state::idle:
    if (state.len == 16) {
      for (uint32_t i = 0; i < command_list_length; i++) {
	struct state_arglen_reply& sar = command_list[i];
	if (state.buf.cmd == sar.command) {
	  uint32_t crc = crc32(&state.buf.u8[0], 12);
	  if (crc == state.buf.crc) {
	    // valid command, do the transition
	    if (state.buf.cmd == command::_write) prestart_write();
	    state.fsm_state = sar.fsm_state;
	    state.len = 0;
	    union command_reply reply = command_reply(sar.reply, state.buf.arg[0], state.buf.arg[1]);
	    serial::string(reply.u8, 16);

	    if (state.buf.cmd == command::_read) prestart_read();
	    return;
	  } else {
            // do nothing
	  }
	}
      }
      // invalid command
      move(&state.buf.u8[0], &state.buf.u8[1], state.len - 1);
      state.len -= 1;
      break;
    }
  default:
    break;
  }
}

void tick()
{
  switch (state.fsm_state) {
  case fsm_state::idle:
    break;
  case fsm_state::write:
    {
      // read chcr1 before dar1 to avoid race
      uint32_t chcr1 = sh7091.DMAC.CHCR1;
      uint32_t dar1 = sh7091.DMAC.DAR1;
      if (dar1 > state.write_crc.offset) {
	uint32_t len = dar1 - state.write_crc.offset;
	const uint8_t * buf = reinterpret_cast<const uint8_t *>(state.write_crc.offset);
	state.write_crc.value = crc32_update(state.write_crc.value, buf, len);
	state.write_crc.offset += len;
      }
      if (chcr1 & dmac::chcr::te::transfers_completed) {
	state.write_crc.value ^= 0xffffffff;
	union command_reply reply = reply::write_crc(state.write_crc.value);
	serial::string(reply.u8, 16);

	sh7091.DMAC.CHCR1 = 0;

	// transition to next state
	state.fsm_state = fsm_state::idle;
      }
    }
    break;
  case fsm_state::read:
    {
      uint32_t chcr1 = sh7091.DMAC.CHCR1;
      uint32_t sar1 = sh7091.DMAC.SAR1;
      if (sar1 > state.read_crc.offset) {
	uint32_t len = sar1 - state.read_crc.offset;
	const uint8_t * buf = reinterpret_cast<const uint8_t *>(state.read_crc.offset);
	state.read_crc.value = crc32_update(state.read_crc.value, buf, len);
	state.read_crc.offset += len;
      }

      if (chcr1 & dmac::chcr::te::transfers_completed) {
	state.read_crc.value ^= 0xffffffff;
	union command_reply reply = reply::read_crc(state.read_crc.value);
	serial::string(reply.u8, 16);

	sh7091.DMAC.CHCR1 = 0;

	// transition to next state
	//state.fsm_state = fsm_state::idle;
      }
    }
    break;
  case fsm_state::jump:
    {
      using namespace scif;
      // wait for serial transmission to end
      constexpr uint32_t transmission_end = scfsr2::tend::bit_mask | scfsr2::tdfe::bit_mask;
      if ((sh7091.SCIF.SCFSR2 & transmission_end) != transmission_end)
	return;

      const uint32_t dest = state.buf.arg[0];
      jump_to_func(dest);

      // cautiously re-initialize serial; it is possible the called
      // function modified serial state
      sh7091.DMAC.CHCR1 = 0;
      serial::init(state.speed);

      // transition to next state
      state.fsm_state = fsm_state::idle;
    }
    break;
  case fsm_state::speed:
    {
      using namespace scif;
      // wait for serial transmission to end
      constexpr uint32_t transmission_end = scfsr2::tend::bit_mask | scfsr2::tdfe::bit_mask;
      if ((sh7091.SCIF.SCFSR2 & transmission_end) != transmission_end)
	return;

      const uint32_t speed = state.buf.arg[0];
      state.speed = speed & 0xff;
      serial::init(state.speed);

      // transition to next state
      state.fsm_state = fsm_state::idle;
    }
    break;
  default:
    break;
  }
}

}
