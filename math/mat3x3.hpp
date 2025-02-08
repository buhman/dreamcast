#pragma once

#include <utility>
#include "vec3.hpp"

template <int R, int C, typename T>
struct mat;

//
// mat3x3
//

template <typename T>
struct mat<3, 3, T>
{
  typedef vec<3, T> row_type;
  typedef vec<3, T> col_type;

private:
  row_type value[3];

public:
  inline constexpr mat();

  inline constexpr mat
  (
    T const& a00, T const& a01, T const& a02,
    T const& a10, T const& a11, T const& a12,
    T const& a20, T const& a21, T const& a22
  );

  inline static constexpr int length() { return 3; }

  inline constexpr typename mat<3, 3, T>::row_type const &
  operator[](int i) const;

  void operator=(const mat<3, 3, T>&) = delete;

};


template<typename T>
inline constexpr mat<3, 3, T>::mat()
  : value{std::move(row_type(1, 0, 0)),
	  std::move(row_type(0, 1, 0)),
          std::move(row_type(0, 0, 1))}
{ }

template<typename T>
inline constexpr mat<3, 3, T>::mat
(
  T const& a00, T const& a01, T const& a02,
  T const& a10, T const& a11, T const& a12,
  T const& a20, T const& a21, T const& a22
)
  : value{std::move(row_type(a00, a01, a02)),
	  std::move(row_type(a10, a11, a12)),
          std::move(row_type(a20, a21, a22))}
{ }

template <typename T>
inline constexpr typename mat<3, 3, T>::row_type const &
mat<3, 3, T>::operator[](int i) const
{
  switch (i)
  {
  default: [[fallthrough]];
  case 0:
    return value[0];
  case 1:
    return value[1];
  case 2:
    return value[2];
  }
}

template<typename T>
inline constexpr mat<3, 3, T> operator+(mat<3, 3, T> const& m1, mat<3, 3, T> const& m2)
{
#define c(i, j) ( m1[i][j] + m2[i][j] )

  return mat<3, 3, T>(c(0,0), c(0,1), c(0,2),
                      c(1,0), c(1,1), c(1,2),
                      c(2,0), c(2,1), c(2,2));
#undef c
}

template<typename T>
inline constexpr mat<3, 3, T> operator*(mat<3, 3, T> const& m1, mat<3, 3, T> const& m2)
{
#define c(i, j) (                               \
    m1[i][0] * m2[0][j]                         \
  + m1[i][1] * m2[1][j]                         \
  + m1[i][2] * m2[2][j] )

  return mat<3, 3, T>(c(0,0), c(0,1), c(0,2),
                      c(1,0), c(1,1), c(1,2),
                      c(2,0), c(2,1), c(2,2));
#undef c
}

template<typename T>
inline constexpr mat<3, 3, T> operator*(mat<3, 3, T> const& m1, float s)
{
#define c(i, j) ( m1[i][j] * s )

  return mat<3, 3, T>(c(0,0), c(0,1), c(0,2),
                      c(1,0), c(1,1), c(1,2),
                      c(2,0), c(2,1), c(2,2));
#undef c
}

template<typename T>
inline constexpr typename mat<3, 3, T>::row_type operator*
(
  mat<3, 3, T> const& m,
  typename mat<3, 3, T>::col_type const& v
)
{
#define c(i) (                                  \
    m[i][0] * v[0]                              \
  + m[i][1] * v[1]                              \
  + m[i][2] * v[2] )

  return typename mat<3, 3, T>::row_type(c(0), c(1), c(2));
#undef c
}

template<typename T>
inline constexpr mat<3, 3, T> transpose(mat<3, 3, T> const& m)
{
  return mat<3, 3, T>(
    m[0][0], m[1][0], m[2][0],
    m[0][1], m[1][1], m[2][1],
    m[0][2], m[1][2], m[2][2]
  );
}
