#pragma once

#include "math/float_types.hpp"
#include "model/blender_export.h"

void shadow_volume_mesh(const vec3 light,
                        const vec3 * position,
                        const vec3 * polygon_normal,
                        const mesh * mesh,
                        void(*render_quad)(vec3 a, vec3 b, vec3 c, vec3 d, bool l));
