#include <stdint.h>

#include "sh7091/serial.hpp"
#include "memorymap.hpp"

void main()
{
  serial::init(0);

  serial::string("start\n");

  for (int i = 0; i < 8388608 / 4; i++) {
    texture_memory64[i] = (1 << 31) | i;
  }

  serial::string("done\n");
  serial::string("done\n");
  serial::string("done\n");
  serial::string("done\n");
}
