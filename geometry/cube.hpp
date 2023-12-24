#pragma once

#include <cstdint>

#include "math/vec3.hpp"

using vec3 = vec<3, float>;

struct vertex__normal {
  uint8_t vertex;
  uint8_t normal;
};

using face = vertex__normal[3];

namespace cube {

  constexpr vec3 vertices[] = {
    { 1.f,  1.f, -1.f},
    { 1.f, -1.f, -1.f},
    { 1.f,  1.f,  1.f},
    { 1.f, -1.f,  1.f},
    {-1.f,  1.f, -1.f},
    {-1.f, -1.f, -1.f},
    {-1.f,  1.f,  1.f},
    {-1.f, -1.f,  1.f},
  };

  constexpr vec3 normals[] = {
    {-0.f,  1.f, -0.f},
    {-0.f, -0.f,  1.f},
    {-1.f, -0.f, -0.f},
    {-0.f, -1.f, -0.f},
    { 1.f, -0.f, -0.f},
    {-0.f, -0.f, -1.f},
  };

  constexpr face faces[] = {
    {{5, 1}, {3, 1}, {1, 1}},
    {{3, 2}, {8, 2}, {4, 2}},
    {{7, 3}, {6, 3}, {8, 3}},
    {{2, 4}, {8, 4}, {6, 4}},
    {{1, 5}, {4, 5}, {2, 5}},
    {{5, 6}, {2, 6}, {6, 6}},
    {{5, 1}, {7, 1}, {3, 1}},
    {{3, 2}, {7, 2}, {8, 2}},
    {{7, 3}, {5, 3}, {6, 3}},
    {{2, 4}, {4, 4}, {8, 4}},
    {{1, 5}, {3, 5}, {4, 5}},
    {{5, 6}, {1, 6}, {2, 6}},
  };

  constexpr int num_faces = (sizeof (faces)) / (sizeof (face));
}
