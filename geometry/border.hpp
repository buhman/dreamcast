#pragma once

#include "geometry.hpp"

namespace border {
  constexpr vec3 vertices[] = {
    { -1.000000f,  0.000000f,  1.000000f },
    {  1.000000f,  0.000000f,  1.000000f },
    { -1.000000f,  0.000000f, -1.000000f },
    {  1.000000f,  0.000000f, -1.000000f },
    {  0.950000f,  0.000000f,  1.000000f },
    {  0.950000f,  0.000000f, -1.000000f },
    {  0.950000f,  0.000000f,  0.950000f },
    {  0.950000f,  0.000000f, -0.950000f },
    { -0.950000f,  0.000000f, -1.000000f },
    { -0.950000f,  0.000000f,  1.000000f },
    { -0.950000f,  0.000000f,  0.950000f },
    { -0.950000f,  0.000000f, -0.950000f },
  };

  constexpr vec3 normals[] = {
    { -0.000000f,  1.000000f, -0.000000f },
  };

  constexpr face_vn faces[] = {
    {{ 5,  0}, {11,  0}, { 7,  0}},
    {{ 4,  0}, {10,  0}, { 9,  0}},
    {{ 5,  0}, { 8,  0}, {11,  0}},
    {{ 4,  0}, { 6,  0}, {10,  0}},
    {{ 7,  0}, { 3,  0}, { 5,  0}},
    {{ 0,  0}, {10,  0}, {11,  0}},
    {{11,  0}, { 2,  0}, { 0,  0}},
    {{ 1,  0}, { 6,  0}, { 4,  0}},
    {{ 7,  0}, { 6,  0}, { 3,  0}},
    {{ 0,  0}, { 9,  0}, {10,  0}},
    {{11,  0}, { 8,  0}, { 2,  0}},
    {{ 1,  0}, { 3,  0}, { 6,  0}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vn));

}
