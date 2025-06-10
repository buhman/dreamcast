#pragma once

struct polygon {
  int a, b, c, d;
  int material_index;
  int uv_index;
};

struct mesh_material {
  int width;
  int height;
  int offset;
};

struct edge {
  int a; // vertices index
  int b; // vertices index
};

struct edge_polygon {
  struct edge edge;
  struct {
    int a;
    int b;
  } polygon_index; // polygon indices
};

struct mesh {
  const vec3 * position;
  const int position_length;
  const vec3 * normal;
  const int normal_length;
  const vec3 * polygon_normal;
  const int polygon_normal_length;
  const polygon * polygons;
  const int polygons_length;
  const vec2 ** uv_layers;
  const int uv_layers_length;
  const mesh_material * materials;
  const int materials_length;
  const edge_polygon * edge_polygons;
  const int edge_polygons_length;
};

struct object {
  const struct mesh * mesh;
  vec3 scale;
  vec4 rotation;
  vec3 location;
};

struct material {
  const void * start;
  const int size;
  const int offset;
};
