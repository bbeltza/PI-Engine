#include <common/cube.h>

#include <piengine/pi_math.hpp>

#include "game.hpp"

static PI_HWORLDENV create_worldenv(PI_HGAME hgame) noexcept;
static PI_HSCENE create_scene(PI_HGAME hgame) noexcept;
static PI_HLIGHT create_light(PI_HSCENE hscene) noexcept;

PI_GAME::PI_GAME():
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this) },

    player{ this, { 0.0f, 2.0f, 0.0f } },

    hcubemesh{ pi_create_mesh_indexed(this,
            PI_ARRAYSIZE(common::CUBE_VERTICES), common::CUBE_VERTICES,
            PI_ARRAYSIZE(common::CUBE_INDICES), common::CUBE_INDICES,
            common::CUBE_INDEX_TYPE
        ) },

    ground{ this, { 0.5f, -2.0f, 1.0f }, { 10.0f, 1.0f, 10.0f }, hcubemesh },
    falling_cube{ this, { 0.0f, 0.0f, 2.0f }, {1.0f, 1.0f, 1.0f }, hcubemesh },

    hlight{ create_light(hscene) }
{
    if (!hscene) {
        return;
    }

    pi_enable_scene(hscene, 0);

    PI_LIGHT_PROPERTIES light_properties = {
        { 0.1f, 0.1f, 0.1f, 1.0f }
    };
    pi_set_light_properties(hscene, &light_properties);

    PI_RIGIDBODY_PROPERTIES* ground_properties = pi_get_rigid_body_properties(ground.hrigidbody);
    ground_properties->mass = 0.0f;

    PI_RIGIDBODY_PROPERTIES* cube_properties = pi_get_rigid_body_properties(falling_cube.hrigidbody);
    cube_properties->mass = 1.5f;
    cube_properties->force.x = 1.0f;
    PI_PHYSICS_SCENE_PROPERTIES* pisceneproperties = pi_get_physics_scene_properties(hscene);
    pisceneproperties->gravity = 2.0f;

    PI_HWND hwnd = pi_get_main_window(this);
    pi_lock_window_cursor(hwnd, true);
}

bool PI_GAME::app_event(PI_EVENT& event) noexcept
{
    switch (event.type)
    {
        case PI_EVENT_KEY:
            return _handle_key(event.key);
        case PI_EVENT_MOUSEMOVE:
            return _handle_motion(event.motion);
        case PI_EVENT_MOUSEWHEEL:
            return _handle_wheel(event.wheel);
        case PI_EVENT_WINDOW_SIZE:
            dirty_projection = true;
            wnd_w = event.window.data1;
            wnd_h = event.window.data2;
            break;
        default:
            break;
    }

    return true;
}


bool PI_GAME::app_update() noexcept
{
    float dt = pi_get_runtime_dtf(this);
    player.update(dt, player_direction);
    _update_dirty();
    return true;
}

//

bool PI_GAME::_handle_key(const PI_KEYEVENT& event) noexcept
{
    const auto key_vector = [](PI_PRESSMODE mode) -> float {
        float v = (mode == PI_PRESS_DOWN) ? 1.0f : (mode == PI_PRESS_UP) ? -1.0f : 0.0f;
        return v;
    };

    switch (event.code)
    {
        case PI_KEYCODE_A:
            player_direction.x -= key_vector(event.press);
            break;
        case PI_KEYCODE_D:
            player_direction.x += key_vector(event.press);
            break;
        case PI_KEYCODE_W:
            player_direction.y += key_vector(event.press);
            break;
        case PI_KEYCODE_S:
            player_direction.y -= key_vector(event.press);
            break;
        
        case PI_KEYCODE_SPACE:
            if (event.press == PI_PRESS_DOWN) {
                PI_RIGIDBODY_PROPERTIES* rbproperties = pi_get_rigid_body_properties(falling_cube.hrigidbody);
                rbproperties->force.y = 2.0f;
            }
            break;
        case PI_KEYCODE_ESCAPE:
            if (event.press == PI_PRESS_DOWN) {
                bool lock = pi_is_window_cursor_locked(event.hwindow);
                pi_lock_window_cursor(event.hwindow, !lock);
            }
            break;
        default:
            break;
    }

    return true;
}

bool PI_GAME::_handle_motion(const PI_MOTIONEVENT& event) noexcept
{
    constexpr float SENSIBILITY = 0.4_degf;

    if (pi_is_window_cursor_locked(event.hwindow)) {
        player.dirty_yawpitch = true;
        player.view_pitch += event.delta_y * SENSIBILITY,
        player.view_yaw += event.delta_x * SENSIBILITY;
    }
    return true;
}

bool PI_GAME::_handle_wheel(const PI_WHEELEVENT& event) noexcept
{

    return true;
}

void PI_GAME::_update_dirty() noexcept
{
    if (ground.instance_dirty) {
        ground.instance_dirty = false;
        pi_update_mesh_instance(ground.hinstance, &ground.instance);
    }
    if (falling_cube.instance_dirty) {
        falling_cube.instance_dirty = false;
        pi_update_mesh_instance(falling_cube.hinstance, &falling_cube.instance);
    }

    if (dirty_projection) {
        dirty_projection = false;

        float ratio = (float)wnd_w/wnd_h;
        PI_MATRIX_4X4 m = pi_mat4x4_perspective(70_degf, ratio, 0.1f, 22.0f);
        pi_set_scene_projection(hscene, &m);
    }
    if (player.dirty_view) {
        player.dirty_view = false;
        
        pi::MATRIX_4X4 m = pi_mat4x4_rotateY(player.view_yaw) * pi_mat4x4_rotateX(player.view_pitch);
        pi::MATRIX_4X4 v = pi_mat4x4_lookalong(player.get_view_position(), m->v.z, m->v.y);
        pi_set_scene_view(hscene, &v);
    }
}

//

static PI_HSCENE create_scene(PI_HGAME hgame) noexcept
{
    PI_HSCENE_RAII hscene{ pi_create_scene(hgame) };
    hscene = pi_init_physics_scene(hscene);
    hscene = pi_init_render_scene(hscene, hgame->get_worldenv());
    return hscene.release();
}

static PI_HWORLDENV create_worldenv(PI_HGAME hgame) noexcept
{
    PI_WORLDENV_RASTERIZER raster = {};
    raster.fillmode = PI_FILL_SOLID;
    raster.cullflags = PI_CULL_NONE;
    raster.depthclip = true;

    PI_WORLDENV_DEPTH depth = {};
    depth.depthwrite = true;
    depth.depthcompare = PI_COMPARE_LESS_EQUAL;

    PI::WORLDENV_DESC desc = {
        PI_WORLDEFFECT_LIGHTING,
        raster,
        depth
    };

    return pi_create_worldenvironment(hgame, &desc);
}

static PI_HLIGHT create_light(PI_HSCENE hscene) noexcept
{
    PI_LIGHT light;
    light.directional.direction = PI_VECTOR3X{ -0.6f, -1.0f, 0.7f };
    light.directional.color = PI_VECTOR4{ 0.8f, 0.76f, 1.0f, 1.0f };
    return pi_add_light(hscene, PI_LIGHT_DIRECTIONAL, &light);
}
