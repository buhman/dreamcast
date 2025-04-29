#include "math/vec2.hpp"
#include "math/vec3.hpp"

#include "q3bsp/q3bsp.h"
#include "q3bsp/q3bsp_patch.hpp"

#include "assert.h"

namespace q3bsp_patch {

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;

vertex_plm patch_vertices[max_surface_count * max_vertices_per_surface];
constexpr int max_patch_vertices = (sizeof (patch_vertices)) / (sizeof (vertex_plm));

bezier::triangle patch_triangles[max_surface_count * max_triangles_per_surface];
constexpr int max_patch_triangles = (sizeof (patch_triangles)) / ((sizeof (int)) * 3);

patch patches[max_patch_count];
int patch_count = 0;

static int triangulate_patch(const q3bsp_vertex_t * vertexes,
                             const int width,
                             const int height,
                             vertex_plm * p_vtx,
                             bezier::triangle * p_tri)
{
  const int h_surfaces = (width - 1) / 2;
  const int v_surfaces = (height - 1) / 2;

  int vertex_base = 0;

  for (int j = 0; j < v_surfaces; j++) {
    for (int k = 0; k < h_surfaces; k++) {
      int ix = j * width * 2 + k * 2;
      const q3bsp_vertex_t * a = &vertexes[ix + width * 0 + 0];
      const q3bsp_vertex_t * b = &vertexes[ix + width * 0 + 1];
      const q3bsp_vertex_t * c = &vertexes[ix + width * 0 + 2];
      const q3bsp_vertex_t * d = &vertexes[ix + width * 1 + 0];
      const q3bsp_vertex_t * e = &vertexes[ix + width * 1 + 1];
      const q3bsp_vertex_t * f = &vertexes[ix + width * 1 + 2];
      const q3bsp_vertex_t * g = &vertexes[ix + width * 2 + 0];
      const q3bsp_vertex_t * h = &vertexes[ix + width * 2 + 1];
      const q3bsp_vertex_t * i = &vertexes[ix + width * 2 + 2];

      const vec3 a_p = {a->position[0], a->position[1], a->position[2]};
      const vec3 b_p = {b->position[0], b->position[1], b->position[2]};
      const vec3 c_p = {c->position[0], c->position[1], c->position[2]};
      const vec3 d_p = {d->position[0], d->position[1], d->position[2]};
      const vec3 e_p = {e->position[0], e->position[1], e->position[2]};
      const vec3 f_p = {f->position[0], f->position[1], f->position[2]};
      const vec3 g_p = {g->position[0], g->position[1], g->position[2]};
      const vec3 h_p = {h->position[0], h->position[1], h->position[2]};
      const vec3 i_p = {i->position[0], i->position[1], i->position[2]};

      const vec2 a_t = {a->texture[0], a->texture[1]};
      const vec2 b_t = {b->texture[0], b->texture[1]};
      const vec2 c_t = {c->texture[0], c->texture[1]};
      const vec2 d_t = {d->texture[0], d->texture[1]};
      const vec2 e_t = {e->texture[0], e->texture[1]};
      const vec2 f_t = {f->texture[0], f->texture[1]};
      const vec2 g_t = {g->texture[0], g->texture[1]};
      const vec2 h_t = {h->texture[0], h->texture[1]};
      const vec2 i_t = {i->texture[0], i->texture[1]};

      const vec2 a_l = {a->lightmap[0], a->lightmap[1]};
      const vec2 b_l = {b->lightmap[0], b->lightmap[1]};
      const vec2 c_l = {c->lightmap[0], c->lightmap[1]};
      const vec2 d_l = {d->lightmap[0], d->lightmap[1]};
      const vec2 e_l = {e->lightmap[0], e->lightmap[1]};
      const vec2 f_l = {f->lightmap[0], f->lightmap[1]};
      const vec2 g_l = {g->lightmap[0], g->lightmap[1]};
      const vec2 h_l = {h->lightmap[0], h->lightmap[1]};
      const vec2 i_l = {i->lightmap[0], i->lightmap[1]};

      const vec3 a_n = {a->normal[0], a->normal[1], a->normal[2]};
      const vec3 b_n = {b->normal[0], b->normal[1], b->normal[2]};
      const vec3 c_n = {c->normal[0], c->normal[1], c->normal[2]};
      const vec3 d_n = {d->normal[0], d->normal[1], d->normal[2]};
      const vec3 e_n = {e->normal[0], e->normal[1], e->normal[2]};
      const vec3 f_n = {f->normal[0], f->normal[1], f->normal[2]};
      const vec3 g_n = {g->normal[0], g->normal[1], g->normal[2]};
      const vec3 h_n = {h->normal[0], h->normal[1], h->normal[2]};
      const vec3 i_n = {i->normal[0], i->normal[1], i->normal[2]};

      const vertex_plm control[9] = {
        {a_p, a_t, a_l, a_n}, {b_p, b_t, b_l, b_n}, {c_p, c_t, c_l, c_n},
        {d_p, d_t, d_l, d_n}, {e_p, e_t, e_l, e_n}, {f_p, f_t, f_l, f_n},
        {g_p, g_t, g_l, g_n}, {h_p, h_t, h_l, h_n}, {i_p, i_t, i_l, i_n},
      };

      bezier::tessellate<float, 3, 2, 2, 3>(level,
                                            control,
                                            p_vtx,
                                            p_tri,
                                            vertex_base);
      p_vtx += vertices_per_surface;
      p_tri += triangles_per_surface;
      vertex_base += vertices_per_surface;
    }
  }

  return h_surfaces * v_surfaces;
}

void triangulate_patches(const void * bsp)
{
  const uint8_t * buf = reinterpret_cast<const uint8_t *>(bsp);
  const q3bsp_header_t * header = reinterpret_cast<const q3bsp_header_t *>(buf);

  const q3bsp_direntry * fe = &header->direntries[LUMP_FACES];
  const q3bsp_face_t * faces = reinterpret_cast<const q3bsp_face_t *>(&buf[fe->offset]);

  const q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  const q3bsp_vertex_t * vertexes = reinterpret_cast<const q3bsp_vertex_t *>(&buf[ve->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  int vertex_ix = 0;
  int triangle_ix = 0;

  for (int i = 0; i < face_count; i++) {
    const q3bsp_face_t * face = &faces[i];
    if (face->type == FACE_TYPE_PATCH) {
      assert(vertex_ix < max_patch_vertices);
      assert(triangle_ix < max_patch_triangles);

      int surface_count = triangulate_patch(&vertexes[face->vertex],
                                            face->size[0],
                                            face->size[1],
                                            &patch_vertices[vertex_ix],
                                            &patch_triangles[triangle_ix]);
      assert(patch_count < max_patch_count);
      patches[patch_count].face_ix = i;
      patches[patch_count].vertex_ix = vertex_ix;
      patches[patch_count].triangle_ix = triangle_ix;
      patch_count += 1;

      vertex_ix += vertices_per_surface * surface_count;
      triangle_ix += triangles_per_surface * surface_count;
    }
  }
  assert(vertex_ix <= max_patch_vertices);
  assert(triangle_ix <= max_patch_triangles);
}

}
