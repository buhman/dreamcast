#pragma once

#include "geometry/geometry.hpp"

namespace circle {
  constexpr vec3 vertices[] = {
    {  0.000000f,  0.000000f,  0.000000f },
    {  0.000000f, -1.000000f,  0.000000f },
    { -0.361242f, -0.932472f,  0.000000f },
    { -0.673696f, -0.739009f,  0.000000f },
    { -0.895163f, -0.445738f,  0.000000f },
    { -0.995734f, -0.092268f,  0.000000f },
    { -0.961826f,  0.273663f, -0.000000f },
    { -0.798017f,  0.602635f, -0.000000f },
    { -0.526432f,  0.850217f, -0.000000f },
    { -0.183750f,  0.982973f, -0.000000f },
    {  0.183750f,  0.982973f, -0.000000f },
    {  0.526432f,  0.850217f, -0.000000f },
    {  0.798017f,  0.602635f, -0.000000f },
    {  0.961826f,  0.273663f, -0.000000f },
    {  0.995734f, -0.092268f,  0.000000f },
    {  0.895163f, -0.445738f,  0.000000f },
    {  0.673696f, -0.739009f,  0.000000f },
    {  0.361242f, -0.932472f,  0.000000f },
  };

  constexpr face_v faces[] = {
    {{ 0}, { 1}, { 2}},
    {{ 0}, { 2}, { 3}},
    {{ 0}, { 3}, { 4}},
    {{ 0}, { 4}, { 5}},
    {{ 0}, { 5}, { 6}},
    {{ 0}, { 6}, { 7}},
    {{ 0}, { 7}, { 8}},
    {{ 0}, { 8}, { 9}},
    {{ 0}, { 9}, {10}},
    {{ 0}, {10}, {11}},
    {{ 0}, {11}, {12}},
    {{ 0}, {12}, {13}},
    {{ 0}, {13}, {14}},
    {{ 0}, {14}, {15}},
    {{ 0}, {15}, {16}},
    {{ 0}, {16}, {17}},
    {{ 0}, {17}, { 1}},
  };

  constexpr uint32_t num_faces = (sizeof (faces)) / (sizeof (face_v));

}
