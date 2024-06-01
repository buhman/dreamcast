#pragma once

#include <cstdint>

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;

using mat4x4 = mat<4, 4, float>;

struct vertex__texture__normal {
  uint16_t vertex;
  uint16_t texture;
  uint16_t normal;
};

struct vertex__normal {
  uint16_t vertex;
  uint16_t normal;
};

struct vertex {
  uint16_t vertex;
};

struct position__color {
  vec3 position;
  vec3 color;
};

using face_vtn = vertex__texture__normal[3];

using face_vn = vertex__normal[3];

using face_v = vertex[3];
