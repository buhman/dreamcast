#include <tuple>

#include "vec.hpp"

namespace geometry {

template <typename T, int L>
constexpr inline
T line_plane_intersection_d(const vec<L, T>& plane_point,  // p0
			    const vec<L, T>& plane_normal, // n
			    const vec<L, T>& line_start,   // l0
			    const vec<L, T>& line_vector   // l
			    )
{
  const T intersection = // d
      dot(plane_point - line_start, plane_normal)
    / dot(line_vector, plane_normal);

  return intersection;
}

template <typename T, int L>
constexpr inline
vec<L, T> line_plane_intersection(const vec<L, T>& plane_point,  // p0
				  const vec<L, T>& plane_normal, // n
				  const vec<L, T>& line_start,   // l0
				  const vec<L, T>& line_end
				  )
{
  const auto line_vector = line_end - line_start; // l

  const T intersection = line_plane_intersection_d(plane_point, plane_normal, line_start, line_vector); // d

  return line_start + line_vector * intersection;
}

template <typename T, int L>
constexpr inline
vec<L, T> interpolate(const vec<L, T>& line_start,
                      const vec<L, T>& line_end,
                      T intersection
                      )
{
  const auto line_vector = line_end - line_start; // l

  return line_start + line_vector * intersection;
}

template <typename T, int L>
T clip_boundary(const vec<L, T>& plane_point,  // X
		const vec<L, T>& plane_normal, // Nc
		const vec<L, T>& line_point
		)
{
  return dot(plane_normal, line_point - plane_point);
}

template <typename T>
inline T positive_modulo(T a, T b)
{
  return (a % b + b) % b;
}

template <int polygon_len, typename T, int L>
inline int clip_polygon1(vec<L, T> * output,
			 const vec<L, T> plane_point,
			 const vec<L, T> plane_normal,
			 const vec<L, T> * polygon,
			 const int ix_s,
			 const int ix_f,
			 const bool last_inside)
{
  const vec<L, T>& s = polygon[ix_s];
  const vec<L, T>& f = polygon[ix_f];

  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f);

  int length = 0;
  switch ((last_inside << 1) | (this_inside << 0)) {
  case 0b00: // no output
    length = 0;
    break;
  case 0b01: // I, F
    length = 2;
    {
      const auto i = line_plane_intersection<T, L>(plane_point, plane_normal, s, f);
      *output++ = i;
      *output++ = f;
    }
    break;
  case 0b10: // I
    length = 1;
    {
      const auto i = line_plane_intersection<T, L>(plane_point, plane_normal, s, f);
      *output++ = i;
    }
    break;
  case 0b11: // F
    length = 1;
    *output++ = f;
    break;
  }

  bool end_of_polygon = ix_f == (polygon_len - 1);
  if (!end_of_polygon) {
    return length +
      clip_polygon1<polygon_len, T, L>(output,
				       plane_point,
				       plane_normal,
				       polygon,
				       ix_f,
				       ix_f + 1,
				       this_inside);
  } else {
    return length;
  }
}

template <int polygon_len, int L, typename T>
int clip_polygon(vec<L, T> * output,
		 const vec<L, T>& plane_point,
		 const vec<L, T>& plane_normal,
		 const vec<L, T> * polygon
		 )
{
  const vec<L, T> f = polygon[polygon_len - 1];

  // It would be nice to remove the extra dot product, but the
  // alternative seems likely uglier.
  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f);

  return clip_polygon1<polygon_len, T, L>(output,
					  plane_point,
					  plane_normal,
					  polygon,
					  polygon_len - 1, // ix_s
					  0,               // ix_f
					  this_inside);
}

template <int polygon_len, typename T, int L, int M>
inline int clip_polygon1_2(vec<L, T> * output_1,
                           vec<M, T> * output_2,
                           const vec<L, T> plane_point,
                           const vec<L, T> plane_normal,
                           const vec<L, T> * polygon_1,
                           const vec<M, T> * polygon_2,
                           const int ix_s,
                           const int ix_f,
                           const bool last_inside)
{
  const vec<L, T>& s_1 = polygon_1[ix_s];
  const vec<L, T>& f_1 = polygon_1[ix_f];

  const vec<M, T>& s_2 = polygon_2[ix_s];
  const vec<M, T>& f_2 = polygon_2[ix_f];

  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f_1);

  int length = 0;
  int control = (last_inside << 1) | (this_inside << 0);
  switch (control) {
  case 0b00: // no output
    break;
  case 0b10: // I
    [[fallthrough]];
  case 0b01: // I, F
    {
      const auto& i_1_start = s_1;
      const auto i_1_vector = f_1 - s_1; // l
      const T intersection = line_plane_intersection_d<T, L>(plane_point, plane_normal,
                                                             i_1_start, i_1_vector);
      const vec<L, T> i_1 = i_1_start + i_1_vector * intersection;

      *output_1++ = i_1;
      *output_2++ = interpolate(s_2, f_2, intersection);

      if (control == 0b01) { // I, F
        *output_1++ = f_1;
        *output_2++ = f_2;
        length = 2;
      } else {
        length = 1;
      }
    }
    break;
  case 0b11: // F
    *output_1++ = f_1;
    *output_2++ = f_2;
    length = 1;
    break;
  }

  bool end_of_polygon = ix_f == (polygon_len - 1);
  if (!end_of_polygon) {
    return length +
      clip_polygon1_2<polygon_len, T, L, M>(output_1,
                                            output_2,
                                            plane_point,
                                            plane_normal,
                                            polygon_1,
                                            polygon_2,
                                            ix_f,
                                            ix_f + 1,
                                            this_inside);
  } else {
    return length;
  }
}

