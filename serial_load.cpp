#include <cstdint>

#include "sh7091/serial.hpp"
#include "sh7091/serial_dma.hpp"
#include "sh7091/cache.hpp"

#include "serial_load.hpp"
#include "crc32.h"

#include "align.hpp"
#include "memory.hpp"

namespace serial_load {

struct state state;

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

static void prestart_write()
{
  uint32_t dest = state.buf.arg[0];
  uint32_t size = state.buf.arg[1];
  serial::recv_dma(dest - 1, size + 1);
  state.reply_crc.value = 0xffffffff;
  state.reply_crc.offset = dest;
}

static void poststart_read()
{
  uint32_t src = state.buf.arg[0];
  uint32_t size = state.buf.arg[1];
  serial::send_dma(src, size);
  state.reply_crc.value = 0xffffffff;
  state.reply_crc.offset = src;
}

static void prestart_maple_raw__command()
{
  uint32_t dest = reinterpret_cast<uint32_t>(&__send_buf);
  uint32_t size = state.buf.arg[0];
  serial::recv_dma(dest - 1, size + 1);
  state.reply_crc.value = 0xffffffff;
  state.reply_crc.offset = dest;
}

static void prestart_maple_raw__response()
{
  uint32_t src = reinterpret_cast<uint32_t>(&__recv_buf);
  uint32_t size = state.buf.arg[1];
  serial::send_dma(src, size);
  state.reply_crc.value = 0xffffffff;
  state.reply_crc.offset = src;
}

  /*
static reply::maple_list_entry maple_list[4 + 4 * 5];

static void prestart_maple_list(struct maple_port * port)
{
  uint32_t function_type = state.buf.arg[0];
  uint32_t list_ix = 0;

  for (int port_ix = 0; port_ix < 4; port_ix++) {

    if ((port[port_ix].device_id.ft & function_type) == 0) {
      continue;
    }

    maple_list[list_ix].port = port_ix;
    maple_list[list_ix].lm = 0;
    maple_list[list_ix]._res = 0;
    maple_list[list_ix].device_id.ft = port[port_ix].device_id.ft;
    maple_list[list_ix].device_id.fd[0] = port[port_ix].device_id.fd[0];
    maple_list[list_ix].device_id.fd[1] = port[port_ix].device_id.fd[1];
    maple_list[list_ix].device_id.fd[2] = port[port_ix].device_id.fd[2];
    list_ix++;

    int bit = 1;
    for (int lm_ix = 0; lm_ix < 5; lm_ix++) {
      if ((port[port_ix].ap__lm & bit) != 0) {
        maple_list[list_ix].port = port_ix;
        maple_list[list_ix].lm = bit;
        maple_list[list_ix]._res = 0;
        maple_list[list_ix].device_id.ft = port[port_ix].lm[lm_ix].device_id.ft;
        maple_list[list_ix].device_id.fd[0] = port[port_ix].lm[lm_ix].device_id.fd[0];
        maple_list[list_ix].device_id.fd[1] = port[port_ix].lm[lm_ix].device_id.fd[1];
        maple_list[list_ix].device_id.fd[2] = port[port_ix].lm[lm_ix].device_id.fd[2];
        list_ix++;
      }
      bit <<= 1;
    }
  }

  state.buf.arg[1] = list_ix * (sizeof (struct reply::maple_list_entry));
}

static void poststart_maple_list()
{
  uint32_t src = reinterpret_cast<uint32_t>(maple_list);
  uint32_t size = state.buf.arg[1];
  serial::send_dma(src, size);
  state.reply_crc.value = 0xffffffff;
  state.reply_crc.offset = src;
}
  */

struct state_arglen_reply command_list[] = {
  {command::_write      , reply::_write      , fsm_state::write               },
  {command::_read       , reply::_read       , fsm_state::read                },
  {command::_jump       , reply::_jump       , fsm_state::jump                },
  {command::_speed      , reply::_speed      , fsm_state::speed               },
  {command::_maple_raw  , reply::_maple_raw  , fsm_state::maple_raw__command  },
};
constexpr uint32_t command_list_length = (sizeof (command_list)) / (sizeof (command_list[0]));

void recv(struct maple_poll_state& poll_state, uint8_t c)
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
	    if (state.buf.cmd == command::_maple_raw) prestart_maple_raw__command();
	    state.fsm_state = sar.fsm_state;
	    state.len = 0;
            union command_reply reply = command_reply(sar.reply, state.buf.arg[0], state.buf.arg[1]);
	    serial::string(reply.u8, 16);

