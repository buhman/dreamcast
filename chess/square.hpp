#pragma once

#include "geometry/geometry.hpp"

namespace square {
  constexpr vec3 vertices[] = {
    { -1.000000f,  1.000000f, -0.000000f },
    {  1.000000f,  1.000000f, -0.000000f },
    { -1.000000f, -1.000000f,  0.000000f },
    {  1.000000f, -1.000000f,  0.000000f },
  };

  constexpr face_v faces[] = {
    {{1}, {2}, {0}},
    {{1}, {3}, {2}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_v));

}
