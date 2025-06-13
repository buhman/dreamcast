#pragma once

template <int L, typename T>
struct vec;

template <int L, typename T>
inline constexpr T magnitude(vec<L, T> const& v)
{
  return sqrt(dot(v, v));
}

template <int L, typename T>
inline constexpr T magnitude_squared(vec<L, T> const& v)
{
  return dot(v, v);
}

template <int L, typename T>
inline constexpr vec<3, T> normalize(vec<L, T> const& v)
{
  return v / magnitude(v);
}

template <int L, typename T>
inline constexpr vec<3, T> reflect(vec<L, T> const& i, vec<L, T> const& n)
{
  return i - dot(n, i) * n * static_cast<T>(2.0);
}
