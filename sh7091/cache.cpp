#include "type.hpp"
#include "sh7091.hpp"
#include "sh7091_bits.hpp"

#include "cache.hpp"

extern volatile reg32 sh7091_ic_a[256][(1 << 5) / 4] __asm("sh7091_ic_a");
extern volatile reg32 sh7091_oc_a[512][(1 << 5) / 4] __asm("sh7091_oc_a");

namespace cache {

void init()
{
  for (int i = 0; i < 256; i++) {
    // clear all V bits of the IC address array
    sh7091_ic_a[i][0] = 0;
  }

  for (int i = 0; i < 512; i++) {
    // clear all V bits of the OC address array
    sh7091_oc_a[i][0] = 0;
  }

  using namespace ccn::ccr;

  sh7091.CCN.CCR = ici::clear_v_bits_of_all_ic_entries       // instruction cache invalidate
                 | ice::ic_used                              // instruction cache enable
                 | oci::clear_v_and_u_bits_of_all_oc_entries // operand cache invalidate
                 | oce::oc_used                              // operand cache enable
                 | cb::copy_back_mode                        // enable copy-back mode for the P1 area
                 ;

  sh7091.CCN.MMUCR = ccn::mmucr::at::mmu_disabled;

  asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;");
}

}
