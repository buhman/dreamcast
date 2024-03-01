#pragma once

#include <stdint.h>
#include <stddef.h>

#include "type.hpp"

#include "aica_channel.hpp"
#include "aica_dsp_out.hpp"
#include "aica_common.hpp"
#include "aica_rtc.hpp"

struct aica_sound {
  // 0x00700000 [64] channel data start
  // 0x00702000 [64] channel data end
  // 0x00702000 [18] dsp out start
  // 0x00702048 [18] dsp out end
  // padding 0x7b8
  // 0x00702800 common data start

  struct aica_channel channel[64];
  struct aica_dsp_out dsp_out[18];
  const uint16_t _pad[0x7b8 / 2];
  struct aica_common common;
};

static_assert((sizeof (struct aica_sound)) == 0x2d08);
static_assert((offsetof (struct aica_sound, channel)) == 0);
static_assert((offsetof (struct aica_sound, dsp_out)) == 0x2000);
static_assert((offsetof (struct aica_sound, common)) == 0x2800);

extern struct aica_sound aica_sound __asm("aica_sound");
extern struct aica_rtc aica_rtc __asm("aica_rtc");
