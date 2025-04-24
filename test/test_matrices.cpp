#include <stdbool.h>
#include <stdio.h>

#include "runner.hpp"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat2x2.hpp"
#include "math/mat3x3.hpp"
#include "math/mat4x4.hpp"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat2x2 = mat<2, 2, float>;
using mat3x3 = mat<3, 3, float>;
using mat4x4 = mat<4, 4, float>;

static const float epsilon = 0.00001;

#define fabsf __builtin_fabsf

inline static bool float_equal(float a, float b)
{
  return fabsf(a - b) < epsilon;
}

inline static bool vec4_equal(vec4 a, vec4 b)
{
  return
    float_equal(a.x, b.x) &&
    float_equal(a.y, b.y) &&
    float_equal(a.z, b.z) &&
    float_equal(a.w, b.w);
}

inline static bool mat2x2_equal(mat2x2 a,
                                mat2x2 b)
{
  return
    float_equal(a[0][0], b[0][0]) &&
    float_equal(a[0][1], b[0][1]) &&
    float_equal(a[1][0], b[1][0]) &&
    float_equal(a[1][1], b[1][1]);
}

inline static bool mat3x3_equal(mat3x3 a,
                                mat3x3 b)
{
  return
    float_equal(a[0][0], b[0][0]) &&
    float_equal(a[0][1], b[0][1]) &&
    float_equal(a[0][2], b[0][2]) &&
    float_equal(a[1][0], b[1][0]) &&
    float_equal(a[1][1], b[1][1]) &&
    float_equal(a[1][2], b[1][2]) &&
    float_equal(a[2][0], b[2][0]) &&
    float_equal(a[2][1], b[2][1]) &&
    float_equal(a[2][2], b[2][2]);
}

inline static bool mat4x4_equal(mat4x4 a,
                                mat4x4 b)
{
  return
    float_equal(a[0][0], b[0][0]) &&
    float_equal(a[0][1], b[0][1]) &&
    float_equal(a[0][2], b[0][2]) &&
    float_equal(a[0][3], b[0][3]) &&
    float_equal(a[1][0], b[1][0]) &&
    float_equal(a[1][1], b[1][1]) &&
    float_equal(a[1][2], b[1][2]) &&
    float_equal(a[1][3], b[1][3]) &&
    float_equal(a[2][0], b[2][0]) &&
    float_equal(a[2][1], b[2][1]) &&
    float_equal(a[2][2], b[2][2]) &&
    float_equal(a[2][3], b[2][3]) &&
    float_equal(a[3][0], b[3][0]) &&
    float_equal(a[3][1], b[3][1]) &&
    float_equal(a[3][2], b[3][2]) &&
    float_equal(a[3][3], b[3][3]);
}

inline static bool mat4x4_is_invertible(mat4x4 a)
{
  return !float_equal(determinant(a), 0.f);
}

static bool matrices_test_0(const char ** scenario)
{
  *scenario = "A 4x4 matrix ought to be representable";
  mat4x4 m = mat4x4( 1.0f,  2.0f,  3.0f,  4.0f,
                            5.5f,  6.5f,  7.5f,  8.5f,
                            9.0f, 10.0f, 11.0f, 12.0f,
                           13.5f, 14.5f, 15.5f, 16.5f);
  return
    m[0][0] == 1.0f &&
    m[0][3] == 4.0f &&
    m[1][0] == 5.5f &&
    m[1][2] == 7.5f &&
    m[2][2] == 11.0f &&
    m[3][0] == 13.5f &&
    m[3][2] == 15.5f;
}

static bool matrices_test_1(const char ** scenario)
{
  *scenario = "A 2x2 matrix ought to be representable";
  mat2x2 m = mat2x2(-3.0f,  5.0f,
                            1.0f, -2.0f);
  return
    m[0][0] == -3.0f &&
    m[0][1] == 5.0f &&
    m[1][0] == 1.0f &&
    m[1][1] == -2.0f;
}

static bool matrices_test_2(const char ** scenario)
{
  *scenario = "A 3x3 matrix ought to be representable";
  mat3x3 m = mat3x3(-3.0f,  5.0f,  0.0f,
                            1.0f, -2.0f, -7.0f,
                            0.0f,  1.0f,  1.0f);
  return
    m[0][0] == -3.0f &&
    m[1][1] == -2.0f &&
    m[2][2] ==  1.0f;
}

static bool matrices_test_3(const char ** scenario)
{
  *scenario = "Matrix equality with identical matrices";
  mat4x4 a = mat4x4(1.0f, 2.0f, 3.0f, 4.0f,
                           5.0f, 6.0f, 7.0f, 8.0f,
                           9.0f, 8.0f, 7.0f, 6.0f,
                           5.0f, 4.0f, 3.0f, 2.0f);

  mat4x4 b = mat4x4(1.0f, 2.0f, 3.0f, 4.0f,
                           5.0f, 6.0f, 7.0f, 8.0f,
                           9.0f, 8.0f, 7.0f, 6.0f,
                           5.0f, 4.0f, 3.0f, 2.0f);

  return mat4x4_equal(a, b);
}

static bool matrices_test_4(const char ** scenario)
{
  *scenario = "Matrix equality with different matrices";

  mat4x4 a = mat4x4(1.0f, 2.0f, 3.0f, 4.0f,
                           5.0f, 6.0f, 7.0f, 8.0f,
                           9.0f, 8.0f, 7.0f, 6.0f,
                           5.0f, 4.0f, 3.0f, 2.0f);

  mat4x4 b = mat4x4(2.0f, 3.0f, 4.0f, 5.0f,
                           6.0f, 7.0f, 8.0f, 9.0f,
                           8.0f, 7.0f, 6.0f, 5.0f,
                           4.0f, 3.0f, 2.0f, 1.0f);

  return !mat4x4_equal(a, b);
}

static bool matrices_test_5(const char ** scenario)
{
  *scenario = "Multiplying two matrices";

  mat4x4 a = mat4x4(1.0f, 2.0f, 3.0f, 4.0f,
                           5.0f, 6.0f, 7.0f, 8.0f,
                           9.0f, 8.0f, 7.0f, 6.0f,
                           5.0f, 4.0f, 3.0f, 2.0f);

  mat4x4 b = mat4x4(-2.0f, 1.0f, 2.0f,  3.0f,
                            3.0f, 2.0f, 1.0f, -1.0f,
                            4.0f, 3.0f, 6.0f,  5.0f,
                            1.0f, 2.0f, 7.0f,  8.0f);

  mat4x4 m1 = a * b;
  mat4x4 m2 = mat4x4(20.0f, 22.0f,  50.0f,  48.0f,
                            44.0f, 54.0f, 114.0f, 108.0f,
                            40.0f, 58.0f, 110.0f, 102.0f,
                            16.0f, 26.0f,  46.0f,  42.0f);
  return mat4x4_equal(m1, m2);
}

static bool matrices_test_6(const char ** scenario)
{
  *scenario = "A matrix multiplied by a vec4";

  mat4x4 a = mat4x4(1.0f, 2.0f, 3.0f, 4.0f,
                           2.0f, 4.0f, 4.0f, 2.0f,
                           8.0f, 6.0f, 4.0f, 1.0f,
                           0.0f, 0.0f, 0.0f, 1.0f);
  vec4 b = vec4(1.0f, 2.0f, 3.0f, 1.0f);

  vec4 c = a * b;
  return vec4_equal(c, vec4(18, 24, 33, 1));
}

static bool matrices_test_7(const char ** scenario)
{
  *scenario = "Multiplying a matrix by the identity matrix";

  mat4x4 a = mat4x4(0.0f, 1.0f,  2.0f,  4.0f,
                           1.0f, 2.0f,  4.0f,  8.0f,
                           2.0f, 4.0f,  8.0f, 16.0f,
                           4.0f, 8.0f, 16.0f, 32.0f);

  mat4x4 id = identity<float>();
  mat4x4 c = a * id;
  return mat4x4_equal(a, c);
}

static bool matrices_test_8(const char ** scenario)
{
  *scenario = "Multiplying a the identity matrix by a vec4";

  vec4 a = vec4(1.0f, 2.0f, 3.0f, 4.0f);

  mat4x4 id = identity<float>();
  vec4 c = id * a;
  return vec4_equal(a, c);
}

static bool matrices_test_9(const char ** scenario)
{
  *scenario = "Transposing a matrix";

  mat4x4 a = mat4x4(0.0f, 9.0f, 3.0f, 0.0f,
                           9.0f, 8.0f, 0.0f, 8.0f,
                           1.0f, 8.0f, 5.0f, 3.0f,
                           0.0f, 0.0f, 5.0f, 8.0f);

  mat4x4 b = mat4x4(0.0f, 9.0f, 1.0f, 0.0f,
                           9.0f, 8.0f, 8.0f, 0.0f,
                           3.0f, 0.0f, 5.0f, 5.0f,
                           0.0f, 8.0f, 3.0f, 8.0f);

  mat4x4 c = transpose(a);

  return mat4x4_equal(c, b);
}

static bool matrices_test_10(const char ** scenario)
{
  *scenario = "Transposing the identity matrix";

  mat4x4 a = identity<float>();

  mat4x4 c = transpose(a);

  return mat4x4_equal(a, c);
}

