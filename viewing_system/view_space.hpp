#include "geometry/geometry.hpp"

namespace view_space {

  constexpr vec3 viewing_direction(const float azimuth,
                                   const float colatitude
                                   )
  {
    const float x = sin(colatitude) * cos(azimuth);
    const float y = sin(colatitude) * sin(azimuth);
    const float z = cos(colatitude);
    return {x, y, z};
  }


  constexpr vec3 project_vector_to_plane(const vec3& n,  // N: plane normal
                                         const vec3& v_  // V': approximate "up" orientation
                                         )
  {
    return v_ - dot(v_, n) * n;
  }

  constexpr mat4x4 transformation_matrix(const vec3& c,  // C: in world space, the position of the viewer
                                         const vec3& n,  // N: in world space, the viewing direction
                                         const vec3& v_  // V': approximate "up" orientation
                                         )
  {
    const vec3 v = project_vector_to_plane(n, v_);
    const vec3 u = cross(n, v);

    const mat4x4 t = { 1.f, 0.f, 0.f, -c.x,
                       0.f, 1.f, 0.f, -c.y,
                       0.f, 0.f, 1.f, -c.z,
                       0.f, 0.f, 0.f,  1.f };

    const mat4x4 r = { u.x, u.y, u.z, 0.f,
                       v.x, v.y, v.z, 0.f,
                       n.x, n.y, n.z, 0.f,
                       0.f, 0.f, 0.f, 1.f };

    return r * t;
  }

}
