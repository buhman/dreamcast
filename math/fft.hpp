#pragma once

#include <cstdint>
#include "math.hpp"

namespace fft {

struct complex {
  float real;
  float imag;

  inline constexpr complex();
  inline constexpr complex(float real, float imag);
};

inline constexpr complex::complex()
  : real(0), imag(0)
{}

inline constexpr complex::complex(float real, float imag)
  : real(real), imag(imag)
{}

inline constexpr complex operator+(complex const& c1, complex const& c2)
{
  return complex(c1.real + c2.real,
		 c1.imag + c2.imag);
}

inline constexpr complex operator-(complex const& c1, complex const& c2)
{
  return complex(c1.real - c2.real,
		 c1.imag - c2.imag);
}

inline constexpr complex operator*(complex const& c1, complex const& c2)
{
  // (a+bi)(c+di) = (ac−bd) + (ad+bc)i
  return complex(c1.real * c2.real - c1.imag * c2.imag,
		 c1.real * c2.imag + c1.imag * c2.real);
}

constexpr float pi = 3.141592653589793;

static void fft(complex * x, int length)
{
  if (length == 1) {
    return;
  }

  complex even[length / 2];
  complex odd[length / 2];

  for (int i = 0; i < length / 2; i++) {
    even[i] = x[i * 2 + 0];
    odd[i] = x[i * 2 + 1];
  }

  fft(even, length / 2);
  fft(odd, length / 2);

  for (int k = 0; k < length / 2; k++) {
    float pi_k = -2 * pi * k / length;
    complex t = complex(cos(pi_k), sin(pi_k)) * odd[k];
    x[k] = even[k] + t;
    x[length / 2 + k] = even[k] - t;
  }
}

static void int16_to_complex(const int16_t * buf, int length,
			     complex * out)
{
  for (int i = 0; i < length; i++) {
    out[i] = complex(buf[i], 0);
  }
}

}
