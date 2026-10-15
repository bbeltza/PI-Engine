#ifndef PI_PHYSICS_INTERNAL_H
#define PI_PHYSICS_INTERNAL_H

#include <piengine/pi_physics.h>
#include "../pi_internal.h"

struct PI_SYSTEM_PHYSICS {
    Uint64 steptime;

    double* p_dt;
};

typedef struct PI_RIGIDBODY_DATA
{
    PI_RIGIDBODY_PROPERTIES rbproperties;
    PI_COLLISION_PROPERTIES clproperties;

    PI_MATRIX_4X4* p_transform;
    uint8_t* p_dirtyflag;
} PI_RIGIDBODY_DATA;

typedef struct PI_SYSTEM_SCENE
{
    PI_RIGIDBODY_DATA* rbdata;
    unsigned rbdata_count, rbdata_capacity;

    unsigned* rbfreelist;
    unsigned rbfreelist_last, rbfreelist_capacity,
             rbfreelist_first;

    PI_COLLISIONPROP_OBB* obbs;
    unsigned obb_count, obb_capacity;

    struct PI_RIGIDBODY_IMPL* rbhead;

    PI_PHYSICS_SCENE_PROPERTIES properties;
} PI_SYSTEM_SCENE;


struct PI_RIGIDBODY_IMPL
{
    struct PI_RIGIDBODY_IMPL* pnext;
    struct PI_RIGIDBODY_IMPL* pprev;
    PI_SYSTEM_SCENE* hscene;

    PI_RIGIDBODY_DATA* p_data;
};


PI_EXTERN_C void fn_physics_scene_deinit(void* hsystem, PI_HSYSTEMSCENE hscene);
PI_EXTERN_C void fn_physics_pre_update(void* hsyste, PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount);

PI_EXTERN_C bool _physics_init_scene(PI_SYSTEM_PHYSICS* hsystem, PI_HSYSTEMSCENE hscene);
PI_EXTERN_C PI_HRIGIDBODY _physics_add_rigid_body(PI_HSYSTEMSCENE hscene);
PI_EXTERN_C void _physics_update_collision(PI_HSYSTEMSCENE hscene, PI_HRIGIDBODY hrigidbody);

#endif
