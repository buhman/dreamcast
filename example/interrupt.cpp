#include <cstdint>

#include "sh7091/serial.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

void main()
{
  uint32_t vbr;

  asm ("stc vbr,%0" : "=r" (vbr));

  serial::integer<uint32_t>(vbr);

  while (1);
}
