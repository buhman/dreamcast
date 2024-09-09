#include <stddef.h>

#include "model.h"

// floating-point
vertex_position cube_position[] = {
  {1.0, 1.0, -1.0},
  {1.0, -1.0, -1.0},
  {1.0, 1.0, 1.0},
  {1.0, -1.0, 1.0},
  {-1.0, 1.0, -1.0},
  {-1.0, -1.0, -1.0},
  {-1.0, 1.0, 1.0},
  {-1.0, -1.0, 1.0},
};

// floating-point
vertex_texture cube_texture[] = {
  {1.0, 0.0},
  {0.0, 1.0},
  {0.0, 0.0},
  {1.0, 1.0},
  {0.0, 0.0},
  {1.0, 0.0},
};

// floating-point
vertex_normal cube_normal[] = {
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
  {-1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 0.0, -1.0},
};

union triangle cube_knight_triangle[] = {
  { .v = {
    {4, 0, 0},
    {2, 1, 0},
    {0, 2, 0},
  }},
  { .v = {
    {2, 3, 1},
    {7, 4, 1},
    {3, 5, 1},
  }},
  { .v = {
    {6, 3, 2},
    {5, 4, 2},
    {7, 5, 2},
  }},
  { .v = {
    {1, 0, 3},
    {7, 1, 3},
    {5, 2, 3},
  }},
  { .v = {
    {0, 3, 4},
    {3, 4, 4},
    {1, 5, 4},
  }},
  { .v = {
    {4, 3, 5},
    {1, 4, 5},
    {5, 5, 5},
  }},
  { .v = {
    {4, 0, 0},
    {6, 3, 0},
    {2, 1, 0},
  }},
  { .v = {
    {2, 3, 1},
    {6, 1, 1},
    {7, 4, 1},
  }},
  { .v = {
    {6, 3, 2},
    {4, 1, 2},
    {5, 4, 2},
  }},
  { .v = {
    {1, 0, 3},
    {3, 3, 3},
    {7, 1, 3},
  }},
  { .v = {
    {0, 3, 4},
    {2, 1, 4},
    {3, 4, 4},
  }},
  { .v = {
    {4, 3, 5},
    {0, 1, 5},
    {1, 4, 5},
  }},
};

struct object cube_knight = {
  .triangle = &cube_knight_triangle[0],
  .quadrilateral = NULL,
  .triangle_count = 12,
  .quadrilateral_count = 0,
  .material = wn,
};

struct object * cube_object_list[] = {
  &cube_knight,
};

struct model cube_model = {
  .position = &cube_position[0],
  .texture = &cube_texture[0],
  .normal = &cube_normal[0],
  .object = &cube_object_list[0],
  .object_count = 1,
};

