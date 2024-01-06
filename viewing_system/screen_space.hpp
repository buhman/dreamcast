#include "geometry/geometry.hpp"

namespace screen_space {

  constexpr mat4x4 transformation_matrix(const float d, // the z-coordinate of the view window and the near clip plane
                                         const float f, // the z-coordnate of the far clip plane
                                         const float h  // the dimension of the square view window
                                         )
  {
    return { d/h, 0.f, 0.f    , 0.f       ,
             0.f, d/h, 0.f    , 0.f       ,
             0.f, 0.f, f/(f-d), -d*f/(f-d),
             0.f, 0.f, 1.f    , 0.f        };
  }
}