	    if (state.buf.cmd == command::_read) poststart_read();
	    return;
	  } else {
            // do nothing
	  }
	}
      }
      // invalid command
      memory::move(&state.buf.u8[0], &state.buf.u8[1], state.len - 1);
      state.len -= 1;
      break;
    }
  default:
    break;
  }
}

void tick(struct maple_poll_state& poll_state)
{
  switch (state.fsm_state) {
  case fsm_state::idle:
    break;
  case fsm_state::maple_raw__command: [[fallthrough]];
  case fsm_state::write:
    {
      // read chcr1 before dar1 to avoid race
      uint32_t chcr1 = sh7091.DMAC.CHCR1;
      uint32_t dar1 = sh7091.DMAC.DAR1;
      if (dar1 > state.reply_crc.offset) {
	uint32_t len = dar1 - state.reply_crc.offset;
	const uint8_t * buf = reinterpret_cast<const uint8_t *>(state.reply_crc.offset);
        const uint8_t * buf32 = reinterpret_cast<const uint8_t *>((state.reply_crc.offset / 32) * 32); // round down

        // purge operand cache blocks for the data written by DMA, rounding up twice
        for (uint32_t i = 0; i < align_32byte(len) + 32; i += 32) {
          asm volatile ("ocbp @%0"
                        :                                             // output
                        : "r" (reinterpret_cast<uint32_t>(&buf32[i])) // input
                        );
        }

	state.reply_crc.value = crc32_update(state.reply_crc.value, buf, len);
	state.reply_crc.offset += len;
      }
      if (chcr1 & dmac::chcr::te::transfers_completed) {
	state.reply_crc.value ^= 0xffffffff;
	union command_reply reply = reply::crc(state.reply_crc.value);
	serial::string(reply.u8, 16);

	sh7091.DMAC.CHCR1 = 0;

	// transition to next state
        if (state.fsm_state == fsm_state::maple_raw__command) {
          poll_state.send_length = state.buf.arg[0];
          poll_state.recv_length = state.buf.arg[1];
          poll_state.want_raw = 1;
          state.fsm_state = fsm_state::maple_raw__maple_dma;
        } else
          state.fsm_state = fsm_state::idle;
      }
    }
    break;
  case fsm_state::maple_raw__maple_dma:
    {
      // transition to next state
      if (poll_state.want_raw == 0) {
        prestart_maple_raw__response();
        state.fsm_state = fsm_state::maple_raw__response;
      }
    }
    break;
  case fsm_state::maple_raw__response: [[fallthrough]];
  case fsm_state::read:
    {
      // read chcr1 before sar1 to avoid race
      uint32_t chcr1 = sh7091.DMAC.CHCR1;
      uint32_t sar1 = sh7091.DMAC.SAR1;
      if (sar1 > state.reply_crc.offset) {
	uint32_t len = sar1 - state.reply_crc.offset;
	const uint8_t * buf = reinterpret_cast<const uint8_t *>(state.reply_crc.offset);
	state.reply_crc.value = crc32_update(state.reply_crc.value, buf, len);
	state.reply_crc.offset += len;
      }

      if (chcr1 & dmac::chcr::te::transfers_completed) {
	state.reply_crc.value ^= 0xffffffff;
	union command_reply reply = reply::crc(state.reply_crc.value);
	serial::string(reply.u8, 16);

	sh7091.DMAC.CHCR1 = 0;

	// transition to next state
	state.fsm_state = fsm_state::idle;
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

      // clear the cache before jumping
      cache::init();

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
