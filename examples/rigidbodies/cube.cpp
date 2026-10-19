#include "game.hpp"
#include <piengine/pi_math.hpp>

PI_HINSTANCE create_hinstance(PI_HSCENE hscene, GAME_CUBE* cube)
{
    return pi_add_scene_mesh(hscene, cube->hmesh, &cube->instance);
}

GAME_CUBE::GAME_CUBE(PI_HGAME hgame, const PI_VECTOR3& position, const PI_VECTOR3& scale, PI_HMESH hmesh):
    hmesh{ hmesh },
    hinstance{ create_hinstance( hgame->get_scene(), this ) },
    hrigidbody{ pi_add_rigid_body( hgame->get_scene() ) }
{
    pi_attach_transform(hrigidbody, &instance.transform, &instance_dirty);
    PI_RIGIDBODY_PROPERTIES* rbproperties = pi_get_rigid_body_properties(hrigidbody);
    rbproperties->position = pi::VECTOR4{position};
    rbproperties->scale = pi::VECTOR4{scale};
    rbproperties->mass = 1.0f;

    pi_set_collision(hrigidbody, PI_COLLISION_AABB);
    PI_COLLISION_PROPERTIES* clproperties = pi_get_collision(hrigidbody);
    clproperties->aabb.min = PI_VECTOR3X{ -0.5f, -0.5f, -0.5f };
    clproperties->aabb.max = PI_VECTOR3X{ 0.5f, 0.5f, 0.5f };
}
