#include "shadow_volume.hpp"

#include "math/float_types.hpp"
#include "assert.h"
#include "printf/printf.h"

static inline void face_indicators(const vec3 light,
                                   const vec3 * position,
                                   const vec3 * polygon_normal,
                                   const mesh * mesh,
                                   float * indicators)
{
  for (int i = 0; i < mesh->polygons_length; i++) {
    vec3 n = polygon_normal[i];
    vec3 p = position[mesh->polygons[i].a];

    float indicator = dot(n, (light - p));
    indicators[i] = indicator;
  }
}

static inline int object_silhouette(const float * indicators,
                                    const mesh * mesh,
                                    int * edge_indices)
{
  int ix = 0;
  for (int i = 0; i < mesh->edge_polygons_length; i++) {
    const edge_polygon * ep = &mesh->edge_polygons[i];
    if ((indicators[ep->polygon_index.a] > 0) != (indicators[ep->polygon_index.b] > 0)) {
      edge_indices[ix] = i;
      ix += 1;
    }
  }

  return ix;
}

struct graph {
  int a;
  int b;
};

void graph_append(graph * g, int v)
{
  if (!(g->a == -1 || g->b == -1)) {
    return;
  }

  if (g->a == -1) {
    g->a = v;
  } else {
    g->b = v;
  }
}

void edge_loop_graph(const mesh * mesh,
                     const int * edge_indices,
                     const int edge_indices_length,
                     graph * graph)
{
  for (int i = 0; i < mesh->position_length; i++) {
    graph[i].a = -1;
    graph[i].b = -1;
  }

  for (int i = 0; i < edge_indices_length; i++) {
    int edge_index = edge_indices[i];
    const edge& edge = mesh->edge_polygons[edge_index].edge;
    graph_append(&graph[edge.a], i);
    graph_append(&graph[edge.b], i);
  }
}

int next_neighbor(const graph& graph, int ix)
{
  if (graph.a == ix)
    return graph.b;
  else
    return graph.a;
}

int edge_loop_inner(const mesh * mesh,
                    const int * edge_indices,
                    const graph * graph,
                    bool * visited_edge_indices,
                    int ix,
                    int * edge_loop)
{
  int edge_loop_ix = 0;

  const edge& e = mesh->edge_polygons[edge_indices[ix]].edge;
  edge_loop[edge_loop_ix] = e.b;
  edge_loop_ix += 1;

  while (true) {
    visited_edge_indices[ix] = true;

    int edge_index = edge_indices[ix];
    const edge& e = mesh->edge_polygons[edge_index].edge;
    int next_ix_a = next_neighbor(graph[e.a], ix);
    int next_ix_b = next_neighbor(graph[e.b], ix);
    if (visited_edge_indices[next_ix_a] == false) {
      edge_loop[edge_loop_ix] = e.a;
      edge_loop_ix += 1;

      ix = next_ix_a;
    } else if (visited_edge_indices[next_ix_b] == false) {
      edge_loop[edge_loop_ix] = e.b;
      edge_loop_ix += 1;

      ix = next_ix_b;
    } else {
      break;
    }
  }
  return edge_loop_ix;
}

int next_unvisited(const bool * visited_edge_indices,
                   const int edge_indices_length)
{
  for (int i = 0; i < edge_indices_length; i++) {
    if (visited_edge_indices[i] == false)
      return i;
  }
  return -1;
}

int edge_loop(const mesh * mesh,
              const int * edge_indices,
              const int edge_indices_length,
              const graph * graph,
              int * edge_loops,
              int * edge_loop_lengths,
              int max_edge_loops)
{
  bool visited_edge_indices[edge_indices_length];
  for (int i = 0; i < edge_indices_length; i++) {
    visited_edge_indices[i] = false;
  }

  int edge_loop_ix = 0;
  int i;
  for (i = 0; i < max_edge_loops; i++) {
    int start = next_unvisited(visited_edge_indices, edge_indices_length);
    if (start == -1)
      break;
    int length = edge_loop_inner(mesh, edge_indices, graph, visited_edge_indices, start,
                                 &edge_loops[edge_loop_ix]);
    edge_loop_lengths[i] = length;
    edge_loop_ix += length;
  }
  return i;
}

