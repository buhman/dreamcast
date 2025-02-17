#pragma once

#include <stddef.h>

#include "../model.h"

// floating-point
const vertex_position inverse_cube_position[] = {
  {1.0, 2.0, -1.0},
  {1.0, 0.0, -1.0},
  {1.0, 2.0, 1.0},
  {1.0, 0.0, 1.0},
  {-1.0, 2.0, -1.0},
  {-1.0, 0.0, -1.0},
  {-1.0, 2.0, 1.0},
  {-1.0, 0.0, 1.0},
};

// floating-point
const vertex_texture inverse_cube_texture[] = {
  {0.0, 0.0},
  {1.0, 0.0},
  {1.0, 1.0},
  {0.0, 1.0},
};

// floating-point
const vertex_normal inverse_cube_normal[] = {
  {0.0, -1.0, 0.0},
  {0.0, 0.0, -1.0},
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {-1.0, 0.0, 0.0},
  {0.0, 0.0, 1.0},
};

union quadrilateral inverse_cube_Cube_quadrilateral[] = {
  { .v = {
    {0, 0, 0},
    {2, 1, 0},
    {6, 2, 0},
    {4, 3, 0},
  }},
  { .v = {
    {3, 0, 1},
    {7, 1, 1},
    {6, 2, 1},
    {2, 3, 1},
  }},
  { .v = {
    {7, 0, 2},
    {5, 1, 2},
    {4, 2, 2},
    {6, 3, 2},
  }},
  { .v = {
    {5, 0, 3},
    {7, 1, 3},
    {3, 2, 3},
    {1, 3, 3},
  }},
  { .v = {
    {1, 0, 4},
    {3, 1, 4},
    {2, 2, 4},
    {0, 3, 4},
  }},
  { .v = {
    {5, 0, 5},
    {1, 1, 5},
    {0, 2, 5},
    {4, 3, 5},
  }},
};

const struct object inverse_cube_Cube = {
  .triangle = NULL,
  .quadrilateral = &inverse_cube_Cube_quadrilateral[0],
  .triangle_count = 0,
  .quadrilateral_count = 6,
  .material = -1,
};

const struct object * inverse_cube_object_list[] = {
  &inverse_cube_Cube,
};

const struct model inverse_cube_model = {
  .position = &inverse_cube_position[0],
  .texture = &inverse_cube_texture[0],
  .normal = &inverse_cube_normal[0],
  .object = &inverse_cube_object_list[0],
  .object_count = 1,
};

