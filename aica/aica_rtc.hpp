#include <cstdint>
#include <cstddef>

#include "type.hpp"

struct aica_rtc {
  reg16 reg_0000;
  const reg16 _pad0[1];
  reg16 reg_0004;
  const reg16 _pad1[1];
  reg16 reg_0008;
  const reg16 _pad2[1];

  uint32_t RTC() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 0) & 0xffff) << 16) | (static_cast<uint32_t>((reg_0004 >> 0) & 0xffff) << 0);
  }
  void RTC(const uint32_t v)
  {
    reg_0000 = (((v >> 16) & 0xffff) << 0);
    reg_0004 = (((v >> 0) & 0xffff) << 0);
  }

  uint32_t EN() const
  {
    return (static_cast<uint32_t>((reg_0008 >> 0) & 0x1) << 0);
  }
  void EN(const uint32_t v)
  {
    reg_0008 = (((v >> 0) & 0x1) << 0);
  }

};

static_assert((sizeof (aica_rtc)) == 0xc - 0x0);
static_assert((offsetof (aica_rtc, reg_0000)) == 0x0 - 0x0);
static_assert((offsetof (aica_rtc, reg_0004)) == 0x4 - 0x0);
static_assert((offsetof (aica_rtc, reg_0008)) == 0x8 - 0x0);
