#ifndef PI_PHYSICS_H
#define PI_PHYSICS_H

#include <piengine/pi_common.h>
#include <piengine/pi_scalars.h>

typedef struct PI_RIGIDBODY_IMPL PI_HRIGIDBODY;

PI_EXTERN_C bool pi_init_physics_scene(PI_HSCENE hscene);


PI_EXTERN_C PI_HRIGIDBODY pi_add_rigid_body(PI_HSCENE hscene);
PI_EXTERN_C void pi_remove_rigid_body(PI_HRIGIDBODY hrigidbody);

typedef struct PI_RIGIDBODY_PROPERTIES
{
    bool isstatic;
    PI_VECTOR4 position;
    PI_VECTOR4 scale;
    PI_VECTOR4 direction; // Axis rotation (`w` component is the angle )
} PI_RIGIDBODY_PROPERTIES;

PI_EXTERN_C PI_RIGIDBODY_PROPERTIES* pi_get_rigid_body_properties(PI_HRIGIDBODY hrigidbody);

#endif
