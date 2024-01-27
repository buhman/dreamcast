#include <functional>

#include "vec.hpp"

namespace geometry {

template <int L, typename T>
vec<L, T> line_plane_intersection(const vec<L, T>& plane_point,  // p0
				  const vec<L, T>& plane_normal, // n
				  const vec<L, T>& line_start,   // l0
				  const vec<L, T>& line_end
				  )
{
  const auto line_vector = line_end - line_start; // l

  const T intersection = // d
      dot(plane_point - line_start, plane_normal)
    / dot(line_vector, plane_normal);

  return line_start + line_vector * intersection;
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
      auto i = line_plane_intersection<L, T>(plane_point, plane_normal, s, f);
      *output++ = i;
      *output++ = f;
    }
    break;
  case 0b10: // I
    length = 1;
    {
      auto i = line_plane_intersection<L, T>(plane_point, plane_normal, s, f);
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

}
