#include <common/cube.h>

#include <piengine/pi_math.h>

#include "game.hpp"

static PI_HWORLDENV create_worldenv(PI_HGAME hgame) noexcept;
static PI_HSCENE create_scene(PI_HGAME hgame) noexcept;

PI_GAME::PI_GAME():
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this) },

    hcubemesh{ pi_create_mesh_indexed(this,
            PI_ARRAYSIZE(common::CUBE_VERTICES), common::CUBE_VERTICES,
            PI_ARRAYSIZE(common::CUBE_INDICES), common::CUBE_INDICES,
            common::CUBE_INDEX_TYPE
        ) },

    ground{ this, { 0.5f, -2.0f, 1.0f }, { 4.0f, 1.0f, 5.0f }, hcubemesh },
    falling_cube{ this, { 0.0f, 0.0f, 2.0f }, {1.0f, 1.0f, 1.0f }, hcubemesh },

    dirty_look{ true },
    dirty_view{ true }
{
    if (!hscene) {
        return;
    }

    pi_enable_scene(hscene, 0);

    PI_RIGIDBODY_PROPERTIES* ground_properties = pi_get_rigid_body_properties(ground.hrigidbody);
    ground_properties->mass = 0.0f;
    PI_RIGIDBODY_PROPERTIES* cube_properties = pi_get_rigid_body_properties(falling_cube.hrigidbody);
    cube_properties->mass = 1.5f;
    cube_properties->force.x = 1.0f;
    PI_PHYSICS_SCENE_PROPERTIES* pisceneproperties = pi_get_physics_scene_properties(hscene);
    pisceneproperties->gravity = 2.0f;
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
    _update_dirty();
    return true;
}

//

bool PI_GAME::_handle_key(const PI_KEYEVENT& event) noexcept
{
    const auto key_vector = [](PI_PRESSMODE mode) -> float {
        float v = (mode == PI_PRESS_DOWN) ? 1.0f : (mode == PI_PRESS_UP) ? -1.0f : 0.0f;
        return v * 1.3f;
    };

    PI_RIGIDBODY_PROPERTIES* rbproperties = pi_get_rigid_body_properties(falling_cube.hrigidbody);
    PI_ASSERT(rbproperties != NULL);
    switch (event.code)
    {
        case PI_KEYCODE_A:
            rbproperties->velocity.x -= key_vector(event.press);
            break;
        case PI_KEYCODE_D:
            rbproperties->velocity.x += key_vector(event.press);
            break;
        case PI_KEYCODE_W:
            rbproperties->velocity.z += key_vector(event.press);
            break;
        case PI_KEYCODE_S:
            rbproperties->velocity.z -= key_vector(event.press);
            break;
        case PI_KEYCODE_E:
            rbproperties->velocity.y += key_vector(event.press);
            break;
        case PI_KEYCODE_Q:
            rbproperties->velocity.y -= key_vector(event.press);
            break;
        
        case PI_KEYCODE_SPACE:
            if (event.press == PI_PRESS_DOWN) {
                rbproperties->force.y = 2.0f;
            }
            break;
        default:
            break;
    }

    return true;
}

bool PI_GAME::_handle_motion(const PI_MOTIONEVENT& event) noexcept
{
    constexpr float SENSIBILITY = 0.01f;

    if (event.state & PI_MSTATE_RIGHT) {
        dirty_look = true;
        camera_look.x += event.delta_y * SENSIBILITY,
        camera_look.y += event.delta_x * SENSIBILITY;
    }
    return true;
}

bool PI_GAME::_handle_wheel(const PI_WHEELEVENT& event) noexcept
{
    dirty_view = true;

    PI_VECTOR4 delta = { event.amnt_x, 0.0f, event.amnt_y };    
    delta = pi_mulv_4x4(delta, &camera_dir);

    camera_eye = pi_add_4(camera_eye, delta);
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

    if (dirty_look) {
        dirty_look = false;
        dirty_view = true;

        PI_MATRIX_4X4 m0, m1;
        pi_matrix_rotate_y(&m0, camera_look.y);
        pi_matrix_rotate_x(&m1, camera_look.x);
        pi_mul_4x4(&camera_dir, &m0, &m1);
    }

    if (dirty_projection) {
        dirty_projection = false;

        float ratio = (float)wnd_w/wnd_h;
        PI_MATRIX_4X4 m = pi_matrix_perspective(70_degf, ratio, 0.1f, 22.0f);
        pi_set_scene_projection(hscene, &m);
    }
    if (dirty_view) {
        dirty_view = false;
        
        PI_MATRIX_4X4 m;
        pi_look_along(&m, camera_eye,
                    *(PI_VECTOR4*)camera_dir.m[2],
                    *(PI_VECTOR4*)camera_dir.m[1]
                );
        pi_set_scene_view(hscene, &m);
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
        PI_WORLDEFFECT_NONE,
        raster,
        depth
    };

    return pi_create_worldenvironment(hgame, &desc);
}
