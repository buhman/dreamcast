#include <cstdint>

#include "sh7091/cache.hpp"

extern uint32_t __bss_link_start __asm("__bss_link_start");
extern uint32_t __bss_link_end __asm("__bss_link_end");

extern void main();

extern "C"
void runtime_init()
__attribute__((section(".text.startup.runtime_init")));

extern "C"
void runtime_init()
{
  // clear BSS
  uint32_t * start = &__bss_link_start;
  uint32_t * end = &__bss_link_end;
  while (start < end) {
    *start++ = 0;
  }

  cache::init();

  main();
}
