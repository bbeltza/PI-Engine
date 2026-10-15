#include "physics_internal.h"

void fn_physics_scene_deinit(void* hsystem, PI_HSYSTEMSCENE hscene)
{
    /* Cleanup all rigid bodies */
    while (hscene->rbhead) {
        struct PI_RIGIDBODY_IMPL* next = hscene->rbhead->pnext;
        PI_FREE(hscene->rbhead);
        hscene->rbhead = next;
    }

    PI_FREE(hscene->rbdata);
    PI_FREE(hscene->rbfreelist);
#ifndef NDEBUG
    memset(hscene, 0xCC, sizeof(PI_SYSTEM_SCENE));
#endif
}

bool _physics_init_scene(PI_SYSTEM_PHYSICS* hsystem, PI_HSYSTEMSCENE hscene)
{
    hscene->rbdata = NULL;
    hscene->rbdata_count = 0;
    hscene->rbdata_capacity = 0;
    hscene->rbfreelist = NULL;
    hscene->rbfreelist_last = 0;
    hscene->rbfreelist_capacity = 0;
    hscene->rbfreelist_first = 0;

    hscene->obbs = NULL;
    hscene->obb_count = 0;
    hscene->obb_capacity = 0;

    hscene->rbhead = NULL;

    hscene->properties.gravity = 0.0f;
    hscene->properties.forcedrag = 0.5f;
    return true;
}

PI_PHYSICS_SCENE_PROPERTIES* pi_get_physics_scene_properties(PI_HSCENE hscene) {
    if (!hscene) {
        return NULL;
    }
    if (!hscene->initsubsystems.physics) {
        return NULL;
    }

    return &hscene->subsystems.physics->properties;
}
