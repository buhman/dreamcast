#pragma once

#include <utility>
#include "vec4.hpp"
#include "mat.hpp"

//
// mat4x4
//

template <typename T>
struct mat<4, 4, T>
{
  typedef vec<4, T> row_type;
  typedef vec<4, T> col_type;

private:
  row_type value[4];

public:
  inline constexpr mat();

  inline constexpr mat
  (
    T const& a00, T const& a01, T const& a02, T const& a03,
    T const& a10, T const& a11, T const& a12, T const& a13,
    T const& a20, T const& a21, T const& a22, T const& a23,
    T const& a30, T const& a31, T const& a32, T const& a33
  );

  inline static constexpr int length() { return 4; }

  inline constexpr typename mat<4, 4, T>::row_type &
  operator[](int i);

  inline constexpr typename mat<4, 4, T>::row_type const &
  operator[](int i) const;

  //void operator=(const mat<4, 4, T>&) = delete;

};


template<typename T>
inline constexpr mat<4, 4, T>::mat()
  : value{std::move(row_type(1, 0, 0, 0)),
	  std::move(row_type(0, 1, 0, 0)),
          std::move(row_type(0, 0, 1, 0)),
          std::move(row_type(0, 0, 0, 1))}
{ }

template<typename T>
inline constexpr mat<4, 4, T>::mat
(
  T const& a00, T const& a01, T const& a02, T const& a03,
  T const& a10, T const& a11, T const& a12, T const& a13,
  T const& a20, T const& a21, T const& a22, T const& a23,
  T const& a30, T const& a31, T const& a32, T const& a33
)
  : value{std::move(row_type(a00, a01, a02, a03)),
	  std::move(row_type(a10, a11, a12, a13)),
          std::move(row_type(a20, a21, a22, a23)),
          std::move(row_type(a30, a31, a32, a33))}
{ }


template <typename T>
inline constexpr typename mat<4, 4, T>::row_type &
mat<4, 4, T>::operator[](int i)
{
  return value[i];
}

template <typename T>
inline constexpr typename mat<4, 4, T>::row_type const &
mat<4, 4, T>::operator[](int i) const
{
  return value[i];
}

template<typename T>
inline constexpr mat<4, 4, T> operator+(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2)
{
#define c(i, j) ( m1[i][j] + m2[i][j] )

  return mat<4, 4, T>(c(0,0), c(0,1), c(0,2), c(0,3),
                      c(1,0), c(1,1), c(1,2), c(1,3),
                      c(2,0), c(2,1), c(2,2), c(2,3),
                      c(3,0), c(3,1), c(3,2), c(3,3));
#undef c
}

template<typename T>
inline constexpr mat<4, 4, T> operator*(mat<4, 4, T> const& m1, mat<4, 4, T> const& m2)
{
#define c(i, j) (                               \
    m1[i][0] * m2[0][j]                         \
  + m1[i][1] * m2[1][j]                         \
  + m1[i][2] * m2[2][j]                         \
  + m1[i][3] * m2[3][j] )

  return mat<4, 4, T>(c(0,0), c(0,1), c(0,2), c(0,3),
                      c(1,0), c(1,1), c(1,2), c(1,3),
                      c(2,0), c(2,1), c(2,2), c(2,3),
                      c(3,0), c(3,1), c(3,2), c(3,3));
#undef c
}

template<typename T>
inline constexpr mat<4, 4, T> operator*(mat<4, 4, T> const& m1, float s)
{
#define c(i, j) ( m1[i][j] * s )

  return mat<4, 4, T>(c(0,0), c(0,1), c(0,2), c(0,3),
                      c(1,0), c(1,1), c(1,2), c(1,3),
                      c(2,0), c(2,1), c(2,2), c(2,3),
                      c(3,0), c(3,1), c(3,2), c(3,3));
#undef c
}

