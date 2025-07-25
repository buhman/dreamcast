#pragma once

template <int R, int C, typename T>
struct mat;

template <int R, int C, typename T>
inline constexpr typename mat<R, C, T>::col_type
col(mat<R, C, T> const& m, int c)
{
  typename mat<R, C, T>::col_type v;
  for (int r = 0; r < R; r++) {
    v[r] = m[r][c];
  }
  return v;
}

template <int R, typename T>
inline constexpr vec<3, T>
col(mat<R, 4, T> const& m, int c)
{
  vec<3, T> v;
  for (int r = 0; r < 3; r++) {
    v[r] = m[r][c];
  }
  return v;
}
