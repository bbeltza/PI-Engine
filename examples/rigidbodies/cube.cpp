#include "game.hpp"
#include <piengine/pi_math.h>

static constexpr PI_VECTOR4 vec3_to_4(const PI_VECTOR3& v, float w = 0.0f) {
    return PI_VECTOR4{ v.x, v.y, v.z, w };
}

PI_HINSTANCE create_hinstance(PI_HSCENE hscene, GAME_CUBE* cube)
{
    return pi_add_scene_mesh(hscene, cube->hmesh, &cube->instance);
}

GAME_CUBE::GAME_CUBE(PI_HGAME hgame, const PI_VECTOR3& position, const PI_VECTOR3& scale, PI_HMESH hmesh):
    instance{
        pi_scale_4x4(scale.x, scale.y, scale.z, 1.0f),
        { 1.0f, 1.0f, 1.0f, 1.0f }
    },

    hmesh{ hmesh },
    hinstance{ create_hinstance( hgame->get_scene(), this ) },
    hrigidbody{ pi_add_rigid_body( hgame->get_scene() ) }
{
    pi_attach_transform(hrigidbody, &instance.transform, &instance_dirty);

    PI_RIGIDBODY_PROPERTIES* rbproperties = pi_get_rigid_body_properties(hrigidbody);
    rbproperties->position = vec3_to_4(position);
    rbproperties->mass = 1.0f;

    PI_VECTOR3X halfscale = pi_mul_4s(vec3_to_4(scale), 0.5f);
    pi_set_collision(hrigidbody, PI_COLLISION_AABB);
    PI_COLLISION_PROPERTIES* clproperties = pi_get_collision(hrigidbody);
    pi_vector_negative4(&clproperties->aabb.min, halfscale);
    clproperties->aabb.max = halfscale;
}
