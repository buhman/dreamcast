#pragma once

#include "geometry/geometry.hpp"

namespace cursor {
  constexpr vec3 vertices[] = {
    {  0.407995f, -0.805648f,  2.000000f },
    {  0.000000f,  0.000000f,  2.000000f },
    {  0.550770f, -0.527772f,  2.000000f },
    {  0.302290f, -0.556623f,  2.000000f },
    {  0.297535f, -0.852536f,  2.000000f },
    {  0.000000f, -0.762211f,  2.000000f },
    {  0.191836f, -0.603526f,  2.000000f },
  };

  constexpr face_v faces[] = {
    {{1}, {6}, {0}},
    {{3}, {2}, {1}},
    {{1}, {5}, {6}},
    {{6}, {4}, {0}},
    {{0}, {3}, {1}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_v));

}
