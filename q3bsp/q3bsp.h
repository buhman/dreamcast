#pragma once

#include <stdint.h>

typedef struct q3bsp_direntry {
  int offset;
  int length;
} q3bsp_direntry_t;

typedef struct q3bsp_header {
  char magic[4];
  int version;
  struct q3bsp_direntry direntries[17];
} q3bsp_header_t;

enum q3bsp_lumps {
  LUMP_ENTITES = 0,
  LUMP_TEXTURES = 1,
  LUMP_PLANES = 2,
  LUMP_NODES = 3,
  LUMP_LEAFS = 4,
  LUMP_LEAFFACES = 5,
  LUMP_LEAFBRUSHES = 6,
  LUMP_MODELS = 7,
  LUMP_BRUSHES = 8,
  LUMP_BRUSHSIDES = 9,
  LUMP_VERTEXES = 10,
  LUMP_MESHVERTS = 11,
  LUMP_EFFECTS = 12,
  LUMP_FACES = 13,
  LUMP_LIGHTMAPS = 14,
  LUMP_LIGHTVOLS = 15,
  LUMP_VISDATA = 16,
};

/*
typedef struct q3bsp_entity {
  char s[];
} q3bsp_entity_t;
*/

typedef struct q3bsp_texture {
  char name[64];
  int flags;
  int contents;
} q3bsp_texture_t;

typedef struct q3bsp_plane {
  float normal[3];
  float dist;
} q3bsp_plane_t;

typedef struct q3bsp_node {
  int plane;
  int children[2];
  int mins[3];
  int maxs[3];
} q3bsp_node_t;

typedef struct q3bsp_leaf {
  int cluster;
  int area;
  int mins[3];
  int maxs[3];
  int leafface;
  int n_leaffaces;
  int leafbrush;
  int n_leafbrushes;
} q3bsp_leaf_t;

typedef struct q3bsp_leafface {
  int face;
} q3bsp_leafface_t;

typedef struct q3bsp_leafbrush {
  int brush;
} q3bsp_leafbrush_t;

typedef struct q3bsp_model {
  float mins[3];
  float maxs[3];
  int face;
  int n_faces;
  int brush;
  int n_brushes;
} q3bsp_model_t;

typedef struct q3bsp_brush {
  int brushside;
  int n_brushsides;
  int texture;
} q3bsp_brush_t;

typedef struct q3bsp_brushside {
  int plane;
  int texture;
} q3bsp_brushside_t;

typedef struct q3bsp_vertex {
  float position[3];
  float texture[2];
  float lightmap[2];
  float normal[3];
  uint8_t color[4];
} q3bsp_vertex_t;

typedef struct q3bsp_meshvert {
  int offset;
} q3bsp_meshvert_t;

typedef struct q3bsp_effect {
  char name[64];
  int brush;
  int unknown;
} q3bsp_effect_t;

enum q3bsp_face_type {
  FACE_TYPE_POLYGON = 1,
  FACE_TYPE_PATCH = 2,
  FACE_TYPE_MESH = 3,
  FACE_TYPE_BILLBOARD = 4,
};

typedef struct q3bsp_face {
  int texture;
  int effect;
  int type; // enum q3bsp_face_type
  int vertex;
  int n_vertexes;
  int meshvert;
  int n_meshverts;
  int lm_index;
  int lm_start[2];
  int lm_size[2];
  float lm_origin[3];
  float lm_vecs[2][3];
  float normal[3];
  int size[2];
} q3bsp_face_t;

typedef struct q3bsp_lightmap {
  union {
    uint8_t u8[128 * 128 * 3];
    uint8_t map[128][128][3];
  };
} q3bsp_lightmap_t;

typedef struct q3bsp_visdata {
  int n_vecs;
  int sz_vecs;
  uint8_t vecs[];
} q3bsp_visdata_t;
