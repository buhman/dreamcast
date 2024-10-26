#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t crc32_table[256];

uint32_t
crc32_update(uint32_t crc, const uint8_t *data, uint32_t len);

uint32_t
crc32(const uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif
