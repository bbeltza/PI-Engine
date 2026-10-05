#include "physics_internal.h"

static void fn_physics_close(void* hsystem);

static size_t _init_physics_open(void) {
    return sizeof(PI_SYSTEM_PHYSICS);
}

static bool _init_physics_start(void* hsystem, PI_ENGINEDATA* hengine, PI_SYSTEM_DESC* pdesc)
{
    pdesc->structures.SCENE_size = sizeof(PI_SYSTEM_SCENE);
    pdesc->callbacks.pfn_close = fn_physics_close;
    pdesc->callbacks.pfn_scene_deinit = fn_physics_scene_deinit;
    pdesc->callbacks.pfn_preupdate = fn_physics_pre_update;
    return true;
}

const PI_INIT_SYSTEM _PI_INIT_PHYSICS = {
    _init_physics_open,
    _init_physics_start
};

/**/

void fn_physics_close(void* hsystem)
{
    (void)hsystem;
}
