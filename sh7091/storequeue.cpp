#include "sh7091.hpp"
#include "memorymap.hpp"

void sq_transfer_32byte(volatile void * dst)
{
  // dst typically 0x10000000 (ta polygon converter)
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(dst) >> 26) & 0b111) << 2;

  // start 32-byte transfer from store queue 0 (SQ0) to QACR0
  asm volatile ("pref @%0"
		:                       // output
		: "r" (&store_queue[0]) // input
		);
}


void sq_transfer_64byte(volatile void * dst)
{
  // dst typically 0x10000000 (ta polygon converter)
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(dst) >> 26) & 0b111) << 2;
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(dst) >> 26) & 0b111) << 2;

  // start 32-byte transfer from store queue 0 (SQ0) to QACR0
  asm volatile ("pref @%0"
		:                       // output
		: "r" (&store_queue[0]) // input
		);

  // start 32-byte transfer from store queue 1 (SQ1) to QACR1
  asm volatile ("pref @%0"
		:                       // output
		: "r" (&store_queue[8]) // input
		);
}
