#pragma once

#include <cstdint>

inline void set_imask(uint32_t imask)
{
  uint32_t sr;

  asm volatile ("stc sr,%0"
		: "=r" (sr)
		:
		);

  sr = (sr & ~0xf0) | (imask << 4);

  asm volatile ("ldc %0,sr"
		:
		: "r" (sr)
		);
}
