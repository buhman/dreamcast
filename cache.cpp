#include "type.h"
#include "sh7091.h"
#include "sh7091_bits.h"

#include "cache.h"

extern volatile reg32 sh7091_ic_a[256][(1 << 5) / 4] __asm("sh7091_ic_a");
extern volatile reg32 sh7091_oc_a[512][(1 << 5) / 4] __asm("sh7091_oc_a");

void cache_init()
{
  for (int i = 0; i < 256; i++) {
    sh7091_ic_a[i][0] = 0;
  }

  for (int i = 0; i < 512; i++) {
    sh7091_oc_a[i][0] = 0;
  }

  sh7091.CCN.CCR = CCR__ICI | CCR__ICE | CCR__OCI | CCR__OCE;

  sh7091.CCN.MMUCR = 0;

  asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;");
}