static bool matrices_test_11(const char ** scenario)
{
  *scenario = "Calculating the determinant of a 2x2 matrix";

  mat2x2 a = mat2x2( 1.0f, 5.0f,
                           -3.0f, 2.0f);

  return float_equal(determinant(a), 17);
}

static bool matrices_test_12(const char ** scenario)
{
  *scenario = "A submatrix of a 3x3 matrix is a 2x2 matrix";

  mat3x3 a = mat3x3( 1.0f, 5.0f,  0.0f,
                           -3.0f, 2.0f,  7.0f,
                            0.0f, 6.0f, -3.0f);

  mat2x2 b = submatrix(a, 0, 2);
  mat2x2 c = mat2x2(-3.0f, 2.0f,
                            0.0f, 6.0f);

  return mat2x2_equal(b, c);
}

static bool matrices_test_13(const char ** scenario)
{
  *scenario = "A submatrix of a 4x4 matrix is a 3x3 matrix";

  mat4x4 a = mat4x4(-6.0f, 1.0f,  1.0f, 6.0f,
                    -8.0f, 5.0f,  8.0f, 6.0f,
                    -1.0f, 0.0f,  8.0f, 2.0f,
                    -7.0f, 1.0f, -1.0f, 1.0f);

  mat3x3 b = submatrix(a, 2, 1);
  mat3x3 c = mat3x3(-6.0f,  1.0f, 6.0f,
                    -8.0f,  8.0f, 6.0f,
                    -7.0f, -1.0f, 1.0f);

  return mat3x3_equal(b, c);
}

static bool matrices_test_14(const char ** scenario)
{
  *scenario = "Calculating a minor of a 3x3 matrix";

  mat3x3 a = mat3x3(3,  5,  0,
                    2, -1, -7,
                    6, -1,  5);
  mat2x2 b = submatrix(a, 1, 0);

  return
    float_equal(determinant(b), 25.0f) &&
    float_equal(minor(a, 1, 0), 25.0f);
}

static bool matrices_test_15(const char ** scenario)
{
  *scenario = "Calculating a cofactor of a 3x3 matrix";

  mat3x3 a = mat3x3(3,  5,  0,
                    2, -1, -7,
                    6, -1,  5);

  return
    float_equal(minor(a, 0, 0), -12.0f) &&
    float_equal(cofactor(a, 0, 0), -12.0f) &&
    float_equal(minor(a, 1, 0),  25.0f) &&
    float_equal(cofactor(a, 1, 0), -25.0f) &&
    float_equal(cofactor(a, 2, 1), 21.0f);

}

static bool matrices_test_16(const char ** scenario)
{
  *scenario = "Calculating the determinant of a 3x3 matrix";

  mat3x3 a = mat3x3( 1.0f, 2.0f,  6.0f,
                           -5.0f, 8.0f, -4.0f,
                            2.0f, 6.0f,  4.0f);

  return
    float_equal(cofactor(a, 0, 0), 56.0f) &&
    float_equal(cofactor(a, 0, 1), 12.0f) &&
    float_equal(cofactor(a, 0, 2), -46.0f) &&
    float_equal(determinant(a), -196.0f);
}

static bool matrices_test_17(const char ** scenario)
{
  *scenario = "Calculating the determinant of a 4x4 matrix";

  mat4x4 a = mat4x4(-2.0f, -8.0f,  3.0f,  5.0f,
                           -3.0f,  1.0f,  7.0f,  3.0f,
                            1.0f,  2.0f, -9.0f,  6.0f,
                           -6.0f,  7.0f,  7.0f, -9.0f);

  return
    float_equal(cofactor(a, 0, 0), 690.0f) &&
    float_equal(cofactor(a, 0, 1), 447.0f) &&
    float_equal(cofactor(a, 0, 2), 210.0f) &&
    float_equal(cofactor(a, 0, 3), 51.0f) &&
    float_equal(determinant(a), -4071.0f);
}

static bool matrices_test_18(const char ** scenario)
{
  *scenario = "Testing an invertible matrix for invertibility";

  mat4x4 a = mat4x4(6.0f,  4.0f, 4.0f,  4.0f,
                           5.0f,  5.0f, 7.0f,  6.0f,
                           4.0f, -9.0f, 3.0f, -7.0f,
                           9.0f,  1.0f, 7.0f, -6.0f);

  return
    float_equal(determinant(a), -2120.0f) &&
    mat4x4_is_invertible(a);
}

