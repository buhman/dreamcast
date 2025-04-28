#include <tuple>

#include "math/vec.hpp"

namespace bezier {

template <typename T, int L, int M, int N>
struct vec_lmn {
  vec<L, T> l;
  vec<M, T> m;
  vec<N, T> n;
};

struct triangle {
  int a;
  int b;
  int c;
};

template <typename T, int L, int M, int N>
constexpr inline
vec_lmn<T, L, M, N>
interpolate_quadratic(const T d,
                      const vec<L, T>& l1, const vec<M, T>& m1, const vec<N, T>& n1,
                      const vec<L, T>& l2, const vec<M, T>& m2, const vec<N, T>& n2,
                      const vec<L, T>& l3, const vec<M, T>& m3, const vec<N, T>& n3)
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

  return {l, m, n};
}

template <typename T, int L, int M, int N>
constexpr inline
vec_lmn<T, L, M, N>
interpolate_quadratic(const T d,
                      const vec_lmn<T, L, M, N>& va,
                      const vec_lmn<T, L, M, N>& vb,
                      const vec_lmn<T, L, M, N>& vc)
{
  return interpolate_quadratic<T>(d,
                                  va.l, va.m, va.n,
                                  vb.l, vb.m, vb.n,
                                  vc.l, vc.m, vc.n);
}

template <typename T, int L, int M, int N>
constexpr inline
void
tessellate(const int level,
           const vec_lmn<T, L, M, N> * control, // [9]
           vec_lmn<T, L, M, N> * vertices,      // [2 * level * level]
           triangle * triangles,
           int vertex_base = 0)
{
  vec_lmn<T, L, M, N> column[3][level + 1];

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
