#pragma once

#ifndef offsetof
#define offsetof(type, member)  __builtin_offsetof (type, member)
#endif

#ifndef bool
#define bool int
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef static_assert
#define static_assert(s) _Static_assert(s, "");
#endif

#ifndef nullptr
#define nullptr ((void *)0)
#endif
