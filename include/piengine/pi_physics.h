#ifndef PI_PHYSICS_H
#define PI_PHYSICS_H

#include <piengine/pi_common.h>
#include <piengine/pi_scalars.h>

typedef struct PI_RIGIDBODY_IMPL *PI_HRIGIDBODY;

#define PI_HRIGIDBODY_DESTRUCTOR pi_remove_rigid_body

PI_EXTERN_C PI_HSCENE pi_init_physics_scene(PI_HSCENE hscene);

typedef struct PI_PHYSICS_SCENE_PROPERTIES {
    float gravity;
    float forcedrag;
} PI_PHYSICS_SCENE_PROPERTIES;

PI_EXTERN_C PI_PHYSICS_SCENE_PROPERTIES* pi_get_physics_scene_properties(PI_HSCENE hscene);

PI_EXTERN_C PI_HRIGIDBODY pi_add_rigid_body(PI_HSCENE hscene);
PI_EXTERN_C void pi_remove_rigid_body(PI_HRIGIDBODY hrigidbody);

typedef enum PI_COLLISION_TYPE {
    PI_COLLISION_NONE = 0,
    PI_COLLISION_AABB = 1,
    PI_COLLISION_OBB = 2 /* Oriented-Bounding-Boxes, experimental. */
} PI_COLLISION_TYPE;


typedef struct PI_COLLISIONPROP_AABB {
    PI_VECTOR3X min;
    PI_VECTOR3X max;
} PI_COLLISIONPROP_AABB;
typedef struct PI_COLLISIONPROP_OBB {
    PI_VECTOR3X half_size;
    PI_VECTOR3X right;
    PI_VECTOR3X up;
} PI_COLLISIONPROP_OBB;
typedef struct PI_COLLISIONPROP_SPHERE {
    PI_VECTOR3X origin;
    float radius;
} PI_COLLISIONPROP_SPHERE;

typedef struct PI_COLLISION_PROPERTIES
{
    PI_COLLISION_TYPE type;
    union {
        PI_COLLISIONPROP_AABB aabb;
        PI_COLLISIONPROP_OBB obb;
        PI_COLLISIONPROP_SPHERE sphere;
    };
} PI_COLLISION_PROPERTIES;

PI_EXTERN_C bool pi_set_collision(PI_HRIGIDBODY hrigidbody, PI_COLLISION_TYPE type);
PI_EXTERN_C PI_COLLISION_PROPERTIES* pi_get_collision(PI_HRIGIDBODY hrigidbody);

PI_EXTERN_C bool pi_attach_transform(PI_HRIGIDBODY hrigidbody, PI_MATRIX_4X4* ptransform, void* p_dirtyflag);

typedef struct PI_RIGIDBODY_PROPERTIES
{
    float mass;
    float _padding[3];
    PI_VECTOR4 direction; // Right vector direction of the rigid body, the `w` component decides whether it points up or down. (1 or -1 respectively)
    PI_VECTOR4 scale;
    PI_VECTOR3X position;
    PI_VECTOR3X velocity;
    PI_VECTOR3X force;
} PI_RIGIDBODY_PROPERTIES;

PI_EXTERN_C PI_RIGIDBODY_PROPERTIES* pi_get_rigid_body_properties(PI_HRIGIDBODY hrigidbody);

struct PI_VERTEX;

PI_EXTERN_C struct PI_COLLISIONPROP_AABB* pi_get_mesh_aabb(struct PI_COLLISIONPROP_AABB* dst, struct PI_VERTEX const* vertices, unsigned num_vertices, const PI_MATRIX_4X4* transform);

#endif
