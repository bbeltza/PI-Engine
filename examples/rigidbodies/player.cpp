#include "game.hpp"

#include <piengine/pi_math.hpp>

GAME_PLAYER::GAME_PLAYER(PI_HGAME hgame, const PI_VECTOR3& position) noexcept:
    hrigidbody{ pi_add_rigid_body( hgame->get_scene() ) },

    dirty_yawpitch{ true },
    dirty_view{ true }
{
    PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
    properties->position = PI_VECTOR3X{ position.x, position.y, position.z };
    properties->mass = 1.0f;

    pi_set_collision(hrigidbody, PI_COLLISION_AABB);
    PI_COLLISION_PROPERTIES* cproperties = pi_get_collision(hrigidbody);
    cproperties->aabb.min = PI_VECTOR3X{ -0.25f, 0.0f, -0.25f };
    cproperties->aabb.max = PI_VECTOR3X{  0.25f, 1.5f,  0.25f };
}

void GAME_PLAYER::update(float dt, const PI_VECTOR2& direction) noexcept
{
    constexpr float PITCH_CLAMP = 89.0_degf;

    dirty_view = true;
    if (dirty_yawpitch) {
        if (view_pitch > PITCH_CLAMP) {
            view_pitch = PITCH_CLAMP;
        }
        else if (view_pitch < -PITCH_CLAMP) {
            view_pitch = -PITCH_CLAMP;
        }

        dirty_yawpitch = false;
        dirty_view = true;
        yaw = view_yaw;
        pi::MATRIX_4X4 m;
        m(pi_matrix_rotate_y, yaw);
        forward = m->v.z;
    } 

    if (direction.x || direction.y) {
        dirty_view = true;
        m_bobbytime += dt;
        
        PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
        PI_VECTOR3X delta = { direction.x, 0.0f, direction.y };
        pi_vector_normalize4(&delta, delta);
        delta *= dt*3.0f;

        pi::VECTOR4 up{ 0.0f, 1.0f, 0.0f, 0.0f };
        pi::VECTOR4 right = pi_vector_cross4(up, forward);
        pi::MATRIX_4X4 m = { 
            right,
            up,
            forward
        };
        delta = pi_mulv_4x4(delta, &m);
        properties->position = pi_add_4(properties->position, delta);        
    }
    else if (m_bobbytime) {
        dirty_view = true;
        m_bobbytime = 0.0f;
    }
}

PI_VECTOR3X GAME_PLAYER::get_view_position() noexcept
{
    constexpr float bobby_speed = 13.0f;
    constexpr float bobby_force = 0.1f;
    constexpr float bobby_fac_force = 0.66f;
    float up = 1.0f;
    if (m_bobbytime) {
        float n = -sinf(m_bobbytime * bobby_speed) * bobby_force;
        n *= cosf(up) * bobby_fac_force;
        up += n;
    }

    PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
    return pi_add_4(properties->position, PI_VECTOR4{ 0.0f, up, 0.0f, 0.0f });
}
