#include <stdint.h>
#include <stddef.h>

#include "type.hpp"

struct aica_dsp_out {
  reg32 reg_0000;

  uint32_t EFDSL() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 8) & 0xf) << 0);
  }
  void EFDSL(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0xf) << 8) | (reg_0000 & 0xf0ff);
  }

  uint32_t EFPAN() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 0) & 0x1f) << 0);
  }
  void EFPAN(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x1f) << 0) | (reg_0000 & 0xffe0);
  }

};

static_assert((sizeof (aica_dsp_out)) == 0x4 - 0x0);
static_assert((offsetof (aica_dsp_out, reg_0000)) == 0x0 - 0x0);
