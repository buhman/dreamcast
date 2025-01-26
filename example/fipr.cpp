#include "stdint.h"

#include "sh7091/serial.hpp"

extern "C" float fipr(float * a, float * b);

void main()
{
  float a[] = {1, 2, 3, 4};
  float b[] = {5, 6, 7, 8};

  // 70

  union {
    float f;
    uint32_t i;
  } v;

  v.f = fipr(a, b);
  serial::integer(v.i);
  serial::integer(v.i);
  serial::integer(v.i);
  serial::integer(v.i);
}
