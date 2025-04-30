#ifdef __cplusplus
extern "C" {
#endif

#define MDX_IDENT (('M'<<24)+('G'<<16)+('L'<<8)+'2')

#define MAX_QPATH 64

// mdxm = mod->mdxm = (mdxmHeader_t*)

typedef struct mdxm_header {
  union {
    int ident;
    uint8_t magic[4];
  };
  int version;
  char name[MAX_QPATH];
  char anim_name[MAX_QPATH];
  int anim_index;
  int num_bones;
  int num_lods;
  int offset_lods;
  int num_surfaces;
  int offset_surface_hierarchy;
  int offset_end;
} mdxm_header_t;

// lod = (mdxmLOD_t *) ( (byte *)mdxm + mdxm->ofsLODs );
typedef struct mdxm_lod {
  int offset_end;
} mdxm_lod_t;

typedef struct mdxm_lod_surf_offset {
  int offsets[1];
} mdxm_lod_surf_offset_t;


typedef struct {
  char name[MAX_QPATH];
  uint32_t flags;
  char shader[MAX_QPATH];
  int shader_index;
  int parent_index;
  int num_children;
  int child_indexes[1];
} mdxm_surf_hierarchy_t;

// surf = (mdxmSurface_t *) ( (byte *)lod + sizeof (mdxmLOD_t) + (mdxm->numSurfaces * sizeof(mdxmLODSurfOffset_t)) );

typedef struct mdxm_surface {
  int ident;
  int this_surface_index;
  int offset_header;
  int num_verts;
  int offset_verts;
  int num_triangles;
  int offset_triangles;
  int num_bone_references;
  int offset_bone_references;
  int offset_end;
} mdxm_surface_t;

/*
  v = (mdxmVertex_t *) ((byte *)surface + surface->offset_verts);
  pTexCoords = (mdxmVertexTexCoord_t *) &v[numVerts];
 */

typedef struct mdxm_triangle {
  int index[3];
} mdxm_triangle_t;

typedef float mdxm_vec2_t[2];
typedef float mdxm_vec3_t[3];

typedef struct mdxm_vertex {
  mdxm_vec3_t normal;
  mdxm_vec3_t position;
  uint32_t flags;
  uint8_t bone_weightings[4];
} mdxm_vertex_t;

typedef struct mdxm_vertex_texture_coord {
  mdxm_vec2_t texture;
} mdxm_vertex_texture_coord_t;

#ifdef __cplusplus
}
#endif
