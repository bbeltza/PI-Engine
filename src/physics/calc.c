#include <piengine/pi_render.h>
#include <piengine/pi_math.h>
#include "physics_internal.h"


PI_EXTERN_C struct PI_COLLISIONPROP_AABB* pi_get_mesh_aabb(struct PI_COLLISIONPROP_AABB* dst, const PI_VERTEX* vertices, unsigned num_vertices, const PI_MATRIX_4X4* ptransform)
{
    const float FLOAT_MAX = 3.402823e+38f;

    PI_VECTOR3X min = { FLOAT_MAX, FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
    PI_VECTOR3X max = { -FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX };
    for (unsigned i = 0; i < num_vertices; i++) {
        const PI_VERTEX* vt = &vertices[i];
        PI_VECTOR3X pos = { vt->pos.x, vt->pos.y, vt->pos.z };
        if (ptransform) {
            pos = pi_mulv_4x4(pos, ptransform);
        }

        min = pi_min4(min, pos);
        max = pi_max4(max, pos);
    }

    dst->min = min;
    dst->max = max;
    return dst;
}
