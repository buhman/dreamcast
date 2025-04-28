#pragma once

#include "math/bezier.hpp"

namespace q3bsp_patch {
  constexpr int max_patch_count = 16; // 12
  constexpr int max_surface_count = 32; // 20
  constexpr int max_level = 3;
  constexpr int level = 3;

  constexpr int max_vertices_per_surface = (max_level + 1) * (max_level + 1);
  constexpr int max_triangles_per_surface = max_level * max_level * 2;

  constexpr int vertices_per_surface = (level + 1) * (level + 1);
  constexpr int triangles_per_surface = level * level * 2;

  void triangulate_patches(const void * bsp);
  extern int patch_count;

  using vertex_plm = bezier::vec_lmn<float, 3, 2, 2>;

  struct patch {
    int face_ix;
    int vertex_ix;
    int triangle_ix;
  };

  extern vertex_plm patch_vertices[];
  extern bezier::triangle patch_triangles[];
  extern patch patches[];
}
