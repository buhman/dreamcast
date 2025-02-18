/* primitives */
struct mesh_face {
  int nfacevertexindices;
  int facevertexindices[];
};

struct float_keys {
  int nvalues;
  float values;
};

struct timed_float_keys {
  int time;
  struct float_keys tfkeys;
};

/* data objects */
struct data_object;

enum struct tag : int {
  header,
  material,
  texture_filename,
  frame,
  frame_transform_matrix,
  mesh,
  mesh_material_list,
  mesh_normals,
  mesh_texture_coords,
  animation_key,
  animation_options,
  animation,
  animation_set,
};

struct header {
  enum tag tag;
  int major;
  int minor;
  int flags;
};

struct material {
  enum tag tag;
  vec4 facecolor;
  float power;
  vec3 specularcolor;
  vec3 emissivecolor;
  struct data_object * objects[];
};

struct texture_filename {
  enum tag tag;
  const char * filename;
};

struct frame {
  enum tag tag;
  struct data_object * objects[];
};

struct frame_transform_matrix {
  enum tag tag;
  mat4x4 framematrix;
};

struct mesh {
  enum tag tag;
  int nvertices;
  vec3 * vertices;
  int nfaces;
  struct mesh_face * faces;
  struct data_object * objects[];
};

struct mesh_material_list {
  enum tag tag;
  int n_materials;
  int n_face_indices;
  int * face_indices;
  struct data_object * objects[];
};

struct mesh_normals {
  enum tag tag;
  int n_normals;
  vec3 * normals;
  int n_face_normals;
  struct mesh_face * face_normals;
};

struct mesh_texture_coords {
  enum tag tag;
  int n_texture_coords;
  vec2 texture_coords[];
};

struct animation_key {
  enum tag tag;
  int key_type;
  int n_keys;
  struct timed_float_keys keys[];
};

struct animation_options {
  enum tag tag;
  int open_closed;
  int position_quality;
};

struct animation {
  enum tag tag;
  struct data_object * objects[];
};

struct animation_set {
  enum tag tag;
  struct data_object * objects[];
};

struct data_object {
  union {
    enum tag tag;
    struct header header;
    struct material material;
    struct texture_filename texture_filename;
    struct frame frame;
    struct frame_transform_matrix frame_transform_matrix;
    struct mesh mesh;
    struct mesh_material_list mesh_material_list;
    struct mesh_normals mesh_normals;
    struct mesh_texture_coords mesh_texture_coords;
    struct animation_set animation_set;
    struct animation animation;
    struct animation_key animation_key;
  };
};
