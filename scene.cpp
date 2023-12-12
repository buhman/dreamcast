#include <cstdint>
#include <cstddef>

#include "holly/ta_parameter.hpp"

#include "holly/texture_memory_alloc.hpp"

/*
  -0.5,-0.5      0.5,-0.5
              |
             ---
  -0.5,0.5    |  0.5,0.5
 */

struct vertex1 {
  float x;
  float y;
  float z;
};

const struct vertex1 scene_quad[4] = {
  { -0.5f,   0.5f, 0.f },
  { -0.5f,  -0.5f, 0.f },
  {  0.5f,  -0.5f, 0.f },
  {  0.5f,   0.5f, 0.f },
};

struct scene_quad_ta_parameters {
  global_sprite sprite;
  vertex_sprite_type_0 vertex;
  global_end_of_list end_of_list;
};

static_assert((sizeof (scene_quad_ta_parameters)) == 32 * 4);

uint32_t scene_transform_quad(uint32_t * _scene, uint32_t base_color)
{
  auto scene = reinterpret_cast<scene_quad_ta_parameters *>(&_scene[0]);

  //uint32_t base_color = 0xffffff00;
  scene->sprite = global_sprite(base_color);
  scene->vertex = vertex_sprite_type_0(scene_quad[0].x * 240 + 320,
				       scene_quad[0].y * 240 + 240,
				       1 / (scene_quad[0].z + 10),
				       scene_quad[1].x * 240 + 320,
				       scene_quad[1].y * 240 + 240,
				       1 / (scene_quad[1].z + 10),
				       scene_quad[2].x * 240 + 320,
				       scene_quad[2].y * 240 + 240,
				       1 / (scene_quad[2].z + 10),
				       scene_quad[3].x * 240 + 320,
				       scene_quad[3].y * 240 + 240);
  scene->end_of_list = global_end_of_list();

  return (sizeof (scene_quad_ta_parameters));
}

static float theta = 0;
constexpr float half_degree = 0.01745329f / 2.f;

uint32_t scene_transform(uint32_t * _scene)
{
  ta_parameter * scene = reinterpret_cast<ta_parameter *>(&_scene[0]);
  int ix = 0;
  uint32_t texture_address = (offsetof (struct texture_memory_alloc, texture));
  scene[ix++].global_polygon_type_0 = global_polygon_type_0(texture_address);

  for (uint32_t i = 0; i < 4; i++) {
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

    scene[ix++].vertex_polygon_type_3 = vertex_polygon_type_3(x, // x
							      y, // y
							      1.f / (z + 10.f), // z
							      scene_triangle[i].u,     // u
							      scene_triangle[i].v,     // v
							      scene_triangle[i].color, // base_color
							      end_of_strip);
  }

  scene[ix++].global_end_of_list = global_end_of_list();

  theta += half_degree;

  return ix * 32;
}
