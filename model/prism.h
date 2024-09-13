#include <stddef.h>

#include "model.h"

// floating-point
vertex_position prism_position[] = {
  {1.0, 0.5, -0.5},
  {1.0, -0.5, -0.5},
  {1.0, 0.5, 0.5},
  {1.0, -0.5, 0.5},
  {-1.0, 0.5, -0.5},
  {-1.0, -0.5, -0.5},
  {-1.0, 0.5, 0.5},
  {-1.0, -0.5, 0.5},
};

// floating-point
vertex_texture prism_texture[] = {
  {0.0, 1.0},
  {1.0, 0.0},
  {1.0, 1.0},
  {0.0, 0.0},
  {1.0, 1.0},
  {0.0, 1.0},
  {0.75, 0.0},
  {0.25, 1.0},
  {0.25, 0.0},
  {0.0, 0.0},
  {0.75, 1.0},
};

// floating-point
vertex_normal prism_normal[] = {
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
  {-1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 0.0, -1.0},
};

union triangle prism_Cube_triangle[] = {
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
    {6, 6, 2},
    {5, 7, 2},
    {7, 8, 2},
  }},
  { .v = {
    {1, 0, 3},
    {7, 1, 3},
    {5, 2, 3},
  }},
  { .v = {
    {0, 6, 4},
    {3, 7, 4},
    {1, 8, 4},
  }},
  { .v = {
    {4, 3, 5},
    {1, 4, 5},
    {5, 5, 5},
  }},
  { .v = {
    {4, 0, 0},
    {6, 9, 0},
    {2, 1, 0},
  }},
  { .v = {
    {2, 3, 1},
    {6, 1, 1},
    {7, 4, 1},
  }},
  { .v = {
    {6, 6, 2},
    {4, 10, 2},
    {5, 7, 2},
  }},
  { .v = {
    {1, 0, 3},
    {3, 9, 3},
    {7, 1, 3},
  }},
  { .v = {
    {0, 6, 4},
    {2, 10, 4},
    {3, 7, 4},
  }},
  { .v = {
    {4, 3, 5},
    {0, 1, 5},
    {1, 4, 5},
  }},
};

struct object prism_Cube = {
  .triangle = &prism_Cube_triangle[0],
  .quadrilateral = NULL,
  .triangle_count = 12,
  .quadrilateral_count = 0,
  .material = Material,
};

struct object * prism_object_list[] = {
  &prism_Cube,
};

struct model prism_model = {
  .position = &prism_position[0],
  .texture = &prism_texture[0],
  .normal = &prism_normal[0],
  .object = &prism_object_list[0],
  .object_count = 1,
};

