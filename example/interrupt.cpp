#include <cstdint>

#include "serial.hpp"

#include "sh7095.hpp"
#include "sh7095_bits.hpp"

void main()
{
  uint32_t vbr;

  asm ("stc vbr,%0" : "=r" (vbr));

  serial::integer<uint32_t>(vbr);

  while (1);
}
