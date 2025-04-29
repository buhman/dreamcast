#include <tuple>

#include "math/vec.hpp"

namespace bezier {

template <typename T, int L, int M, int N, int O>
struct vec_lmno {
  vec<L, T> l;
  vec<M, T> m;
  vec<N, T> n;
  vec<O, T> o;
};

struct triangle {
  int a;
  int b;
  int c;
};

template <typename T, int L, int M, int N, int O>
constexpr inline
vec_lmno<T, L, M, N, O>
interpolate_quadratic(const T d,
                      const vec<L, T>& l1, const vec<M, T>& m1, const vec<N, T>& n1, const vec<O, T>& o1,
                      const vec<L, T>& l2, const vec<M, T>& m2, const vec<N, T>& n2, const vec<O, T>& o2,
                      const vec<L, T>& l3, const vec<M, T>& m3, const vec<N, T>& n3, const vec<O, T>& o3)
{
  T invd = 1.0 - d;
  T d1 = invd * invd;
  T d2 = 2.0 * d * invd;
  T d3 = d * d;

  vec<L, T> l;
  for (int i = 0; i < L; i++) {
    l[i] = l1[i] * d1 + l2[i] * d2 + l3[i] * d3;
  };

  vec<M, T> m;
  for (int i = 0; i < M; i++) {
    m[i] = m1[i] * d1 + m2[i] * d2 + m3[i] * d3;
  }

  vec<N, T> n;
  for (int i = 0; i < N; i++) {
    n[i] = n1[i] * d1 + n2[i] * d2 + n3[i] * d3;
  };

  vec<O, T> o;
  for (int i = 0; i < O; i++) {
    o[i] = o1[i] * d1 + o2[i] * d2 + o3[i] * d3;
  };

  return {l, m, n, o};
}

template <typename T, int L, int M, int N, int O>
constexpr inline
vec_lmno<T, L, M, N, O>
interpolate_quadratic(const T d,
                      const vec_lmno<T, L, M, N, O>& va,
                      const vec_lmno<T, L, M, N, O>& vb,
                      const vec_lmno<T, L, M, N, O>& vc)
{
  return interpolate_quadratic<T>(d,
                                  va.l, va.m, va.n, va.o,
                                  vb.l, vb.m, vb.n, vb.o,
                                  vc.l, vc.m, vc.n, vc.o);
}

template <typename T, int L, int M, int N, int O>
constexpr inline
void
tessellate(const int level,
           const vec_lmno<T, L, M, N, O> * control, // [9]
           vec_lmno<T, L, M, N, O> * vertices,      // [2 * level * level]
           triangle * triangles,
           int vertex_base = 0)
{
  vec_lmno<T, L, M, N, O> column[3][level + 1];

  T inv_level = 1.0 / level;
  for (int i = 0; i <= level; i++) {
    T d = inv_level * i;
    column[0][i] = interpolate_quadratic(d, control[0], control[3], control[6]);
    column[1][i] = interpolate_quadratic(d, control[1], control[4], control[7]);
    column[2][i] = interpolate_quadratic(d, control[2], control[5], control[8]);
  }

  for (int i = 0; i <= level; i++) {
    for (int j = 0; j <= level; j++) {
      T d = inv_level * j;
      *vertices++ = interpolate_quadratic(d, column[0][i], column[1][i], column[2][i]);
    }
  }

  for (int i = 0; i < level; i++) {
    for (int j = 0; j < level; j++) {
      int ix = vertex_base + j * (level + 1) + i;
      *triangles++ = {ix + 0,
                      ix + (level + 1) + 0,
                      ix + (level + 1) + 1};
      *triangles++ = {ix + 0,
                      ix + (level + 1) + 1,
                      ix + 1};
    }
  }
}

}
