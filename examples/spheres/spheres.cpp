#include "spheres.hpp"

#include <piengine/pi_math.hpp>

#include "common.cc"

PI_GAME::PI_GAME():
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this) },

    hsphere{ create_sphere_mesh(this) },
    hcube{ pi_create_mesh_indexed(this,
                PI_ARRAYSIZE(common::CUBE_VERTICES), common::CUBE_VERTICES,
                PI_ARRAYSIZE(common::CUBE_INDICES), common::CUBE_INDICES,
                common::CUBE_INDEX_TYPE
            ) },

    ground{ this, PI_VECTOR3X{ 0.0f, -2.0f, 0.0f }, PI_VECTOR3X{ 6.0f, 1.0f, 1.0f } }
{
    spheres.reserve(33);
    constexpr float gap = 0.1f;
    size_t num_spheres = spheres.capacity();
    for (size_t i = 0; i < num_spheres; i++) {
        spheres.emplace_back(this, PI_VECTOR3X{ (i-num_spheres*0.5f)*gap, 2.5f, 0.0f }, 0.1f);
    }

    pi::MATRIX_4X4 m = pi_mat4x4_lookat(PI_VECTOR3X{ 0.0f, 0.0f, -4.0f },
                                        PI_VECTOR3X{ 0.0f, 0.0f, 0.0f },
                                        PI_VECTOR3X{ 0.0f, 1.0f, 0.0f }
                                    );
    pi_set_scene_view(hscene, &m);

    PI_LIGHT l;
    l.point = PI_POINT_LIGHT{
        PI_VECTOR3X{ 1.0f, 0.9f, -2.0f },
        PI_VECTOR4{ 1.0f, 1.0f, 1.0f, 0.5f },
        10.0f,
        0.0f      
    };
    pi_add_light(hscene, PI_LIGHT_POINT, &l);
    l.directional = PI_DIRECTIONAL_LIGHT{
        PI_VECTOR3X{ 0.1f, 0.4f, -0.3f },
        PI_VECTOR4{ 1.0f, 1.0f, 1.0f, 0.1f }
    };
    pi_add_light(hscene, PI_LIGHT_DIRECTIONAL, &l);

    PI_LIGHT_PROPERTIES lightprops{
        { 0.3f, 0.3f, 0.4f, 1.0f },
            { 0.0f, 0.0f, -4.0f },
            22.0f,
            { 1.0f, 1.0f, 0.9f, 0.9f }
    };
    pi_set_light_properties(hscene, &lightprops);

    PI_PHYSICS_SCENE_PROPERTIES* psprops = pi_get_physics_scene_properties(hscene);
    psprops->forcedrag = 1.0f;
    psprops->gravity = 1.0f;
    
    pi_set_scene_clear_color(hscene, 0.05f, 0.05f, 0.1f, 1.0f);
    pi_enable_scene(hscene, 0);
}

bool PI_GAME::app_event(const PI_EVENT& event) noexcept
{
    switch (event.type)
    {
        case PI_EVENT_WINDOW_SIZE: {
            float ratio = (float)event.window.data1/event.window.data2;
            pi::MATRIX_4X4 m = pi_mat4x4_perspective( 70_degf, ratio, 0.01f, 5.0f );
            pi_set_scene_projection(hscene, &m);
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
    return true;
}

//

