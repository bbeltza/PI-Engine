#ifndef COMMON_CUBE_H
#define COMMON_CUBE_H

#include <piengine/pi_common.h>
#include <piengine/pi_render.h>

#ifdef PI_CPP
namespace common {
#endif

    PI_EXTERN_C const PI_VERTEX CUBE_VERTICES[24];
    PI_EXTERN_C const uint32_t CUBE_INDICES[36];
    PI_CONSTEXPR_V PI_INDEX CUBE_INDEX_TYPE = PI_INDEX_U32;

#ifdef PI_CPP
} 
#endif

#endif
