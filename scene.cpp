#include <cstdint>
#include <cstddef>

#include "holly/ta_parameter.h"

#include "holly/texture_memory_alloc.h"

/*
  -0.5,-0.5      0.5,-0.5
              |
             ---
  -0.5,0.5    |  0.5,0.5
 */

struct triangle {
  float x;
  float y;
  float z;
  float u;
  float v;
  uint32_t color;
};

const struct triangle scene_triangle[4] = {
  { -0.5f,   0.5f,  0.f, 0.f        , 128.f/128.f, 0x00000000}, // the first two base colors in a
  { -0.5f,  -0.5f,  0.f, 0.f        , 0.f        , 0x00000000}, // triangle strip are ignored
  {  0.5f,   0.5f,  0.f, 128.f/128.f, 128.f/128.f, 0xffff00ff},
  {  0.5f,  -0.5f,  0.f, 128.f/128.f, 0.f        , 0xffffff00},
};

static float theta = 0;
constexpr float one_degree = 0.01745329f / 2.f;

uint32_t scene_transform(volatile uint32_t * scene)
{
  uint32_t ix = 0;

  uint32_t address = (offsetof (struct texture_memory_alloc, texture));
  textured_triangle(&scene[(32 * ix) / 4],
		    address);
  ix++;

  for (int i = 0; i < 4; i++) {
    bool end_of_strip = i == 3;

    float x = scene_triangle[i].x;
    float y = scene_triangle[i].y;
    float z = scene_triangle[i].z;
    float x1;

    x1 = x * __builtin_cosf(theta) - z * __builtin_sinf(theta);
    z  = x * __builtin_sinf(theta) + z * __builtin_cosf(theta);
    x  = x1;
    x *= 240.f;
    y *= 240.f;
    x += 320.f;
    y += 240.f;

    textured_vertex(&scene[(32 * ix) / 4],
		    x, // x
		    y, // y
		    1.f / (z + 10.f), // z
		    scene_triangle[i].u,     // u
		    scene_triangle[i].v,     // v
		    scene_triangle[i].color, // base_color
		    0, // offset_color
		    end_of_strip);
    ix++;
  }

  end_of_list(&scene[(32 * ix) / 4]);
  ix++;

  theta += one_degree;

  return ix;
}
