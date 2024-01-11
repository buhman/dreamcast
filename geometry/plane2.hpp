#pragma once

#include "geometry.hpp"

namespace plane {
  constexpr position__color vertices[] = {
    { {-10.000000f,  0.000000f, -10.000000f}, { 0.011800f,  1.000000f,  1.000000f} },
    { {10.000000f,  0.000000f, -10.000000f}, { 1.000000f,  0.003900f,  0.066700f} },
    { {-10.000000f,  0.000000f, 10.000000f}, { 0.019600f,  1.000000f,  0.019600f} },
    { {10.000000f,  0.000000f, 10.000000f}, { 1.000000f,  0.952900f,  0.011800f} },
  };

  constexpr vec2 texture[] = {
    {  1.000000f,  0.000000f },
    {  0.000000f,  1.000000f },
    {  0.000000f,  0.000000f },
    {  1.000000f,  1.000000f },
  };

  constexpr vec3 normals[] = {
    { -0.000000f, -1.000000f, -0.000000f },
  };

  constexpr face_vtn faces[] = {
    {{1, 0, 0}, {2, 1, 0}, {0, 2, 0}},
    {{1, 0, 0}, {3, 3, 0}, {2, 1, 0}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vtn));

}
