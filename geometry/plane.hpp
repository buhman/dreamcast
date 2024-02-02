#pragma once

#include "geometry.hpp"

namespace plane {
  constexpr vec3 vertices[] = {
    { -1.000000f,  0.000000f,  1.000000f },
    {  1.000000f,  0.000000f,  1.000000f },
    { -1.000000f,  0.000000f, -1.000000f },
    {  1.000000f,  0.000000f, -1.000000f },
  };

  constexpr vec2 texture[] = {
    {  1.000000f,  0.000000f },
    {  0.000000f,  1.000000f },
    {  0.000000f,  0.000000f },
    {  1.000000f,  1.000000f },
  };

  constexpr vec3 normals[] = {
    { -0.000000f,  1.000000f, -0.000000f },
  };

  constexpr face_vtn faces[] = {
    {{1, 0, 0}, {2, 1, 0}, {0, 2, 0}},
    {{1, 0, 0}, {3, 3, 0}, {2, 1, 0}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vtn));

}
