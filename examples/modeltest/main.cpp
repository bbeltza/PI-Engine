#include <piengine/pi_render.h>
#include <piengine/pi_core.h>
#include <piengine/pi_math.h>
#include <piengine/pi_main.h>

#include "tiny_obj_loader.h"

#include <piengine/pi_raii.hpp>

PI_DECLARE_RAIIH(PI_HSCENE);
PI_DECLARE_RAIIH(PI_HMESH);
PI_DECLARE_RAIIH(PI_HWORLDENV);
PI_DECLARE_RAIIH(PI_HINSTANCE);

struct PI_GAME
{
    PI_GAME() noexcept;

    bool ok() const noexcept { return hscene; }
    bool app_event(PI_EVENT& event) noexcept;
    bool app_update() noexcept;
private:
    PI_HSCENE_RAII hscene;
    PI_HWORLDENV_RAII hworldenv;

    PI_HMESH htriangle;
    PI_HINSTANCE htriangle_instance;

    float time = 0.0f;
    PI_VECTOR2 rotation;

    unsigned wnd_w, wnd_h;

    struct {
        bool triangle : 1;
        bool projection : 1;
        bool useortho : 1;
    } dirty{ true };
};

PI_HMESH create_guitar_mesh(PI_HGAME hgame) noexcept
{
    std::puts("create_guitar_mesh: Loading guitar model...");

    tinyobj::ObjReaderConfig readerconfig;
    readerconfig.triangulate = true;
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile("guitarmodel.obj", readerconfig
                ) ) {
        if (!reader.Error().empty()) {
            std::fprintf(stderr, "LOADING ERROR: %s", reader.Error().c_str());
        }

        return NULL;
    }

    std::vector<PI_VERTEX> vertices;
    std::vector<unsigned> indices;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    for (auto& shape : shapes)
    {
        indices.reserve(indices.size() + shape.mesh.indices.size());
        vertices.reserve(vertices.size() + shape.mesh.indices.size());
        
        unsigned i = 0;
        for (unsigned fv : shape.mesh.num_face_vertices)
        {
            for (unsigned vi = 0; vi < fv; vi++, i++) {
                const tinyobj::index_t& index = shape.mesh.indices.at(i);

                const unsigned vtsindex = 3*index.vertex_index;
                PI_VERTEX vt = {
                    PI_VECTOR3{
                        attrib.vertices.at(vtsindex+0),
                        attrib.vertices.at(vtsindex+1),
                        attrib.vertices.at(vtsindex+2)
                    },
                    PI_VECTOR3{ 1.0f, 1.0f, 1.0f }
                };

                if (index.normal_index >= 0) {
                    /* We don't have lighting yet, do we generate static colors ourselves based on the model's normals. */

                    unsigned nmsindex = 3*index.normal_index;

                    static PI_VECTOR4 LIGHT_DIR = *pi_vector_normalize4(&LIGHT_DIR,
                        PI_VECTOR3X{
                            -0.3f,
                            0.7f,
                            0.8f
                        } );

                    PI_VECTOR3 normal = {
                        attrib.normals.at(nmsindex+0),
                        attrib.normals.at(nmsindex+1),
                        attrib.normals.at(nmsindex+2)
                    };

                    float diff = LIGHT_DIR.x*normal.x + LIGHT_DIR.y*normal.y + LIGHT_DIR.z*normal.z;
                    diff = PI_CLAMP(diff, 0.4f, 1.0f);

                    vt.col = PI_VECTOR3{
                        diff * 0.9f,
                        diff * 1.1f,
                        diff * 1.4f
                    };
                }

                vertices.push_back(vt);
                indices.push_back(i);
            }

            if (fv > 3) {
                PI_ASSERT(fv == 4);
                indices.push_back(i - fv + 0);
                indices.push_back(i - fv + 1);
            }
        }
    }

    std::puts("create_guitar_mesh: Successfully loaded model!");
    return pi_create_mesh_indexed(hgame,
            (int)vertices.size(), vertices.data(),
            (int)indices.size(), indices.data(),
            PI_INDEX_U32
        );
}

PI_HWORLDENV create_worldenv(PI_HGAME hgame)
{
    PI_WORLDENV_RASTERIZER raster = {};
    raster.cullflags = PI_CULL_NONE;
    raster.depthclip = true;

    PI_WORLDENV_DEPTH depth = {};
    depth.depthwrite = true;
    depth.depthcompare = PI_COMPARE_LESS_EQUAL;
    //depth.depthcompare = PI_COMPARE_ALWAYS;

    PI_WORLDENV_MSAA multisample = {};
    multisample.samples = 4;
    multisample.best_quality = false;

    PI::WORLDENV_DESC desc{
        PI_WORLDEFFECT_NONE,
        raster,
        depth,
        multisample
    };

    return pi_create_worldenvironment(hgame, &desc);
}

