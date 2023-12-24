#pragma once

#include <cstdint>

#include "math/vec3.hpp"
#include "math/vec4.hpp"

using vec3 = vec<3, float>;
using vec4 = vec<4, float>;

struct vertex__normal {
  uint16_t vertex;
  uint16_t normal;
};

using face = vertex__normal[3];
