#include "physics_internal.h"

static void fn_physics_close(void* hsystem);

static size_t _init_physics_open(void) {
    return sizeof(PI_SYSTEM_PHYSICS);
}

static bool _init_physics_start(void* hsystem, PI_ENGINEDATA* hengine, PI_SYSTEM_DESC* pdesc)
{
    PI_SYSTEM_PHYSICS* system = (PI_SYSTEM_PHYSICS*)hsystem;
    system->steptime = 0;
    system->p_dt = &hengine->dt_double;

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

/**/

PI_HSCENE pi_init_physics_scene(PI_HSCENE hscene)
{
    if (!hscene) {
        return NULL;
    }

    if (hscene->initsubsystems.physics) {
        return NULL;
    }

    if (!_physics_init_scene(hscene->engine->subsystems.physics, hscene->subsystems.physics)) {
        return NULL;
    }
    hscene->initsubsystems.physics = true;
    return hscene;
}

PI_HRIGIDBODY pi_add_rigid_body(PI_HSCENE hscene)
{
    if (!hscene) {
        return NULL;
    }
    if (!hscene->initsubsystems.physics) {
        return NULL;
    }

    return _physics_add_rigid_body(hscene->subsystems.physics);
}