PI_HMESH create_triangle(PI_HGAME hgame)
{
    PI_VERTEX vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };

    return pi_create_mesh(hgame, PI_ARRAYSIZE(vertices), vertices);
}

//

PI_GAME::PI_GAME() noexcept:
    hscene{ pi_create_scene(this) },
    hworldenv{ create_worldenv(this) },
    htriangle{ create_guitar_mesh(this) },
    htriangle_instance{ /* .. */ }
{
    if (!htriangle) {
        hscene.release();
        return;
    }

    if (!pi_init_render_scene(hscene, hworldenv)) {
        hscene.release();
        return;
    }

    htriangle_instance = pi_add_scene_mesh(hscene, htriangle, NULL);

    pi_enable_scene(hscene, 0);
    std::puts("GAME LOADED: Press E to switch between orthographic projection and perspective projection! (Currently using perspective projection)");
}

bool PI_GAME::app_event(PI_EVENT& event) noexcept
{
    static constexpr float SENSIBILITY = 0.01f;
    switch (event.type)
    {
        case PI_EVENT_MOUSEMOVE:
            if (!(event.motion.state & PI_MSTATE_LEFT))
                break;
            
            dirty.triangle = true;
            rotation.x += event.motion.delta_x * SENSIBILITY;
            rotation.y += event.motion.delta_y * SENSIBILITY;
            break;
        case PI_EVENT_WINDOW_SIZE:
            dirty.projection = true;
            wnd_w = event.window.data1;
            wnd_h = event.window.data2;
            break;
        case PI_EVENT_KEY:
            if (event.key.press != PI_PRESS_DOWN)
                break;

            switch (event.key.code)
            {
                case PI_KEYCODE_E:
                    dirty.projection = true;
                    dirty.useortho = !dirty.useortho;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return true;
}

bool PI_GAME::app_update() noexcept
{
    #define SNORM_TO_UNORM(x) ((x)*0.5f + 0.5f)
    float s = SNORM_TO_UNORM(sinf(time));
    float c = SNORM_TO_UNORM(sinf(time + (float)PI_M_PI/2));
    float r = SNORM_TO_UNORM(sinf(time + (float)PI_M_PI/1));

    pi_set_scene_clear_color(hscene, s, c, r, 1.0f);
    
    time += pi_get_runtime_dtf(this) * 0.1f;

    if (dirty.triangle)
    {
        dirty.triangle = false;

        PI_INSTANCE inst;
        inst.transform = pi_scale_4x4(0.015f, 0.015f, 0.015f, 1.0f);
        inst.transform.wy -= 0.6f;
        inst.transform.zz *= -1.0f;
        PI_MATRIX_4X4 m;
        pi_matrix_rotate_y(&m, -rotation.x);
        pi_mul_4x4(&inst.transform, &m, &inst.transform);
        pi_matrix_rotate_x(&m, -rotation.y);
        pi_mul_4x4(&inst.transform, &m, &inst.transform);

        inst.transform.wz += 1.5f;
        pi_update_mesh_instance(htriangle_instance, &inst);
    }
    if (dirty.projection)
    {
        dirty.projection = false;

        float ratio = (float)wnd_w/wnd_h;

        PI_MATRIX_4X4 m = dirty.useortho ?
                pi_matrix_ortho(
                    ratio*-0.5f, ratio*0.5f,
                    1.0f, -1.0f,
                    0.0f, 3.0f
                )
              : pi_matrix_perspective(70_degf, ratio, 0.1f, 10.0f);
        
        pi_set_scene_projection(hscene, &m);
    }

    return true;
}

//

int main()
{
    PI_APPCALLBACKS callbacks = {
        [](size_t* psize) -> bool {
            *psize = sizeof(PI_GAME);
            return true;
        },
        [](PI_HGAME hgame, va_list args) -> bool {
            new (hgame) PI_GAME{};
            if (hgame->ok())
                return true;

            hgame->~PI_GAME();
            return false;
        },
        [](PI_HGAME hgame) -> void {
            hgame->~PI_GAME();
        },
        [](PI_HGAME hgame, PI_EVENT* event) -> bool {
            return hgame->app_event(*event);
        },
        [](PI_HGAME hgame) -> bool {
            return hgame->app_update();
        }
    };

    PI_APPINFO appinfo = {
        "My App!!!",
        &callbacks
    };

    return pi_run(&appinfo);
}
