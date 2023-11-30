#include <stdint.h>

extern volatile uint32_t system_boot_rom[0x200000] __asm("system_boot_rom");
extern volatile uint32_t aica_wave_memory[0x200000] __asm("aica_wave_memory");
extern volatile uint32_t texture_memory[0x800000] __asm("texture_memory");
extern volatile uint32_t system_memory[0x1000000] __asm("system_memory");
extern volatile uint32_t ta_fifo_polygon_converter[0x800000] __asm("ta_fifo_polygon_converter");
extern volatile uint32_t ta_fifo_yuv_converter[0x800000] __asm("ta_fifo_yuv_converter");
extern volatile uint32_t ta_fifo_texture_memory[0x800000] __asm("ta_fifo_texture_memory");
extern volatile uint32_t ta_fifo_polygon_converter_mirror[0x800000] __asm("ta_fifo_polygon_converter_mirror");
extern volatile uint32_t ta_fifo_yuv_converter_mirror[0x800000] __asm("ta_fifo_yuv_converter_mirror");
extern volatile uint32_t ta_fifo_texture_memory_mirror[0x800000] __asm("ta_fifo_texture_memory_mirror");
extern volatile uint32_t store_queue[0x4000000] __asm("store_queue");
