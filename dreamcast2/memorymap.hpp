#pragma once

#include <cstdint>

extern volatile uint8_t system_boot_rom[0x200000] __asm("system_boot_rom") __attribute__((aligned(32)));
extern volatile uint8_t aica_wave_memory[0x200000] __asm("aica_wave_memory") __attribute__((aligned(32)));
extern volatile uint8_t texture_memory64[0x800000] __asm("texture_memory64") __attribute__((aligned(32)));
extern volatile uint8_t texture_memory32[0x800000] __asm("texture_memory32") __attribute__((aligned(32)));
extern volatile uint8_t system_memory[0x1000000] __asm("system_memory") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_polygon_converter[0x800000] __asm("ta_fifo_polygon_converter") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_yuv_converter[0x800000] __asm("ta_fifo_yuv_converter") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_texture_memory[0x800000] __asm("ta_fifo_texture_memory") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_polygon_converter_mirror[0x800000] __asm("ta_fifo_polygon_converter_mirror") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_yuv_converter_mirror[0x800000] __asm("ta_fifo_yuv_converter_mirror") __attribute__((aligned(32)));
extern volatile uint8_t ta_fifo_texture_memory_mirror[0x800000] __asm("ta_fifo_texture_memory_mirror") __attribute__((aligned(32)));
extern volatile uint8_t store_queue[0x4000000] __asm("store_queue") __attribute__((aligned(32)));
extern volatile uint8_t sh7091_oc_d[0x1000] __asm("sh7091_oc_d") __attribute__((aligned(32)));
