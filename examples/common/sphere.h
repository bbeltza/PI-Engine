#ifndef COMMON_SPHERE_H
#define COMMON_SPHERE_H

#include <piengine/pi_render.h>

#ifdef PI_CPP
namespace common {
#endif

    PI_EXTERN_C PI_VERTEX* generate_sphere_mesh(float radius, int h_vertices, int v_vertices, PI_VERTEX* vertex, int* indices, bool flatshaded PI_MEMBER_DEFAULT(true));
    PI_CONSTEXPR_V PI_INDEX SPHERE_INDEX_TYPE = PI_INDEX_U32;

    PI_CONSTEXPR_F size_t calc_num_sphere_vertices(int h_vertices, int v_vertices, bool flatshaded) {
         return h_vertices*v_vertices * (flatshaded ? 4 : 1) - (flatshaded ? h_vertices : 0);
    }

    PI_CONSTEXPR_F size_t calc_num_sphere_indices(int h_vertices, int v_vertices) {
        return h_vertices*(v_vertices-1)*6;
    }

#ifdef PI_CPP
} 
#endif

#endif
