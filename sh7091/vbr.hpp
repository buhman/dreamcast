#pragma once

#include <cstdint>

__attribute__((section(".vbr.100")))
__attribute__((interrupt_handler))
void vbr100();

__attribute__((section(".vbr.400")))
__attribute__((interrupt_handler))
void vbr400();

__attribute__((section(".vbr.600")))
__attribute__((interrupt_handler))
void vbr600();

extern uint32_t __vbr_link_start __asm("__vbr_link_start");
