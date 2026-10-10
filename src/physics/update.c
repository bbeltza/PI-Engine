#include "physics_internal.h"

#include <piengine/pi_math.h>

static void update_scene(PI_SYSTEM_PHYSICS* hsystem, PI_HSYSTEMSCENE hsysscene);
static void update_freelists(PI_HSYSTEMSCENE hscene);

PI_EXTERN_C void fn_physics_pre_update(void* hsystem, PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount)
{
    PI_SYSTEM_PHYSICS* system = (PI_SYSTEM_PHYSICS*)hsystem;
    for (unsigned i = 0; i < scenecount; i++) {
        PI_HSYSTEMSCENE hsysscene = hsysscenes[i];
        update_scene(system, hsysscene);
    }
}

void update_scene(PI_SYSTEM_PHYSICS* hsystem, PI_HSYSTEMSCENE hscene)
{
    update_freelists(hscene);

    float dt = (float)*hsystem->p_dt;
    PI_HRIGIDBODY curhrigidbody = hscene->rbhead;
    while (curhrigidbody) {
        PI_RIGIDBODY_DATA* rbdata = curhrigidbody->p_data;
        if (rbdata->rbproperties.mass == 0.0f) {
            // Rigid bodies with a mass = 0.0f are totally static
                // They don't get any collision checks, nor velocity impulses,
                // but other rigid bodies with a mass can still collide with them.

            if (rbdata->p_transform) {
                rbdata->rbproperties.position.w = 1.0f;
                rbdata->p_transform->v.w = rbdata->rbproperties.position;
            }
            goto END_LOOP;
        }

        PI_VECTOR3X oldpos = rbdata->rbproperties.position;

        PI_VECTOR3X pdelta = rbdata->rbproperties.velocity;
        if (hscene->properties.gravity || hscene->properties.forcedrag) {
            float fac = dt * rbdata->rbproperties.mass;
            PI_VECTOR3X fdt = {
                -rbdata->rbproperties.force.x * hscene->properties.forcedrag,
                -0.01f * hscene->properties.gravity,
                -rbdata->rbproperties.force.z * hscene->properties.forcedrag
            };
            fdt = pi_mul_4s(fdt, fac);
            rbdata->rbproperties.force = pi_add_4(
                        rbdata->rbproperties.force,
                        fdt
                    );

            pdelta = pi_add_4(pdelta, rbdata->rbproperties.force);
        }


        rbdata->rbproperties.position = pi_add_4(
                rbdata->rbproperties.position,
                pdelta
            );

        _physics_update_collision(hscene, curhrigidbody);

        if (rbdata->p_dirtyflag &&
                (memcmp(&rbdata->rbproperties.position,
                        &oldpos,
                        sizeof(PI_VECTOR3)) != 0)
        ) {
            *rbdata->p_dirtyflag |= 1;
        }
        if (rbdata->p_transform) {
            rbdata->rbproperties.position.w = 1.0f;
            rbdata->p_transform->v.w = rbdata->rbproperties.position;
        }

    END_LOOP:
        curhrigidbody = curhrigidbody->pnext;
    }
}

void update_freelists(PI_HSYSTEMSCENE hscene)
{
    if (!hscene->rbfreelist_last)
        return;

    unsigned firstindex = hscene->rbfreelist[hscene->rbfreelist_last - 1];
    for (unsigned i = hscene->rbfreelist_first; i < hscene->rbfreelist_last - 1; i++) {
        unsigned rbi = hscene->rbfreelist[i];
        if (rbi < firstindex)
            firstindex = rbi;

        if (!firstindex)
            break;
    }

    struct PI_RIGIDBODY_IMPL* curhrigidbody = hscene->rbhead;
    PI_RIGIDBODY_DATA* p_rbdata = hscene->rbdata + firstindex;
    while (curhrigidbody) {
        unsigned rbi = (unsigned)(curhrigidbody->p_data - hscene->rbdata);
        if (rbi < firstindex)
            goto END_LOOP;

        *p_rbdata = *curhrigidbody->p_data;
        curhrigidbody->p_data = p_rbdata;
        p_rbdata++;
    END_LOOP:
        curhrigidbody = curhrigidbody->pnext;
    }

    hscene->rbfreelist_first = 0;
    hscene->rbfreelist_last = 0;
}
