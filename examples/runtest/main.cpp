#include <piengine/pi_main.h>
#include <piengine/pi_render.h>
#include <piengine/pi_core.h>
#include <piengine/pi_math.h>

#include <cstdio>

namespace appcallbacks
{
    bool alloc_data(size_t* size);
    bool start(PI_HGAME hgame, va_list args);
    void close(PI_HGAME hgame);

    bool event(PI_HGAME hgame, PI_EVENT* event);
    bool update(PI_HGAME hgame);
}

int main(int argc, char* argv[])
{
    PI_APPCALLBACKS callbacks{
        appcallbacks::alloc_data,
        appcallbacks::start,
        appcallbacks::close,
        appcallbacks::event,
        appcallbacks::update
    };
    PI_APPINFO appinfo{
        "Test Runtime App",
            &callbacks
    };

    return pi_run_ex(&appinfo, 2, argc, argv);
}

#define gaem_t PI_GAME

struct gaem_t
{
    gaem_t(bool& fail);
    ~gaem_t();

    bool event(PI_EVENT* event);
    bool update();
private:
    PI_HSCENE m_hscene;

    PI_HWORLDENV m_hworldenv;
    
    PI_HMESH m_hcube;
    PI_HINSTANCE m_hinstances[8];
private:
    PI_VECTOR4 m_viewangle = { 0.0f, 0.0f, 0.0f, 2.0f };
    float m_viewrealdistance = m_viewangle.w;

    struct {
        bool inst = true;
        bool view = true;
    } m_dirty;
};


bool appcallbacks::alloc_data(size_t* size/*, size_t* alignment*/) {
    *size = sizeof(gaem_t);
    //*alignment = alignof(gaem_t);

    return true;
}
bool appcallbacks::start(PI_HGAME hgame, va_list args)
{
    bool fail = false;
    new (hgame) gaem_t{fail};
    if (fail) {
        
        return false;
    }
    return true;
}
void appcallbacks::close(PI_HGAME hgame) {
    hgame->~gaem_t();
}

bool  appcallbacks::event(PI_HGAME hgame, PI_EVENT* event) {
    return hgame->event(event);
}
bool appcallbacks::update(PI_HGAME hgame) {
    return hgame->update();
}

//

PI_HWORLDENV create_game_worldenv(PI_HGAME hgame) noexcept
{
    PI_WORLDENV_RASTERIZER rasterizer{};
    rasterizer.cullflags = PI_CULL_BACK;
    rasterizer.fillmode = PI_FILL_SOLID;

    PI_WORLDENV_DEPTH depth{};
    depth.depthcompare = PI_COMPARE_LESS;
    depth.writeenable = true;

    PI_WORLDENV_MSAA multisample{};
    multisample.samples = 4;
    multisample.best_quality = false;

    PI::WORLDENV_DESC desc{PI_WORLDEFFECT_NONE, rasterizer, depth, multisample};
    return pi_create_worldenvironment(hgame, &desc);
}

//

