#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
#ifndef static_assert
#define static_assert _Static_assert
#endif
#endif

typedef volatile uint8_t reg8;
typedef volatile uint16_t reg16;
typedef volatile uint32_t reg32;

static_assert((sizeof (reg8)) == 1);
static_assert((sizeof (reg16)) == 2);
static_assert((sizeof (reg32)) == 4);
