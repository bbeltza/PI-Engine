#include "spheres.hpp"
#include "../spheres/common.cc"

#include <piengine/pi_math.hpp>

constexpr PI_VECTOR4 ground_color{ 0.4f, 0.4f, 0.5f, 0.8f };

PI_GAME::PI_GAME() noexcept:
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this) },

    hsphere{ create_sphere_mesh(this) },
    hcube{ pi_create_mesh_indexed(this,
                PI_ARRAYSIZE(common::CUBE_VERTICES), common::CUBE_VERTICES,
                PI_ARRAYSIZE(common::CUBE_INDICES), common::CUBE_INDICES,
                common::CUBE_INDEX_TYPE
            ) },

    grounds{
        CUBE{ this, PI_VECTOR3X{ 0.0f, -2.0f, 0.0f }, PI_VECTOR3X{ 5.0f, 1.0f, 5.0f }, ground_color },

        CUBE{ this, PI_VECTOR3X{ 0.0f, -1.0f, 1.25f }, PI_VECTOR3X{ 3.0f, 1.0f, 0.5f }, ground_color },
        CUBE{ this, PI_VECTOR3X{ 0.0f, -1.0f, -1.25f }, PI_VECTOR3X{ 3.0f, 1.0f, 0.5f }, ground_color },

        CUBE{ this, PI_VECTOR3X{ 1.25f, -1.0f, 0.0f }, PI_VECTOR3X{ 0.5f, 1.0f, 3.0f }, ground_color },
        CUBE{ this, PI_VECTOR3X{ -1.25f, -1.0f, 0.0f }, PI_VECTOR3X{ 0.5f, 1.0f, 3.0f }, ground_color },
    }
{
    size_t spherew = 15;
    size_t sphereh = 15;
    float interval = 0.05f;
    for (size_t i = 0; i < sphereh; i++) {
        float y = (i - sphereh*0.5f + 0.5f)*interval;
        for (size_t j = 0; j < spherew; j++) {
            float x = (j - spherew*0.5f + 0.5f)*interval;
            spheres.emplace_back(this, PI_VECTOR3X{ x, 3.0f, y }, 0.1f);
        }    
    }

    PI_PHYSICS_SCENE_PROPERTIES* psprops = pi_get_physics_scene_properties(hscene);
    psprops->gravity = 2.0f;
    psprops->forcedrag = 1.0f;

    PI_LIGHT light{};
    light.point.position = PI_VECTOR3X{ 2.0f, 3.0f, 1.0f };
    light.point.color = PI_VECTOR4{ 1.0f, 1.0f, 0.97f, 1.0f };
    light.point.range = 10.0f;
    light.point.attenuation = 10.0f;
    pi_add_light(hscene, PI_LIGHT_POINT, &light);

    pi_set_scene_clear_color(hscene, 0.1f, 0.1f, 0.12f, 1.0f);
    pi_enable_scene(hscene, -1);
}

bool PI_GAME::app_event(const PI_EVENT& event) noexcept
{
    PI_ASSERT(!event.window.hwindow || event.window.hwindow == pi_get_main_window(this));
    switch (event.type)
    {
        case PI_EVENT_WINDOW_SIZE: {
            float ratio = static_cast<float>(event.window.data1)/event.window.data2;
            pi::MATRIX_4X4 m = pi_mat4x4_perspective(70_degf, ratio, 0.01f, 10.0f);
            pi_set_scene_projection(hscene, &m);
        } break;
        case PI_EVENT_KEY:
            switch (event.key.code) {
                case PI_KEYCODE_W: keys.w = event.key.press != PI_PRESS_UP; break;
                case PI_KEYCODE_A: keys.a = event.key.press != PI_PRESS_UP; break;
                case PI_KEYCODE_S: keys.s = event.key.press != PI_PRESS_UP; break;
                case PI_KEYCODE_D: keys.d = event.key.press != PI_PRESS_UP; break;
                case PI_KEYCODE_Q: keys.q = event.key.press != PI_PRESS_UP; break;
                case PI_KEYCODE_E: keys.e = event.key.press != PI_PRESS_UP; break;
                
                case PI_KEYCODE_ESCAPE: if (event.key.press == PI_PRESS_DOWN) {
                    bool lock = pi_is_window_cursor_locked(event.key.hwindow);
                    pi_lock_window_cursor(event.key.hwindow, !lock);
                } break;
                default: break;
            } break;
        case PI_EVENT_MOUSEMOVE: if (pi_is_window_cursor_locked(event.key.hwindow)) {
            constexpr float SENS = 0.009f;
            camera.rotate(
                    event.motion.delta_x * SENS,
                    event.motion.delta_y * SENS
                );                     
        } break;
        default:
            break;
    }
    return true;
}
bool PI_GAME::app_update() noexcept
{
    for (auto& sphere : spheres) {
        sphere.update();
    }
    PI_VECTOR3 dir{
        static_cast<float>(keys.d - keys.a),
        static_cast<float>(keys.e - keys.q),
        static_cast<float>(keys.w - keys.s)
    };
    if (camera.update(this, dir)) {
        PI_LIGHT_PROPERTIES lighting{
            PI_VECTOR3X{ 0.22f, 0.22f, 0.25f },
            camera.get_position(),
            22.2f,
            PI_VECTOR4{ 1.0f, 1.0f, 0.9f, 1.0f }
        };
        pi_set_light_properties(hscene, &lighting);
    }
    return true;
}

/*  */

bool CAMERA::update(PI_HGAME hgame, const PI_VECTOR3& direction) noexcept
{
    float dt = pi_get_runtime_dtf(hgame);
    //PI_DEBUG_FMT("%f %f", dt, 1/dt);

    pi::VECTOR4 dir{ direction };
    if (m_dirtyrot) {
        m_dirtyrot = false;

        pi::MATRIX_4X4 m = pi_mat4x4_rotateY(m_yaw) * pi_mat4x4_rotateX(m_pitch);
        m_right = m->v.x;
        m_up    = m->v.y;
        m_look  = m->v.z;
    }
    else if (!dir) {
        return false;
    }

    if (dir) {
        dir = pi_vec4_normalize(dir) * dt * 2.0f;
        pi::MATRIX_4X4 m{
            m_right,
            m_up,
            m_look
        };
        dir *= m;
        m_position += dir.ref();
    }

    pi::MATRIX_4X4 m = pi_mat4x4_lookalong(m_position, m_look, m_up);
    pi_set_scene_view(hgame->hscene, &m);

    if (m_init) {
        m_init = false;
        return true;
    }
    return !dir.zero();
}

void CAMERA::rotate(float dtyaw, float dtpitch) noexcept
{
    if (!dtyaw && !dtpitch)
        return;

    m_yaw += dtyaw;
    m_pitch += dtpitch;
    m_dirtyrot = true;
}
