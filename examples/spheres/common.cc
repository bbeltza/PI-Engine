#include "spheres.hpp"

#include <common/cube.h>
#include <common/sphere.h>

static PI_HWORLDENV create_worldenv(PI_HGAME hgame) noexcept
{
    PI_WORLDENV_RASTERIZER raster{
        PI_FILL_SOLID,
        PI_CULL_BACK,
        true
    };
    PI_WORLDENV_DEPTH depth{
        true,
        PI_COMPARE_LESS
    };

    PI_WORLDENV_DESC desc{};
    desc.effects = PI_WORLDEFFECT_LIGHTING | PI_WORLDEFFECT_SPECULAR;
    desc.rasterizer = &raster;
    desc.depth = &depth;

    return pi_create_worldenvironment(hgame, &desc);    
}
static PI_HSCENE create_scene(PI_HGAME hgame) noexcept
{
    PI_ASSERT(hgame->hworldenv);

    PI_HSCENE_RAII hscene{ pi_create_scene(hgame) };
    hscene = pi_init_render_scene(hscene, hgame->hworldenv);
    hscene = pi_init_physics_scene(hscene);
    return hscene.release();
}


static const int h_vertices = 32;
static const int v_vertices = 32;
static const bool flatshaded = false;
static const int num_vertices = static_cast<int>(
             common::calc_num_sphere_vertices(h_vertices, v_vertices, flatshaded)
         );
static const int num_indices = static_cast<int>(
             common::calc_num_sphere_indices(h_vertices, v_vertices)
         );
static PI_VERTEX vertices[ num_vertices ];
static int indices[ num_indices ];

static PI_HMESH create_sphere_mesh(PI_HGAME hgame) noexcept
{

    common::generate_sphere_mesh(1, h_vertices, v_vertices, vertices, indices, flatshaded);
    return pi_create_mesh_indexed(hgame, num_vertices, vertices, num_indices, indices, common::SPHERE_INDEX_TYPE);
}



SPHERE::SPHERE(PI_HGAME hgame, const PI_VECTOR3X& start_position, float radius) noexcept:
    hrigidbody{ pi_add_rigid_body(hgame->hscene) },
    hinstance{ pi_add_scene_mesh(hgame->hscene, hgame->hsphere, NULL) }
{
    PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
    properties->position = start_position;
    properties->scale = PI_VECTOR4{ radius, radius, radius, 1.0f };
    properties->mass = 1.0f;

    pi_set_collision(hrigidbody, PI_COLLISION_SPHERE);
    PI_COLLISION_PROPERTIES* clproperties = pi_get_collision(hrigidbody);
    clproperties->sphere.radius = radius;

    pi_attach_transform(hrigidbody, &m_instance.transform, &m_dirtyflag);
}

void SPHERE::update() noexcept
{
    if (m_dirtyflag) {
        m_dirtyflag = false;
        pi_update_mesh_instance(hinstance, &m_instance);
    }
}

static PI_INSTANCE* acquire_instance_ptr(const PI_VECTOR3X& position, const PI_VECTOR3X& scale, const PI_VECTOR4& color) noexcept
{
    static thread_local PI_INSTANCE s_threadlocal_instance;

    s_threadlocal_instance.transform = PI_MATRIX_4X4{
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    s_threadlocal_instance.col = color;
    return &s_threadlocal_instance;
}

CUBE::CUBE(PI_HGAME hgame, const PI_VECTOR3X& position, const PI_VECTOR3X& scale, const PI_VECTOR4& color) noexcept:
    m_hrigidbody{ pi_add_rigid_body(hgame->hscene) },
    m_hinstance{ pi_add_scene_mesh(hgame->hscene, hgame->hcube, acquire_instance_ptr(position, scale, color)) }
{
    pi_set_collision(m_hrigidbody, PI_COLLISION_AABB);

    PI_RIGIDBODY_PROPERTIES* rbproperties = pi_get_rigid_body_properties(m_hrigidbody);
    rbproperties->mass = 0.0f;
    rbproperties->position = position;
    rbproperties->scale = scale;
}
