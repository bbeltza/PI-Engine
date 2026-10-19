#include "spheres.hpp"

#include <piengine/pi_math.hpp>

static PI_HWORLDENV create_worldenv(PI_HGAME hgame) noexcept;
static PI_HSCENE create_scene(PI_HGAME hgame) noexcept;
static PI_HMESH create_sphere_mesh(PI_HGAME hgame) noexcept;

PI_GAME::PI_GAME():
    hworldenv{ create_worldenv(this) },
    hscene{ create_scene(this) },
    hsphere{ create_sphere_mesh(this) },

    sphere1{ this, { -1.0, 0.0f, 0.0f, 0.0f }, 1.0f},
    sphere2{ this, {  1.0, 0.0f, 0.0f, 0.0f }, 0.6f}
{

    pi::MATRIX_4X4 m;
    m(pi_look_at, PI_VECTOR3X{ 0.0f, 0.0f, -4.0f },
                  PI_VECTOR3X{ 0.0f, 0.0f, 0.0f },
                  PI_VECTOR3X{ 0.0f, 1.0f, 0.0f });
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
        { 0.03f, 0.03f, 0.04f, 1.0f },
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
            pi::MATRIX_4X4 m = pi_matrix_perspective( 70_degf, ratio, 0.01f, 5.0f );
            pi_set_scene_projection(hscene, &m);
        } break;
        case PI_EVENT_KEY: {
            switch (event.key.code) {
                case PI_KEYCODE_W: keys.w = event.key.press; break;
                case PI_KEYCODE_A: keys.a = event.key.press; break;
                case PI_KEYCODE_S: keys.s = event.key.press; break;
                case PI_KEYCODE_D: keys.d = event.key.press; break;

                case PI_KEYCODE_UP: keys.up = event.key.press; break;
                case PI_KEYCODE_LEFT: keys.left = event.key.press; break;
                case PI_KEYCODE_DOWN: keys.down = event.key.press; break;
                case PI_KEYCODE_RIGHT: keys.right = event.key.press; break;

                default: break;
            }
        } break;
        default:
            break;
    }

    return true;
}
bool PI_GAME::app_update() noexcept
{
    sphere1.set_dirty_flag();
    float dt = pi_get_runtime_dtf(this);
    float SPEED = 3.0f;
    if ((keys.d ^ keys.a) || (keys.w ^ keys.s)) {
        PI_VECTOR3X vel{
            static_cast<float>(keys.d - keys.a),
            static_cast<float>(keys.w - keys.s),
            0.0f,
            0.0f
        };
        pi_vector_normalize4(&vel, vel);

        PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(sphere1.hrigidbody);
        properties->position += vel * dt * SPEED;
        PI_CLAMPX(properties->position.x, -3.0f, 3.0f);
        PI_CLAMPX(properties->position.y, -2.0f, 2.0f);
        sphere1.set_dirty_flag();
    }
    if ((keys.right ^ keys.left) || (keys.up ^ keys.down)) {
        PI_VECTOR3X vel{
            static_cast<float>(keys.right - keys.left),
            static_cast<float>(keys.up - keys.down),
            0.0f,
            0.0f
        };
        pi_vector_normalize4(&vel, vel);

        PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(sphere2.hrigidbody);
        properties->position += vel * dt * SPEED;
        PI_CLAMPX(properties->position.x, -3.0f, 3.0f);
        PI_CLAMPX(properties->position.y, -2.0f, 2.0f);
        sphere2.set_dirty_flag();
    }

    sphere1.update();
    sphere2.update();
    return true;
}

//

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

#include <vector>
using VERTEX_VECTOR = std::vector<PI_VERTEX>;