const PI_VERTEX TRIANGLE[] = {
    PI_VERTEX{ { -0.5f, -0.5f, 0.0f} , { 1.0f, 0.0f, 0.0f } },
    PI_VERTEX{ { 0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    PI_VERTEX{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
};

const PI_VERTEX QUAD[] = {
    PI_VERTEX{ { -0.5f,  0.5f, 0.0f }, { 0.5f, 0.5f, 0.5f } },
    PI_VERTEX{ { 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
    PI_VERTEX{ { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.5f } },
    PI_VERTEX{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f } }
};

const uint16_t QUAD_INDICES[] = {
    0, 1, 2,
    2, 3, 0
};

const uint16_t CUBE_INDICES[] = {
    0, 1, 2,
    2, 3, 0,

    4, 5, 6,
    6, 7, 4,

    8, 9, 10,
    10, 11, 8,

    12, 13, 14,
    14, 15, 12,

    16, 17, 18,
    18, 19, 16,

    20, 21, 22,
    22, 23, 20
};

const PI_VERTEX CUBE_VERTICES[] = {
    { { -0.5f,  0.5f, -0.5f }, { 0.7f, 0.7f, 0.7f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.7f, 0.7f, 0.7f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.7f, 0.7f, 0.7f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.7f, 0.7f, 0.7f } },

    { {  0.5f,  0.5f,  0.5f }, { 0.55f, 0.55f, 0.55f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.55f, 0.55f, 0.55f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.55f, 0.55f, 0.55f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.55f, 0.55f, 0.55f } },

    { {  -0.5f,  0.5f,  0.5f }, { 0.8f, 0.8f, 0.8f } },
    { {  -0.5f,  0.5f,  -0.5f }, { 0.8f, 0.8f, 0.8f } },
    { {  -0.5f, -0.5f, -0.5f }, { 0.8f, 0.8f, 0.8f } },
    { {  -0.5f, -0.5f,  0.5f }, { 0.8f, 0.8f, 0.8f } },

    { {   0.5f,  0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
    { {   0.5f,  0.5f,  0.5f }, { 0.5f, 0.5f, 0.5f } },
    { {   0.5f, -0.5f,  0.5f }, { 0.5f, 0.5f, 0.5f } },
    { {   0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },

    { {  -0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } },
    { {   0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } },
    { {   0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
    { {  -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },

    { {  -0.5f, -0.5f, -0.5f }, { 0.4f, 0.4f, 0.4f } },
    { {   0.5f, -0.5f, -0.5f }, { 0.4f, 0.4f, 0.4f } },
    { {   0.5f, -0.5f,  0.5f }, { 0.4f, 0.4f, 0.4f } },
    { {  -0.5f, -0.5f,  0.5f }, { 0.4f, 0.4f, 0.4f } },
};


gaem_t::gaem_t(bool& fail):
    m_hscene{ pi_create_scene(this) },
    m_hworldenv{ create_game_worldenv(this) },
    m_hcube{ pi_create_mesh_indexed(this,
            PI_ARRAYSIZE(CUBE_VERTICES), CUBE_VERTICES,
            PI_ARRAYSIZE(CUBE_INDICES), CUBE_INDICES,
            PI_INDEX_U16
        )
    }
{
    //pi_set_scene_view(m_hscene, i.m);
    if (!m_hworldenv) {
        fail = true;
        return;
    }
    if (!m_hscene) {
        fail = true;
        return;
    }

    /* Initialize the render subsystem in the scene */
    if (!pi_init_render_scene(m_hscene, m_hworldenv)) {
        fail = true;
        return;
    }

    static const PI_INSTANCE INSTANCES[PI_ARRAYSIZE(m_hinstances)] = {
        {
            {
                3.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 3.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 2.0f, 0.0f,
                -0.65f, -1.0f, -1.0f, 1.0f
            },
            { 1.0f, 1.0f, 0.55f, 1.0f }
        }, {
            {
                1.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 3.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.5f, 0.0f,
                1.2f, -0.75f,
                INSTANCES[0].transform.wz + (INSTANCES[0].transform.zz + INSTANCES[1].transform.zz) / 2,
                1.0f
            },
            INSTANCES[0].col
        }, {
            {
                1.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 3.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.5f, 0.0f,
                -1.2f, -0.75f,
                INSTANCES[0].transform.wz + (INSTANCES[0].transform.zz + INSTANCES[1].transform.zz) / 2,
                1.0f
            },
            INSTANCES[0].col
        }, {
            {
                0.3f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.31f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.3f, 0.0,
                0.0f, 0.7f, 0.7f, 1.0f
            },
            { 0.4f, 1.0f, 0.4f, 0.95f }
        },
            
        {
            {
                0.3f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.3f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, INSTANCES[0].transform.wy + (INSTANCES[0].transform.yy + INSTANCES[4].transform.yy) / 2,
                -0.35f, 1.0f
            },
            { 1.0f, 0.45f, 0.15f, 0.8f }
        }, {
            {
                INSTANCES[4].transform.zz - 0.4f, 0.0f, 0.0f, 0.0f,
                0.0f, INSTANCES[4].transform.yy, 0.0f, 0.0f,
                0.0f, 0.0f, INSTANCES[4].transform.xx, 0.0f,
                INSTANCES[4].transform.wx - (INSTANCES[4].transform.xx + INSTANCES[5].transform.xx) / 2,
                INSTANCES[4].transform.wy,
                INSTANCES[4].transform.wz - (INSTANCES[4].transform.zz - INSTANCES[5].transform.zz) / 2, 1.0f
            },
            INSTANCES[4].col
        }, {
            {
                INSTANCES[5].transform.zz, 0.0f, 0.0f, 0.0f,
                0.0f, INSTANCES[5].transform.yy, 0.0f, 0.0f,
                0.0f, 0.0f, INSTANCES[5].transform.xx, 0.0f,
                INSTANCES[5].transform.wx - (INSTANCES[5].transform.xx - INSTANCES[6].transform.xx) / 2,
                INSTANCES[5].transform.wy,
                INSTANCES[5].transform.wz - (INSTANCES[5].transform.zz + INSTANCES[6].transform.zz) / 2, 1.0f
            },
            INSTANCES[4].col
        }, {
            {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, INSTANCES[6].transform.yy, 0.0f, 0.0f,
                0.0f, 0.0f, INSTANCES[6].transform.xx, 0.0f,
                INSTANCES[6].transform.wx - (INSTANCES[6].transform.xx + INSTANCES[7].transform.xx) / 2,
                INSTANCES[6].transform.wy,
                INSTANCES[6].transform.wz - (INSTANCES[6].transform.zz - INSTANCES[7].transform.zz) / 2, 1.0f
            },
            INSTANCES[4].col
        } 
    };

    for (int i = 0; i < PI_ARRAYSIZE(m_hinstances); i++) {
        m_hinstances[i] = pi_add_scene_mesh(m_hscene, m_hcube, &INSTANCES[i]);
    }

    pi_set_scene_clear_color(m_hscene, 1.0f, 0.95f, 0.35f, 1.0f);
    pi_enable_scene(m_hscene, -1);
}

gaem_t::~gaem_t() {
    pi_release_worldenvironment(m_hworldenv);

    for (auto hinst : m_hinstances) {
        pi_remove_scene_mesh(hinst);
    }
    pi_destroy_mesh(m_hcube);
    pi_destroy_scene(m_hscene);
}

bool gaem_t::event(PI_EVENT* event)
{
    constexpr float SENSIBILITY = 1_degf;
    constexpr float WHEEL_FORCE = 0.5f;
    switch (event->type)
    {
        case PI_EVENT_MOUSEMOVE:
            if (event->motion.state & PI_MSTATE_LEFT) {
                m_dirty.inst = true;
            }
            if (event->motion.state & PI_MSTATE_RIGHT) {
                m_dirty.view = true;
                m_viewangle.x += event->motion.delta_y * SENSIBILITY;
                m_viewangle.y += event->motion.delta_x * SENSIBILITY;
            }

            break;
        case PI_EVENT_WINDOW_SIZE: {
            float ratio = event->window.data1 / static_cast<float>(event->window.data2);
            PI_MATRIX_4X4 p = pi_matrix_perspective(70_degf, ratio, 0.1f, 10.0f);
            pi_set_scene_projection(m_hscene, &p);
            //std::printf("WINDOW_RESIZED: %d %d\n", event->window.data1, event->window.data2);
        } break;
        case PI_EVENT_MOUSEWHEEL:
            if (!event->wheel.amnt_y)
                break;
            m_viewangle.w -= event->wheel.amnt_y * WHEEL_FORCE;
            if (m_viewangle.w < 1.0f)
                m_viewangle.w = 1.0f;
            else
                m_dirty.view = true;
            break;
        case PI_EVENT_UNKNOWN:
            //std::printf("Got UNKNOWN event\n");
            break;
        default:
            break;
    }

    return true;
}

bool gaem_t::update()
{ 
    float dt = pi_get_runtime_dtf(this);
    //std::printf("dt: %f (1/dt: %f)\n", dt, 1/dt);
  
    
    if (m_dirty.inst) {
        m_dirty.inst = false;
    }
    if (m_dirty.view) {
        float t = PI_CLAMP(dt*15.0f, 0.0f, 1.0f);
        PI_LERPA(m_viewrealdistance, m_viewangle.w, t);
        if (m_viewangle.w == m_viewrealdistance)
            m_dirty.view = false;

        static constexpr PI_VECTOR4 CENTER_VECTOR = { 0.0f, 0.0f, 0.0f };

        PI_MATRIX_4X4 rotx;
        PI_MATRIX_4X4 roty;
        PI_VECTOR4 eye = { 0.0f, 0.0f, -m_viewrealdistance };
        pi_matrix_rotate_y(&rotx, m_viewangle.y);
        pi_matrix_rotate_x(&roty, m_viewangle.x);
        eye = pi_mulv_4x4(eye, pi_mul_4x4(&rotx, &rotx, &roty));

        PI_VECTOR4 up = *(PI_VECTOR4*)rotx.m[1];

        PI_MATRIX_4X4 view;
        pi_look_at(&view, eye, CENTER_VECTOR, up);

        pi_set_scene_view(m_hscene, &view);
    }
    return true;
}

