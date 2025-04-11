#pragma once

#include <stdint.h>

#include "math/vec3.hpp"
#include "math/vec2.hpp"

#ifdef __dreamcast__
using vertex_position = vec<3, float>;
using vertex_normal = vec<3, float>;
using vertex_texture = vec<2, float>;
#endif
#ifdef __saturn__
#include "math/fp.hpp"
using vertex_position = vec<3, fp16_16>;
using vertex_normal = vec<3, fp16_16>;
using vertex_texture = vec<2, fp16_16>;
#endif

struct index_ptn {
  uint16_t position;
  uint16_t texture;
  uint16_t normal;
};

union triangle {
  struct {
    struct index_ptn a;
    struct index_ptn b;
    struct index_ptn c;
  };
  struct index_ptn v[3];
};

union quadrilateral {
  struct {
    struct index_ptn a;
    struct index_ptn b;
    struct index_ptn c;
    struct index_ptn d;
  };
  struct index_ptn v[4];
};

struct object {
  const union triangle * triangle;
  const union quadrilateral * quadrilateral;
  const int triangle_count;
  const int quadrilateral_count;
  const int material;
};

struct model {
  const vertex_position * position;
  const vertex_texture * texture;
  const vertex_normal * normal;
  const struct object ** object;
  const int object_count;
};
