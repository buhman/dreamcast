#pragma once

#include "geometry.hpp"

namespace circle {
  constexpr vec3 vertices[] = {
    {  0.000000f,  0.000000f,  1.000000f },
    {  0.195090f,  0.000000f,  0.980785f },
    {  0.382683f,  0.000000f,  0.923880f },
    {  0.555570f,  0.000000f,  0.831470f },
    {  0.707107f,  0.000000f,  0.707107f },
    {  0.831470f,  0.000000f,  0.555570f },
    {  0.923880f,  0.000000f,  0.382683f },
    {  0.980785f,  0.000000f,  0.195090f },
    {  1.000000f,  0.000000f,  0.000000f },
    {  0.980785f,  0.000000f, -0.195090f },
    {  0.923880f,  0.000000f, -0.382683f },
    {  0.831470f,  0.000000f, -0.555570f },
    {  0.707107f,  0.000000f, -0.707107f },
    {  0.555570f,  0.000000f, -0.831470f },
    {  0.382683f,  0.000000f, -0.923880f },
    {  0.195090f,  0.000000f, -0.980785f },
    {  0.000000f,  0.000000f, -1.000000f },
    { -0.195090f,  0.000000f, -0.980785f },
    { -0.382683f,  0.000000f, -0.923880f },
    { -0.555570f,  0.000000f, -0.831470f },
    { -0.707107f,  0.000000f, -0.707107f },
    { -0.831470f,  0.000000f, -0.555570f },
    { -0.923880f,  0.000000f, -0.382683f },
    { -0.980785f,  0.000000f, -0.195090f },
    { -1.000000f,  0.000000f,  0.000000f },
    { -0.980785f,  0.000000f,  0.195090f },
    { -0.923880f,  0.000000f,  0.382683f },
    { -0.831470f,  0.000000f,  0.555570f },
    { -0.707107f,  0.000000f,  0.707107f },
    { -0.555570f,  0.000000f,  0.831470f },
    { -0.382683f,  0.000000f,  0.923880f },
    { -0.195090f,  0.000000f,  0.980785f },
  };

  constexpr vec2 texture[] = {
    {  0.450764f,  0.000100f },
    {  0.999900f,  0.450764f },
    {  0.549236f,  0.999900f },
    {  0.450764f,  0.999900f },
    {  0.181332f,  0.888297f },
    {  0.354184f,  0.980689f },
    {  0.263208f,  0.943006f },
    {  0.111702f,  0.818668f },
    {  0.056994f,  0.736791f },
    {  0.019311f,  0.645815f },
    {  0.000100f,  0.549236f },
    {  0.000100f,  0.450764f },
    {  0.111702f,  0.181332f },
    {  0.019311f,  0.354184f },
    {  0.056994f,  0.263208f },
    {  0.181332f,  0.111702f },
    {  0.263209f,  0.056994f },
    {  0.354185f,  0.019311f },
    {  0.549236f,  0.000100f },
    {  0.818668f,  0.111702f },
    {  0.645816f,  0.019311f },
    {  0.736792f,  0.056994f },
    {  0.888298f,  0.181332f },
    {  0.943006f,  0.263209f },
    {  0.980689f,  0.354185f },
    {  0.999900f,  0.549236f },
    {  0.888298f,  0.818668f },
    {  0.980689f,  0.645815f },
    {  0.943006f,  0.736791f },
    {  0.818668f,  0.888298f },
    {  0.736791f,  0.943006f },
    {  0.645815f,  0.980689f },
  };

  constexpr vec3 normals[] = {
    { -0.000000f,  1.000000f, -0.000000f },
  };

  constexpr face_vtn faces[] = {
    {{16,  0,  0}, {24,  1,  0}, { 0,  2,  0}},
    {{ 0,  2,  0}, { 1,  3,  0}, { 4,  4,  0}},
    {{ 1,  3,  0}, { 2,  5,  0}, { 4,  4,  0}},
    {{ 2,  5,  0}, { 3,  6,  0}, { 4,  4,  0}},
    {{ 4,  4,  0}, { 5,  7,  0}, { 6,  8,  0}},
    {{ 6,  8,  0}, { 7,  9,  0}, { 4,  4,  0}},
    {{ 7,  9,  0}, { 8, 10,  0}, { 4,  4,  0}},
    {{ 8, 10,  0}, { 9, 11,  0}, {12, 12,  0}},
    {{ 9, 11,  0}, {10, 13,  0}, {12, 12,  0}},
    {{10, 13,  0}, {11, 14,  0}, {12, 12,  0}},
    {{12, 12,  0}, {13, 15,  0}, {14, 16,  0}},
    {{14, 16,  0}, {15, 17,  0}, {12, 12,  0}},
    {{15, 17,  0}, {16,  0,  0}, {12, 12,  0}},
    {{16,  0,  0}, {17, 18,  0}, {20, 19,  0}},
    {{17, 18,  0}, {18, 20,  0}, {20, 19,  0}},
    {{18, 20,  0}, {19, 21,  0}, {20, 19,  0}},
    {{20, 19,  0}, {21, 22,  0}, {22, 23,  0}},
    {{22, 23,  0}, {23, 24,  0}, {20, 19,  0}},
    {{23, 24,  0}, {24,  1,  0}, {20, 19,  0}},
    {{24,  1,  0}, {25, 25,  0}, {28, 26,  0}},
    {{25, 25,  0}, {26, 27,  0}, {28, 26,  0}},
    {{26, 27,  0}, {27, 28,  0}, {28, 26,  0}},
    {{28, 26,  0}, {29, 29,  0}, {30, 30,  0}},
    {{30, 30,  0}, {31, 31,  0}, {28, 26,  0}},
    {{31, 31,  0}, { 0,  2,  0}, {28, 26,  0}},
    {{ 0,  2,  0}, { 4,  4,  0}, { 8, 10,  0}},
    {{ 8, 10,  0}, {12, 12,  0}, {16,  0,  0}},
    {{16,  0,  0}, {20, 19,  0}, {24,  1,  0}},
    {{24,  1,  0}, {28, 26,  0}, { 0,  2,  0}},
    {{ 0,  2,  0}, { 8, 10,  0}, {16,  0,  0}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_vtn));

}
