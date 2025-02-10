#pragma once

#include <stddef.h>

#include "model.h"

// floating-point
const vertex_position cube_position[] = {
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
const vertex_texture cube_texture[] = {
  {0.625, 0.5},
  {0.875, 0.5},
  {0.875, 0.75},
  {0.625, 0.75},
  {0.375, 0.75},
  {0.625, 1.0},
  {0.375, 1.0},
  {0.375, 0.0},
  {0.625, 0.0},
  {0.625, 0.25},
  {0.375, 0.25},
  {0.125, 0.5},
  {0.375, 0.5},
  {0.125, 0.75},
};

// floating-point
const vertex_normal cube_normal[] = {
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
  {-1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {1.0, 0.0, 0.0},
  {0.0, 0.0, -1.0},
};

const union quadrilateral cube_Cube_quadrilateral[] = {
  { .v = {
    {0, 0, 0},
    {4, 1, 0},
    {6, 2, 0},
    {2, 3, 0},
  }},
  { .v = {
    {3, 4, 1},
    {2, 3, 1},
    {6, 5, 1},
    {7, 6, 1},
  }},
  { .v = {
    {7, 7, 2},
    {6, 8, 2},
    {4, 9, 2},
    {5, 10, 2},
  }},
  { .v = {
    {5, 11, 3},
    {1, 12, 3},
    {3, 4, 3},
    {7, 13, 3},
  }},
  { .v = {
    {1, 12, 4},
    {0, 0, 4},
    {2, 3, 4},
    {3, 4, 4},
  }},
  { .v = {
    {5, 10, 5},
    {4, 9, 5},
    {0, 0, 5},
    {1, 12, 5},
  }},
};

const struct object cube_Cube = {
  .triangle = NULL,
  .quadrilateral = &cube_Cube_quadrilateral[0],
  .triangle_count = 0,
  .quadrilateral_count = 6,
  .material = -1,
};

const struct object * cube_object_list[] = {
  &cube_Cube,
};

const struct model cube_model = {
  .position = &cube_position[0],
  .texture = &cube_texture[0],
  .normal = &cube_normal[0],
  .object = &cube_object_list[0],
  .object_count = 1,
};
