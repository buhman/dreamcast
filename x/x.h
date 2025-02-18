struct header {
  int tag;
  int major;
  int minor;
  int flags;
};

struct color_rgba {
  float r;
  float g;
  float b;
  float a;
};

struct color_rgb {
  float r;
  float g;
  float b;
};

struct material {
  int tag;
  x_color_rgba facecolor;
  float power;
  x_color_rgb specularcolor;
  x_color_rgb emissivecolor;
  void * objects[];
};

struct frame {
  int tag;
  void * objects[];
};

struct frame_transform_matrix {
  int tag;
  mat4x4 framematrix;
};

struct mesh_face {
  int nfacevertexindices;
  int facevertexindices[];
};

struct mesh {
  int nvertices;
  vec3 * vertices;
  int nfaces;
  mesh_face * faces;
  void * objects[];
};

struct mesh_material_list {
  int n_materials;
  int n_face_indices;
  int * face_indices;
  void * objects[];
};

struct mesh_normals {
  int tag;
  int n_normals;
  vec3 * normals;
  int n_face_normals;
  mesh_face * face_normals;
};

struct mesh_texture_coords {
  int tag;
  int n_texture_coords;
  vec2 texture_coords[];
};

struct texture_filename {
  const char * filename;
};

struct float_keys {
  int nvalues;
  float values;
};

struct timed_float_keys {
  int time;
  float_keys tfkeys;
};

struct animation_key {
  int tag;
  int key_type;
  int n_keys;
  timed_float_keys keys[];
};

struct animation_options {
  int tag;
  int open_closed;
  int position_quality;
};

struct animation {
  int tag;
  void * objects[];
};

struct animation_set {
  int tag;
  void * objects[];
}
