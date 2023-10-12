#include "type.h"
#include "sh7091.h"
#include "sh7091_bits.h"

#include "cache.h"

extern volatile reg32 SH7091_IC_A[256][(1 << 5) / 4] __asm("SH7091_IC_A");
extern volatile reg32 SH7091_OC_A[512][(1 << 5) / 4] __asm("SH7091_OC_A");

void cache_init()
{
  for (int i = 0; i < 256; i++) {
    SH7091_IC_A[i][0] = 0;
  }

  for (int i = 0; i < 512; i++) {
    SH7091_OC_A[i][0] = 0;
  }

  SH7091.CCN.CCR = CCR__ICI | CCR__ICE | CCR__OCI | CCR__OCE;

  SH7091.CCN.MMUCR = 0;

  asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;");
}