static bool matrices_test_19(const char ** scenario)
{
  *scenario = "Testing a noninvertible matrix for invertibility";

  mat4x4 a = mat4x4(-4.0f,  2.0f, -2.0f, -3.0f,
                            9.0f,  6.0f,  2.0f,  6.0f,
                            0.0f, -5.0f,  1.0f, -5.0f,
                            0.0f,  0.0f,  0.0f,  0.0f);

  return
    float_equal(determinant(a), 0.0f) &&
    !mat4x4_is_invertible(a);
}

static bool matrices_test_20(const char ** scenario)
{
  *scenario = "Calculating the inverse of a matrix";

  mat4x4 a = mat4x4(-5.0,  2.0,  6.0, -8.0,
                            1.0, -5.0,  1.0,  8.0,
                            7.0,  7.0, -6.0, -7.0,
                            1.0, -3.0,  7.0,  4.0);
  mat4x4 b = inverse(a);

  mat4x4 c = mat4x4( 0.218045f,  0.451128f,  0.240602f, -0.045113f,
                           -0.808271f, -1.456767f, -0.443609f,  0.520677f,
                           -0.078947f, -0.223684f, -0.052632f,  0.197368f,
                           -0.522564f, -0.813910f, -0.300752f,  0.306391f);

  return
    float_equal(determinant(a), 532.0f) &&
    float_equal(cofactor(a, 2, 3), -160.0f) &&
    float_equal(b[3][2], -160.0f/532.0f) &&
    float_equal(cofactor(a, 3, 2), 105.0f) &&
    float_equal(b[2][3], 105.0f/532.0f) &&
    mat4x4_equal(b, c);
}

static bool matrices_test_21(const char ** scenario)
{
  *scenario = "Calculating the inverse of a second matrix";

  mat4x4 a = mat4x4( 8.0f, -5.0f,  9.0f,  2.0f,
                            7.0f,  5.0f,  6.0f,  1.0f,
                           -6.0f,  0.0f,  9.0f,  6.0f,
                           -3.0f,  0.0f, -9.0f, -4.0f);

  mat4x4 b = inverse(a);

  mat4x4 c = mat4x4(-0.15385f, -0.15385f, -0.28205f, -0.53846f,
                    -0.07692f,  0.12308f,  0.02564f,  0.03077f,
                    0.35897f,  0.35897f,  0.43590f,  0.92308f,
                    -0.69231f, -0.69231f, -0.76923f, -1.92308f);

  return mat4x4_equal(b, c);
}

static bool matrices_test_22(const char ** scenario)
{
  *scenario = "Calculating the inverse of a third matrix";

  mat4x4 a = mat4x4( 9.0f,  3.0f,  0.0f,  9.0f,
                           -5.0f, -2.0f, -6.0f, -3.0f,
                           -4.0f,  9.0f,  6.0f,  4.0f,
                           -7.0f,  6.0f,  6.0f,  2.0f);

  mat4x4 b = inverse(a);

  mat4x4 c = mat4x4(-0.04074f, -0.07778f,  0.14444f, -0.22222f,
                           -0.07778f,  0.03333f,  0.36667f, -0.33333f,
                           -0.02901f, -0.14630f, -0.10926f,  0.12963f,
                            0.17778f,  0.06667f, -0.26667f,  0.33333f);

  return mat4x4_equal(b, c);
}

static bool matrices_test_23(const char ** scenario)
{
  *scenario = "Multiplying a product by its inverse";

  mat4x4 a = mat4x4( 3.0f, -9.0f,  7.0f,  3.0f,
                            3.0f, -8.0f,  2.0f, -9.0f,
                           -4.0f,  4.0f,  4.0f,  1.0f,
                           -6.0f,  5.0f, -1.0f,  1.0f);

  mat4x4 b = mat4x4( 8.0f,  2.0f,  2.0f,  2.0f,
                            3.0f, -1.0f,  7.0f,  0.0f,
                            7.0f,  0.0f,  5.0f,  4.0f,
                            6.0f, -2.0f,  0.0f,  5.0f);

  mat4x4 c = a * b;

  mat4x4 b_i = inverse(b);
  mat4x4 d = c * b_i;

  return mat4x4_equal(d, a);
}

test_t matrices_tests[] = {
  matrices_test_0,
  matrices_test_1,
  matrices_test_2,
  matrices_test_3,
  matrices_test_4,
  matrices_test_5,
  matrices_test_6,
  matrices_test_7,
  matrices_test_8,
  matrices_test_9,
  matrices_test_10,
  matrices_test_11,
  matrices_test_12,
  matrices_test_13,
  matrices_test_14,
  matrices_test_15,
  matrices_test_16,
  matrices_test_17,
  matrices_test_18,
  matrices_test_19,
  matrices_test_20,
  matrices_test_21,
  matrices_test_22,
  matrices_test_23,
};

RUNNER(matrices_tests)