static inline vec3 cast_ray(const vec3 light,
                            const vec3 start)
{
  vec3 ray = start - light;
  return start + (normalize(ray) * 7.f);
}

void shadow_volume_mesh_rays(const vec3 light,
                             const vec3 * position,
                             const vec3 * cast_position,
                             const int * edge_loop,
                             const int edge_loop_length,
                             void(*render_quad)(vec3 a, vec3 b, vec3 c, vec3 d, bool l))
{
  for (int i = 0; i < edge_loop_length; i++) {
    int j = i + 1;
    if (j >= edge_loop_length) j = 0;

    int i1 = edge_loop[i];
    int i2 = edge_loop[j];

    vec3 a = position[i1];
    vec3 b = position[i2];
    vec3 c = cast_position[i2];
    vec3 d = cast_position[i1];

    bool last_in_volume = (i == (edge_loop_length - 1));
    render_quad(a, b, c, d, last_in_volume);
  }
}

void shadow_volume_end_caps(const vec3 light,
                            const vec3 * position,
                            const vec3 * cast_position,
                            const mesh * mesh,
                            const float * indicators,
                            void(*render_quad)(vec3 a, vec3 b, vec3 c, vec3 d, bool l))
{
  for (int i = 0; i < mesh->polygons_length; i++) {
    const polygon * p = &mesh->polygons[i];

    if (indicators[i] > 0) {
      vec3 a = position[p->a];
      vec3 b = position[p->b];
      vec3 c = position[p->c];
      vec3 d = position[p->d];
      render_quad(a, b, c, d, false);
    } else {
      vec3 a = cast_position[p->a];
      vec3 b = cast_position[p->b];
      vec3 c = cast_position[p->c];
      vec3 d = cast_position[p->d];
      render_quad(a, b, c, d, false);
    }
  }
}

void shadow_volume_mesh(const vec3 light,
                        const vec3 * position,
                        const vec3 * polygon_normal,
                        const mesh * mesh,
                        void(*render_quad)(vec3 a, vec3 b, vec3 c, vec3 d, bool l))
{
  // light in world space

  float indicators[mesh->polygon_normal_length];
  face_indicators(light, position, polygon_normal, mesh, indicators);

  // edge_indicies: mesh->edge_polygons indices
  int edge_indices[mesh->edge_polygons_length];
  int edge_indices_length = object_silhouette(indicators, mesh, edge_indices);

  // graph contains indexes to edge_indices (not mesh edge indices)
  graph graph[mesh->position_length];
  edge_loop_graph(mesh, edge_indices, edge_indices_length, graph);

  const int max_edge_loops = 2;
  int edge_loops[edge_indices_length];
  int edge_loop_lengths[max_edge_loops];
  int loop_count = edge_loop(mesh,
                             edge_indices,
                             edge_indices_length,
                             graph,
                             edge_loops,
                             edge_loop_lengths,
                             max_edge_loops);


  vec3 cast_position[mesh->position_length];
  for (int i = 0; i < mesh->position_length; i++) {
    cast_position[i] = cast_ray(light, position[i]);
  }

  shadow_volume_end_caps(light,
                         position,
                         cast_position,
                         mesh,
                         indicators,
                         render_quad);

  // edge_loops contains position indices
  int edge_loop_ix = 0;
  for (int i = 0; i < loop_count; i++) {
    int edge_loop_length = edge_loop_lengths[i];
    int * edge_loop = &edge_loops[edge_loop_ix];
    shadow_volume_mesh_rays(light,
                            position,
                            cast_position,
                            edge_loop,
                            edge_loop_length,
                            render_quad);
    edge_loop_ix += edge_loop_length;
  }

  if (0) {
    int edge_loop_ix = 0;
    for (int i = 0; i < loop_count; i++) {
      int length = edge_loop_lengths[i];
      printf("loop %d: %d\n", i, length);
      for (int j = 0; j < length; j++) {
        printf(" %d", edge_loops[j + edge_loop_ix]);
      }
      printf("\n\n");
      edge_loop_ix += length;
    }
  }

  /*
  for (int i = 0; i < silhouette_length; i++) {
    out_edges[edges[i]] = 1;
  }
  */
}
