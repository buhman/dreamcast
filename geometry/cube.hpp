#pragma once

#include "geometry.hpp"

namespace cube {
  constexpr vec3 vertices[] = {
    { -1.000000f, -1.000000f,  1.000000f },
    { -1.000000f,  1.000000f,  1.000000f },
    { -1.000000f, -1.000000f, -1.000000f },
    { -1.000000f,  1.000000f, -1.000000f },
    {  1.000000f, -1.000000f,  1.000000f },
    {  1.000000f,  1.000000f,  1.000000f },
    {  1.000000f, -1.000000f, -1.000000f },
    {  1.000000f,  1.000000f, -1.000000f },
  };

  constexpr vec2 texture[] = {
    {  1.000000f,  1.000000f },
    {  1.000000f,  0.000000f },
    {  0.000000f,  1.000000f },
    {  0.000000f,  0.000000f },
  };

  constexpr vec3 normals[] = {
    { -1.000000f, -0.000000f, -0.000000f },
    { -0.000000f, -0.000000f, -1.000000f },
    {  1.000000f, -0.000000f, -0.000000f },
    { -0.000000f, -0.000000f,  1.000000f },
    { -0.000000f, -1.000000f, -0.000000f },
    { -0.000000f,  1.000000f, -0.000000f },
  };

  constexpr face_vtn faces[] = {
    {{1, 0, 0}, {2, 0, 0}, {0, 1, 0}},
    {{3, 1, 1}, {6, 2, 1}, {2, 3, 1}},
    {{7, 0, 2}, {4, 3, 2}, {6, 1, 2}},
    {{5, 2, 3}, {0, 1, 3}, {4, 3, 3}},
    {{6, 2, 4}, {0, 1, 4}, {2, 0, 4}},
    {{3, 1, 5}, {5, 2, 5}, {7, 3, 5}},
    {{1, 0, 0}, {3, 1, 0}, {2, 0, 0}},
    {{3, 1, 1}, {7, 0, 1}, {6, 2, 1}},
    {{7, 0, 2}, {5, 2, 2}, {4, 3, 2}},
    {{5, 2, 3}, {1, 0, 3}, {0, 1, 3}},
    {{6, 2, 4}, {4, 3, 4}, {0, 1, 4}},
    {{3, 1, 5}, {1, 0, 5}, {5, 2, 5}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vtn));

}
