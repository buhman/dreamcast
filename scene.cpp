#include <cstdint>

#include "holly/ta_parameter.h"

/*
            0,-.5
              |
             ---
  -0.5,0.5    |  0.5,0.5
 */

float scene_triangle[3][3] = {
  { 0.f,  -0.5f,  1/10.f},
  { 0.5f,  0.5f,  1/10.f},
  { -0.5f,  0.5f,  1/10.f},
};

static float theta = 0;
constexpr float one_degree = 0.01745329f;

uint32_t scene_transform(volatile uint32_t * scene)
{
  uint32_t ix = 0;

  triangle(&scene[(32 * ix) / 4]);
  ix++;

  for (int i = 0; i < 3; i++) {
    bool end_of_strip = i == 2;

    float x = scene_triangle[i][0];
    float y = scene_triangle[i][1];

    x = x * __builtin_cosf(theta) - y * __builtin_sinf(theta);
    y = x * __builtin_sinf(theta) + y * __builtin_cosf(theta);
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;

    vertex(&scene[(32 * ix) / 4],
           x, // x
           y, // y
           scene_triangle[i][2], // z
           0xffff00ff,           // base_color
           end_of_strip);
    ix++;
  }

  end_of_list(&scene[(32 * ix) / 4]);
  ix++;

  theta += one_degree;

  return ix;
}
