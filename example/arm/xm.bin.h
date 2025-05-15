#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _binary_example_arm_xm_bin_start __asm("_binary_example_arm_xm_bin_start");
extern uint32_t _binary_example_arm_xm_bin_end __asm("_binary_example_arm_xm_bin_end");
extern uint32_t _binary_example_arm_xm_bin_size __asm("_binary_example_arm_xm_bin_size");

#ifdef __cplusplus
}
#endif
