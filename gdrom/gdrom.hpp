#include <cstdint>
#include <cstddef>

#include "type.hpp"

struct gdrom_if_reg {
  const reg8  _pad0[24];
  union {
    const reg8  alternate_status;
    reg8  device_control;
  };
  const reg8  _pad1[103];
  reg16 data;
  const reg8  _pad2[2];
  union {
    const reg8  error;
    reg8  features;
  };
  const reg8  _pad3[3];
  union {
    const reg8  interrupt_reason;
    reg8  sector_count;
  };
  const reg8  _pad4[3];
  reg8  sector_number;
  const reg8  _pad5[3];
  reg8  byte_control_low;
  const reg8  _pad6[3];
  reg8  byte_control_high;
  const reg8  _pad7[3];
  reg8  drive_select;
  const reg8  _pad8[3];
  union {
    const reg8  status;
    reg8  command;
  };
};

static_assert((offsetof (struct gdrom_if_reg, alternate_status)) == 24);
static_assert((offsetof (struct gdrom_if_reg, device_control)) == 24);
static_assert((offsetof (struct gdrom_if_reg, data)) == 128);
static_assert((offsetof (struct gdrom_if_reg, error)) == 132);
static_assert((offsetof (struct gdrom_if_reg, features)) == 132);
static_assert((offsetof (struct gdrom_if_reg, interrupt_reason)) == 136);
static_assert((offsetof (struct gdrom_if_reg, sector_count)) == 136);
static_assert((offsetof (struct gdrom_if_reg, sector_number)) == 140);
static_assert((offsetof (struct gdrom_if_reg, byte_control_low)) == 144);
static_assert((offsetof (struct gdrom_if_reg, byte_control_high)) == 148);
static_assert((offsetof (struct gdrom_if_reg, drive_select)) == 152);
static_assert((offsetof (struct gdrom_if_reg, status)) == 156);
static_assert((offsetof (struct gdrom_if_reg, command)) == 156);

extern struct gdrom_if_reg gdrom_if __asm("gdrom_if");
