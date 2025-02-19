/* primitives */
struct mesh_face {
  //int n_face_vertex_indices;
  int face_vertex_indices[3];
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
  vec4 face_color;
  float power;
  vec3 specular_color;
  vec3 emissive_color;
  const data_object * objects[];
};

struct texture_filename {
  enum tag tag;
  //const char * filename;
  const void * start;
  int size;
  int texture_memory_offset;
  int16_t width;
  int16_t height;
};

struct frame {
  enum tag tag;
  const data_object * objects[];
};

struct frame_transform_matrix {
  enum tag tag;
  mat4x4 frame_matrix;
};

struct mesh_material_list;
struct mesh_normals;
struct mesh_texture_coords;

struct mesh {
  enum tag tag;
  int n_vertices;
  vec3 * vertices;
  int n_faces;
  mesh_face * faces;
  //
  const mesh_material_list * material_list;
  const mesh_normals * normals;
  const mesh_texture_coords * texture_coords;

  //const data_object * objects[];
};

struct mesh_material_list {
  enum tag tag;
  int n_materials;
  int n_face_indices;
  int * face_indices;
  const data_object * objects[];
};

struct mesh_normals {
  enum tag tag;
  int n_normals;
  vec3 * normals;
  int n_face_normals;
  mesh_face * face_normals;
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
  const data_object * objects[];
};

struct animation_set {
  enum tag tag;
  const data_object * objects[];
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