static VERTEX_VECTOR generate_sphere_mesh(float radius, int h_vertices, int v_vertices, int* indices, bool flatshaded=false) noexcept {
    VERTEX_VECTOR vertices;
    vertices.reserve(h_vertices*v_vertices);

    static thread_local std::vector<float> h_anglesines;
    h_anglesines.resize(h_vertices*2);

    float inc = static_cast<float>(360_deg/h_vertices);
    float vinc = static_cast<float>(2.0/(v_vertices-1));

    {
        float angle = 0.0f;
        for (int i = 0; i < h_vertices; i++, angle += inc) {
            h_anglesines.at(i*2+0) = cosf(angle); 
            h_anglesines.at(i*2+1) = sinf(angle); 
        }
    }

    float y = -1.0f;
    for (int j = 0; j < v_vertices; j++, y += vinc) {
        float angle = 0.0f;
        float cy = static_cast<float>(cos(y*(PI_M_PI/2)));
        cy = powf(cy, 0.5f);
        for (int i = 0; i < h_vertices; i++, angle += inc) {
            float cx = h_anglesines.at(i*2+0);
            float sx = h_anglesines.at(i*2+1);
            PI_VERTEX vt{
                { cy*cx*radius, y*radius, cy*sx*radius },
                { cy*cx, y, cy*sx }
            };
            vertices.push_back(vt);
        }
    }

    int num_indices = (h_vertices*(v_vertices-1)) * 6;
    if (flatshaded) {
        VERTEX_VECTOR flatverts;
        flatverts.reserve(vertices.size()*4);

        for (size_t i = 0; i < vertices.size() - h_vertices; i++) {
            int a = 1;
            a = i%h_vertices == h_vertices-1 ? 1-h_vertices : a;
            const PI_VERTEX* verts[] = {
                &vertices.at(i+0),
                &vertices.at(i+a),
                &vertices.at(i+h_vertices+0),
                &vertices.at(i+h_vertices+a)
            };
            
            PI_VECTOR3 normal{
                (verts[0]->normal.x + verts[1]->normal.x + verts[2]->normal.x + verts[3]->normal.x)*0.25f,
                (verts[0]->normal.y + verts[1]->normal.y + verts[2]->normal.y + verts[3]->normal.y)*0.25f,
                (verts[0]->normal.z + verts[1]->normal.z + verts[2]->normal.z + verts[3]->normal.z)*0.25f
            };
            PI_VERTEX vt;
            vt.normal = normal;
            for (const auto v : verts) {
                vt.pos = v->pos;
                flatverts.push_back(vt);
            }
        }

        for (int i = 0, n = 0; i < num_indices; i+= 6, n += 4) {
            indices[i+0] = n+2;            
            indices[i+1] = n+3;            
            indices[i+2] = n+1;            
            indices[i+3] = n+1;            
            indices[i+4] = n+0;            
            indices[i+5] = n+2;            
        }
        return flatverts;
    }
    else
    {
        for (int i = 0, n = 0; i < num_indices; i += 6, n++) {
            int face[4] = {
                n+0, n+h_vertices+0,
                n+1, n+h_vertices+1
            };

            indices[i+0] = face[0];
            indices[i+1] = face[1];
            indices[i+2] = face[2];
            indices[i+3] = face[2];
            indices[i+4] = face[1];
            indices[i+5] = face[3];
        }
    }


    return vertices;
}

static PI_HMESH create_sphere_mesh(PI_HGAME hgame) noexcept
{
    const int num_indices = 32*32*6;
    int indices[ num_indices ];

    auto verts = generate_sphere_mesh(1, 32, 32, indices, true);
    //return pi_create_mesh(hgame, (int)verts.size(), verts.data());
    return pi_create_mesh_indexed(hgame, (int)verts.size(), verts.data(), num_indices, indices, PI_INDEX_U32);
}



SPHERE::SPHERE(PI_HGAME hgame, const PI_VECTOR3X& start_position, float radius) noexcept:
    hrigidbody{ pi_add_rigid_body(hgame->hscene) },
    hinstance{ pi_add_scene_mesh(hgame->hscene, hgame->hsphere, NULL) }
{
    PI_RIGIDBODY_PROPERTIES* properties = pi_get_rigid_body_properties(hrigidbody);
    properties->position = start_position;
    properties->scale = { radius, radius, radius, 1.0f };
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
