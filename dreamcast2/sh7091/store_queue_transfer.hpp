#pragma once

#include "sh7091/sh7091.hpp"
#include "memorymap.hpp"

namespace sh7091 {
  namespace store_queue_transfer {

    static inline void copy(void * out_addr,
                            const void * src,
                            int length) __attribute__((always_inline));

    static inline void copy(void * out_addr,
                            const void * src,
                            int length)
    {
      uint32_t out = reinterpret_cast<uint32_t>(out_addr);
      sh7091.CCN.QACR0 = ((out >> 24) & 0b11100);
      sh7091.CCN.QACR1 = ((out >> 24) & 0b11100);

      volatile uint32_t * base = (volatile uint32_t *)&store_queue[(out & 0x03ffffe0)];
      const uint32_t * src32 = reinterpret_cast<const uint32_t *>(src);

      length = (length + 31) & ~31; // round up to nearest multiple of 32
      while (length > 0) {
        base[0] = src32[0];
        base[1] = src32[1];
        base[2] = src32[2];
        base[3] = src32[3];
        base[4] = src32[4];
        base[5] = src32[5];
        base[6] = src32[6];
        base[7] = src32[7];
        asm volatile ("pref @%0"
                      :                // output
                      : "r" (&base[0]) // input
                      : "memory");
        length -= 32;
        base += 8;
        src32 += 8;
      }
    }


    static inline void zeroize(void * out_addr,
                               int length,
                               const uint32_t value) __attribute__((always_inline));

    static inline void zeroize(void * out_addr,
                               int length,
                               const uint32_t value)
    {
      uint32_t out = reinterpret_cast<uint32_t>(out_addr);
      sh7091.CCN.QACR0 = ((out >> 24) & 0b11100);
      sh7091.CCN.QACR1 = ((out >> 24) & 0b11100);

      volatile uint32_t * base = (volatile uint32_t *)&store_queue[(out & 0x03ffffe0)];

      length = (length + 31) & ~31; // round up to nearest multiple of 32
      while (length > 0) {
        base[0] = value;
        base[1] = value;
        base[2] = value;
        base[3] = value;
        base[4] = value;
        base[5] = value;
        base[6] = value;
        base[7] = value;
        asm volatile ("pref @%0"
                      :                // output
                      : "r" (&base[0]) // input
                      : "memory");
        length -= 32;
        base += 8;
      }
    }
  }
}
