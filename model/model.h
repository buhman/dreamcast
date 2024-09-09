#pragma once

#include <stdint.h>

#include "math/vec3.hpp"
#include "math/vec2.hpp"

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

using vertex_position = vec<3, float>;
using vertex_normal = vec<3, float>;
using vertex_texture = vec<2, float>;

struct object {
  union triangle * triangle;
  union quadrilateral * quadrilateral;
  int triangle_count;
  int quadrilateral_count;

  int material;
};

struct model {
  vertex_position * position;
  vertex_texture * texture;
  vertex_normal * normal;

  struct object ** object;
  int object_count;
};
