#ifndef PI_PHYSICS_INTERNAL_H
#define PI_PHYSICS_INTERNAL_H

#include "../pi_internal.h"

struct PI_SYSTEM_PHYSICS {
    void* happy;
};

typedef struct PI_SYSTEM_SCENE
{
    void* happy;
} PI_SYSTEM_SCENE;


PI_EXTERN_C void fn_physics_scene_deinit(void* hsystem, PI_HSYSTEMSCENE hscene);
PI_EXTERN_C void fn_physics_pre_update(void* hsyste, PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount);

#endif
