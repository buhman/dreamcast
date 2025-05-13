#pragma once

struct md5_mesh_joint {
  const char * bone_name;
  const int parent_index;
  const vec3 pos;
  const vec4 orient;
};

struct md5_mesh_vert {
  const int vert_index;
  const vec2 tex;
  const int weight_index;
  const int weight_elem;
};

struct md5_mesh_tri {
  const int tri_index;
  struct {
    const int a;
    const int b;
    const int c;
  } vert_index;
};

struct md5_mesh_weight {
  const int weight_index;
  const int joint_index;
  const float weight_value;
  const vec3 pos;
};

struct md5_mesh_mesh {
  const char * shader;
  const int num_verts;
  const md5_mesh_vert * verts;
  const int num_tris;
  const md5_mesh_tri * tris;
  const int num_weights;
  const md5_mesh_weight * weights;
};

struct md5_mesh {
  const int num_joints;
  const int num_meshes;
  const md5_mesh_joint * joints;
  const md5_mesh_mesh * meshes;
};
