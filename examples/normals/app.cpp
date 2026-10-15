#include "app.hpp"

#include <common/cube.h>
#include <piengine/pi_math.hpp>

static PI_HWORLDENV create_worldenv(PI_HGAME hgame)
{
    PI_WORLDENV_RASTERIZER raster{};
    raster.cullflags = PI_CULL_BACK;
    raster.depthclip = true;

    PI_WORLDENV_DEPTH depth{};
    depth.depthwrite = true;
    depth.depthcompare = PI_COMPARE_LESS_EQUAL;

    PI_WORLDENV_MSAA msaa{};
    msaa.samples = 2;

    PI::WORLDENV_DESC desc{
        PI_WORLDEFFECT_LIGHTING | PI_WORLDEFFECT_SPECULAR,
        raster,
        depth,
        msaa
    };
    return pi_create_worldenvironment(hgame, &desc);
}

static PI_HSCENE create_scene(PI_HGAME hgame, PI_HWORLDENV hworldenv) {
    PI_HSCENE_RAII hscene{ pi_create_scene(hgame) };
    hscene = pi_init_render_scene(hscene, hworldenv);
    return hscene.release();
}

PI_GAME::PI_GAME() noexcept:
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this, hworldenv) },
    hmesh{ pi_create_mesh_indexed(this,
            PI_ARRAYSIZE(common::CUBE_VERTICES), common::CUBE_VERTICES,
            PI_ARRAYSIZE(common::CUBE_INDICES), common::CUBE_INDICES,
            common::CUBE_INDEX_TYPE
            ) },
    hinstance{ pi_add_scene_mesh(hscene, hmesh, NULL ) }
{
    PI_MATRIX_4X4 m = pi_identity_4x4();
    m.wz = 2.0f;
    pi_set_scene_view(hscene, &m);

    PI_LIGHT light;
    light.directional = {
        { -1.0f, -2.0f, 0.3f }, // Directions will be normalized by the engine
        { 0.9f, 0.9f, 1.0f, 1.0f }
    };
    pi_add_light(hscene, PI_LIGHT_DIRECTIONAL, &light);
    
    light.point = {
        { -2.0f, -3.0f, 0.3f },
        { 1.0f, 0.8f, 0.9f, 1.0f },
        6.0f,
        4.0f
    };
    pi_add_light(hscene, PI_LIGHT_POINT, &light);


    PI_LIGHT_PROPERTIES light_properties = {
        { 0.1f, 0.1f, 0.1f },
        { 0.0f, 0.0f, -2.0f },
        { 22 },
        { 1.0f, 1.0f, 1.0f, 1.0f }
    };
    pi_set_light_properties(hscene, &light_properties);

    pi_enable_scene(hscene, 0);
}

PI_GAME::~PI_GAME() noexcept
{}

bool PI_GAME::app_event(const PI_EVENT& event) noexcept
{
    switch (event.type) {
        case PI_EVENT_MOUSEMOVE: if (event.motion.state & PI_MSTATE_LEFT) {
            static constexpr float SENSIBILITY = 0.01f;
            dirty.cube = true;
            cubeyaw -= event.motion.delta_x * SENSIBILITY;
            cubepitch -= event.motion.delta_y * SENSIBILITY;
        } break;
        case PI_EVENT_WINDOW_SIZE: {
            float ratio = static_cast<float>(event.window.data1)/event.window.data2;
            PI_MATRIX_4X4 m = pi_matrix_perspective(70_degf, ratio, 0.01f, 10.0f);
            pi_set_scene_projection(hscene, &m);
        } break;
        default:
            break;
    }
    return true;
}

bool PI_GAME::app_update() noexcept
{
    float dt = pi_get_runtime_dtf(this);
    if (dirty.cube) {
        float t = dt * 10.0f;
        t = PI_MIN(t, 1.0f);
        PI_LERPA(realcubepitch, cubepitch, t);
        PI_LERPA(realcubeyaw, cubeyaw, t);

        if (realcubepitch == cubepitch && realcubeyaw == cubeyaw)
            dirty.cube = false;

        PI_INSTANCE i;
        pi::MATRIX_4X4 m{pi_matrix_rotate_x, realcubepitch};
        pi_matrix_rotate_y(&i.transform, realcubeyaw);
        i.transform = m * i.transform;
        i.transform.wz = 0.0f;

        pi_update_mesh_instance(hinstance, &i);
    }

    return true;
}
