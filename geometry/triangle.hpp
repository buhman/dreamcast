#pragma once

#include "geometry.hpp"

namespace triangle {
  constexpr vec3 vertices[] = {
    { -1.000000f,  0.000000f,  0.000000f },
    {  1.000000f,  0.000000f,  1.000000f },
    {  1.000000f,  0.000000f, -1.000000f },
  };

  constexpr vec2 texture[] = {
    {  0.000000f,  0.000000f },
    {  1.000000f,  0.000000f },
    {  1.000000f,  1.000000f },
  };

  constexpr vec3 normals[] = {
    { -0.000000f,  1.000000f, -0.000000f },
  };

  constexpr face_vtn faces[] = {
    {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vtn));

}