template <int polygon_len, typename T, int L, int M>
int clip_polygon_2(vec<L, T> * output_1,
                   vec<M, T> * output_2,
                   const vec<L, T>& plane_point,
                   const vec<L, T>& plane_normal,
                   const vec<L, T> * polygon_1,
                   const vec<M, T> * polygon_2
                   )
{
  const vec<L, T> f = polygon_1[polygon_len - 1];

  // It would be nice to remove the extra dot product, but the
  // alternative seems likely uglier.
  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f);

  return clip_polygon1_2<polygon_len, T, L, M>(output_1,
                                               output_2,
                                               plane_point,
                                               plane_normal,
                                               polygon_1,
                                               polygon_2,
                                               polygon_len - 1, // ix_s
                                               0,               // ix_f
                                               this_inside);
}

template <int polygon_len, typename T, int L, int M, int N>
inline int clip_polygon1_3(vec<L, T> * output_1,
                           vec<M, T> * output_2,
                           vec<N, T> * output_3,
                           const vec<L, T> plane_point,
                           const vec<L, T> plane_normal,
                           const vec<L, T> * polygon_1,
                           const vec<M, T> * polygon_2,
                           const vec<N, T> * polygon_3,
                           const int ix_s,
                           const int ix_f,
                           const bool last_inside)
{
  const vec<L, T>& s_1 = polygon_1[ix_s];
  const vec<L, T>& f_1 = polygon_1[ix_f];

  const vec<M, T>& s_2 = polygon_2[ix_s];
  const vec<M, T>& f_2 = polygon_2[ix_f];

  const vec<N, T>& s_3 = polygon_3[ix_s];
  const vec<N, T>& f_3 = polygon_3[ix_f];

  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f_1);

  int length = 0;
  int control = (last_inside << 1) | (this_inside << 0);
  switch (control) {
  case 0b00: // no output
    length = 0;
    break;
  case 0b10: // I
    [[fallthrough]];
  case 0b01: // I, F
    {
      const auto& i_1_start = s_1;
      const auto i_1_vector = f_1 - s_1; // l
      const T intersection = line_plane_intersection_d<T, L>(plane_point, plane_normal,
                                                             i_1_start, i_1_vector);
      const vec<L, T> i_1 = i_1_start + i_1_vector * intersection;

      *output_1++ = i_1;
      *output_2++ = interpolate(s_2, f_2, intersection);
      *output_3++ = interpolate(s_3, f_3, intersection);

      if (control == 0b01) { // I, F
        *output_1++ = f_1;
        *output_2++ = f_2;
        *output_3++ = f_3;
        length = 2;
      } else {
        length = 1;
      }
    }
    break;
  case 0b11: // F
    *output_1++ = f_1;
    *output_2++ = f_2;
    *output_3++ = f_3;
    length = 1;
    break;
  }

  bool end_of_polygon = ix_f == (polygon_len - 1);
  if (!end_of_polygon) {
    return length +
      clip_polygon1_3<polygon_len, T, L, M, N>(output_1,
                                               output_2,
                                               output_3,
                                               plane_point,
                                               plane_normal,
                                               polygon_1,
                                               polygon_2,
                                               polygon_3,
                                               ix_f,
                                               ix_f + 1,
                                               this_inside);
  } else {
    return length;
  }
}

template <int polygon_len, typename T, int L, int M, int N>
int clip_polygon_3(vec<L, T> * output_1,
                   vec<M, T> * output_2,
                   vec<N, T> * output_3,
                   const vec<L, T>& plane_point,
                   const vec<L, T>& plane_normal,
                   const vec<L, T> * polygon_1,
                   const vec<M, T> * polygon_2,
                   const vec<N, T> * polygon_3
                   )
{
  const vec<L, T> f = polygon_1[polygon_len - 1];

  // It would be nice to remove the extra dot product, but the
  // alternative seems likely uglier.
  bool this_inside = 0.f < clip_boundary<T, L>(plane_point,
					       plane_normal,
					       f);

  return clip_polygon1_3<polygon_len, T, L, M>(output_1,
                                               output_2,
                                               output_3,
                                               plane_point,
                                               plane_normal,
                                               polygon_1,
                                               polygon_2,
                                               polygon_3,
                                               polygon_len - 1, // ix_s
                                               0,               // ix_f
                                               this_inside);
}

}