template<typename T>
inline constexpr typename mat<4, 4, T>::row_type operator*
(
  mat<4, 4, T> const& m,
  typename mat<4, 4, T>::col_type const& v
)
{
#define c(i) (                                  \
    m[i][0] * v[0]                              \
  + m[i][1] * v[1]                              \
  + m[i][2] * v[2]                              \
  + m[i][3] * v[3] )

  return typename mat<4, 4, T>::row_type(c(0), c(1), c(2), c(3));
#undef c
}

template<typename T>
inline constexpr vec<3, T> operator*
(
  mat<4, 4, T> const& m,
  vec<3, T> const& v
)
{
#define c(i) (                                  \
    m[i][0] * v[0]                              \
  + m[i][1] * v[1]                              \
  + m[i][2] * v[2]                              \
  + m[i][3] )

  return vec<3, T>(c(0), c(1), c(2));
#undef c
}

template<typename T>
inline constexpr mat<4, 4, T> transpose(mat<4, 4, T> const& m)
{
  return mat<4, 4, T>(
    m[0][0], m[1][0], m[2][0], m[3][0],
    m[0][1], m[1][1], m[2][1], m[3][1],
    m[0][2], m[1][2], m[2][2], m[3][2],
    m[0][3], m[1][3], m[2][3], m[3][3]
  );
}

template<typename T>
inline constexpr mat<3, 3, T> submatrix(mat<4, 4, T> const& a, int r, int c)
{
  mat<3, 3, T> b;
  int row3 = 0;
  for (int row4 = 0; row4 < 4; row4++) {
    if (row4 == r) continue;
    int col3 = 0;
    for (int col4 = 0; col4 < 4; col4++) {
      if (col4 == c) continue;
      b[row3][col3] = a[row4][col4];
      col3++;
    }
    row3++;
  }
  return b;
}

template<typename T>
inline constexpr float minor(mat<4, 4, T> const& a, int r, int c)
{
  mat<3, 3, T> s = submatrix(a, r, c);
  float ret = determinant(s);
  return ret;
}

template<typename T>
inline constexpr float cofactor(mat<4, 4, T> const& a, int r, int c)
{
  float m = minor(a, r, c);
  if ((r + c) & 1)
    return -m;
  else
    return m;
}

template<typename T>
inline constexpr float determinant(mat<4, 4, T> const& a)
{
  float f0 = cofactor(a, 0, 0);
  float f1 = cofactor(a, 0, 1);
  float f2 = cofactor(a, 0, 2);
  float f3 = cofactor(a, 0, 3);
  return
    a[0][0] * f0 +
    a[0][1] * f1 +
    a[0][2] * f2 +
    a[0][3] * f3;
}

template<typename T>
inline constexpr mat<4, 4, T> inverse(mat<4, 4, T> const& a)
{
  mat<4, 4, T> m;
  float idet = 1.0f / determinant(a);
  m[0][0] = cofactor(a, 0, 0) * idet;
  m[1][0] = cofactor(a, 0, 1) * idet;
  m[2][0] = cofactor(a, 0, 2) * idet;
  m[3][0] = cofactor(a, 0, 3) * idet;
  m[0][1] = cofactor(a, 1, 0) * idet;
  m[1][1] = cofactor(a, 1, 1) * idet;
  m[2][1] = cofactor(a, 1, 2) * idet;
  m[3][1] = cofactor(a, 1, 3) * idet;
  m[0][2] = cofactor(a, 2, 0) * idet;
  m[1][2] = cofactor(a, 2, 1) * idet;
  m[2][2] = cofactor(a, 2, 2) * idet;
  m[3][2] = cofactor(a, 2, 3) * idet;
  m[0][3] = cofactor(a, 3, 0) * idet;
  m[1][3] = cofactor(a, 3, 1) * idet;
  m[2][3] = cofactor(a, 3, 2) * idet;
  m[3][3] = cofactor(a, 3, 3) * idet;
  return m;
}

template<typename T>
inline constexpr mat<4, 4, T> identity()
{
  return mat<4, 4, T>(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}
