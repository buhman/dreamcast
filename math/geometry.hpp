#include <tuple>

#include "vec.hpp"

namespace geometry {

template <int L, typename T>
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

template <int L, typename T>
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

template <int L, typename T, int M>
constexpr inline
std::tuple<vec<L, T>, vec<M, T>> line_plane_intersection_two_lines(const vec<L, T>& plane_point,  // p0
								   const vec<L, T>& plane_normal, // n
								   const vec<L, T>& line_start,   // l0
								   const vec<L, T>& line_end,
								   const vec<M, T>& line2_start,
								   const vec<M, T>& line2_end
								   )
{
  /* it is assumed that line and line2 are the same line, but in different
     coordinates spaces. It is therefore possible to re-use the same
     interpolation value on either line vector.
  */

  const auto line_vector = line_end - line_start; // l
  const auto line2_vector = line2_end - line2_start;

  const T intersection = line_plane_intersection_d(plane_point, plane_normal, line_start, line_vector); // d

  return { line_start  + line_vector  * intersection
	 , line2_start + line2_vector * intersection
         };
}

template <int L, typename T>
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

template <int polygon_len, int L, typename T>
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

  bool this_inside = 0.f < clip_boundary<L, T>(plane_point,
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
      const auto i = line_plane_intersection<L, T>(plane_point, plane_normal, s, f);
      *output++ = i;
      *output++ = f;
    }
    break;
  case 0b10: // I
    length = 1;
    {
      const auto i = line_plane_intersection<L, T>(plane_point, plane_normal, s, f);
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
      clip_polygon1<polygon_len, L, T>(output,
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
  bool this_inside = 0.f < clip_boundary<L, T>(plane_point,
					       plane_normal,
					       f);

  return clip_polygon1<polygon_len, L, T>(output,
					  plane_point,
					  plane_normal,
					  polygon,
					  polygon_len - 1, // ix_s
					  0,               // ix_f
					  this_inside);
}

template <int polygon_len, int L, typename T, int M>
inline int clip_polygon1_uv(vec<L, T> * output,
			    vec<M, T> * output_uv,
			    const vec<L, T> plane_point,
			    const vec<L, T> plane_normal,
			    const vec<L, T> * polygon,
			    const vec<M, T> * polygon_uv,
			    const int ix_s,
			    const int ix_f,
			    const bool last_inside)
{
  const vec<L, T>& s = polygon[ix_s];
  const vec<L, T>& f = polygon[ix_f];

  const vec<M, T>& s_uv = polygon_uv[ix_s];
  const vec<M, T>& f_uv = polygon_uv[ix_f];

  bool this_inside = 0.f < clip_boundary<L, T>(plane_point,
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
      auto [i, i_uv] = line_plane_intersection_two_lines<L, T, M>(plane_point, plane_normal,
								  s, f,
								  s_uv, f_uv);
      *output++ = i;
      *output_uv++ = i_uv;
      *output++ = f;
      *output_uv++ = f_uv;
    }
    break;
  case 0b10: // I
    length = 1;
    {
      auto [i, i_uv] = line_plane_intersection_two_lines<L, T, M>(plane_point, plane_normal,
								  s, f,
								  s_uv, f_uv);
      *output++ = i;
      *output_uv++ = i_uv;
    }
    break;
  case 0b11: // F
    length = 1;
    *output++ = f;
    *output_uv++ = f_uv;
    break;
  }

  bool end_of_polygon = ix_f == (polygon_len - 1);
  if (!end_of_polygon) {
    return length +
      clip_polygon1_uv<polygon_len, L, T, M>(output,
					     output_uv,
					     plane_point,
					     plane_normal,
					     polygon,
					     polygon_uv,
					     ix_f,
					     ix_f + 1,
					     this_inside);
  } else {
    return length;
  }
}

template <int polygon_len, int L, typename T, int M>
int clip_polygon_uv(vec<L, T> * output,
		    vec<M, T> * output_uv,
		    const vec<L, T>& plane_point,
		    const vec<L, T>& plane_normal,
		    const vec<L, T> * polygon,
		    const vec<M, T> * polygon_uv
		    )
{
  const vec<L, T> f = polygon[polygon_len - 1];

  // It would be nice to remove the extra dot product, but the
  // alternative seems likely uglier.
  bool this_inside = 0.f < clip_boundary<L, T>(plane_point,
					       plane_normal,
					       f);

  return clip_polygon1_uv<polygon_len, L, T, M>(output,
						output_uv,
						plane_point,
						plane_normal,
						polygon,
						polygon_uv,
						polygon_len - 1, // ix_s
						0,               // ix_f
						this_inside);
}

}
