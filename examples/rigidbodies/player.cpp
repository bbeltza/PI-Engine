#include "game.hpp"

#include <piengine/pi_math.h>

GAME_PLAYER::GAME_PLAYER(PI_HGAME hgame, const PI_VECTOR3& position) noexcept:
    hrigidbody{ pi_add_rigid_body( hgame->get_scene() ) },

    dirty_yawpitch{ true },
    dirty_view{ true }
{
    PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
    properties->position = { position.x, position.y, position.z };
    properties->mass = 1.0f;

    pi_set_collision(hrigidbody, PI_COLLISION_AABB);
    PI_COLLISION_PROPERTIES* cproperties = pi_get_collision(hrigidbody);
    cproperties->aabb.min = { -0.25f, 0.0f, -0.25f };
    cproperties->aabb.max = { 0.25f, 1.5f, 0.25f };
}

#include <cstdio>
void GAME_PLAYER::update(float dt, const PI_VECTOR2& direction) noexcept
{
    constexpr float PITCH_CLAMP = 89.0_degf;

    dirty_view = true;
    if (dirty_yawpitch) {
        if (pitch > PITCH_CLAMP) {
            pitch = PITCH_CLAMP;
        }
        else if (pitch < -PITCH_CLAMP) {
            pitch = -PITCH_CLAMP;
        }

        dirty_yawpitch = false;
        dirty_view = true;
        PI_MATRIX_4X4 m, m1;
        pi_matrix_rotate_y(&m, this->yaw);
        forward = m.v.z;
        right = m.v.x;

        pi_matrix_rotate_x(&m1, this->pitch);
        pi_mul_4x4(&m, &m, &m1);
        look = m.v.z;
    } 

    if (direction.x || direction.y) {
        dirty_view = true;
        m_bobbytime += dt;
        
        PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
        PI_VECTOR3X delta = { direction.x, 0.0f, direction.y };
        pi_vector_normalize4(&delta, delta);
        delta = pi_mul_4s(delta, dt * 3.0f);

        PI_MATRIX_4X4 m{};
        m.v.x = right;
        m.v.y.y = 1.0f;
        m.v.z = forward;
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
    return pi_add_4(properties->position, { 0.0f, up, 0.0f, 0.0f });
}
