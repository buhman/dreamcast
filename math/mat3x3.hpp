#pragma once

#include <utility>
#include "vec3.hpp"
#include "mat.hpp"

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

  inline constexpr typename mat<3, 3, T>::row_type &
  operator[](int i);

  inline constexpr typename mat<3, 3, T>::row_type const &
  operator[](int i) const;

  //void operator=(const mat<3, 3, T>&) = delete;

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
inline constexpr typename mat<3, 3, T>::row_type &
mat<3, 3, T>::operator[](int i)
{
  return value[i];
}

template <typename T>
inline constexpr typename mat<3, 3, T>::row_type const &
mat<3, 3, T>::operator[](int i) const
{
  return value[i];
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

template<typename T>
inline constexpr mat<2, 2, T> submatrix(mat<3, 3, T> const& a, int r, int c)
{
  mat<2, 2, T> b;
  int row2 = 0;
  for (int row3 = 0; row3 < 3; row3++) {
    if (row3 == r) continue;
    int col2 = 0;
    for (int col3 = 0; col3 < 3; col3++) {
      if (col3 == c) continue;
      b[row2][col2] = a[row3][col3];
      col2++;
    }
    row2++;
  }
  return b;
}

template<typename T>
inline constexpr float minor(mat<3, 3, T> const& a, int r, int c)
{
  mat<2, 2, T> s = submatrix(a, r, c);
  float ret = determinant(s);
  return ret;
}

template<typename T>
inline constexpr float cofactor(mat<3, 3, T> const& a, int r, int c)
{
  float m = minor(a, r, c);
  if ((r + c) & 1)
    return -m;
  else
    return m;
}

template<typename T>
inline constexpr float determinant(mat<3, 3, T> const& a)
{
  float f0 = cofactor(a, 0, 0);
  float f1 = cofactor(a, 0, 1);
  float f2 = cofactor(a, 0, 2);
  return
    a[0][0] * f0 +
    a[0][1] * f1 +
    a[0][2] * f2;
}

template<typename T>
inline constexpr mat<3, 3, T> inverse(mat<3, 3, T> const& a)
{
  mat<3, 3, T> m;
  float idet = 1.0f / determinant(a);
  m[0][0] = cofactor(a, 0, 0) * idet;
  m[1][0] = cofactor(a, 0, 1) * idet;
  m[2][0] = cofactor(a, 0, 2) * idet;
  m[0][1] = cofactor(a, 1, 0) * idet;
  m[1][1] = cofactor(a, 1, 1) * idet;
  m[2][1] = cofactor(a, 1, 2) * idet;
  m[0][2] = cofactor(a, 2, 0) * idet;
  m[1][2] = cofactor(a, 2, 1) * idet;
  m[2][2] = cofactor(a, 2, 2) * idet;
  return m;
}
