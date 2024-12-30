#include "sh7091/serial.hpp"

int main()
{
  serial::init(0);

  while (1) {
    //*(uint32_t *)(0xa0620000) = 0xffffffff;
    *(uint8_t *)(0xa0600480) = 0;
    for (int i = 0; i < 30; i++) {
      asm volatile ("nop");
    }

    *(uint8_t *)(0xa0600480) = 1;
    for (int i = 0; i < 70; i++) {
      asm volatile ("nop");
    }

    *(uint8_t *)(0xa0600480) = 0;
    for (int i = 0; i < 120; i++) {
      asm volatile ("nop");
    }
    serial::character('.');
  }
}
