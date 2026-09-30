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
    
    PI_HMESH m_htriangle;
    PI_HINSTANCE m_hgroundinstance;
    PI_HINSTANCE m_hinstance;

private:
    PI_VECTOR3 m_tri_rotation = {0.0f, 0.0f, 0.0f};
    PI_INSTANCE m_tri_datainst = {
        {},
        3.0f, 3.0f, 3.0f, 1.0f
    };

    struct {
        bool inst = true;
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

const PI_VERTEX TRIANGLE[] = {
    PI_VERTEX{ -0.5f, -0.5, 0.0f, 1.0f, 0.0f, 0.0f },
    PI_VERTEX{  0.0f,  0.5, 0.0f, 0.0f, 1.0f, 0.0f },
    PI_VERTEX{  0.5f, -0.5, 0.0f, 0.0f, 0.0f, 1.0f }
};

const PI_INSTANCE GROUND_INSTANCE = {
    {
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f, 0.0f,
        0.0f, -2.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 2.0f, 1.0f
    },
    1.0f, 1.0f, 1.0f, 0.5f
};

gaem_t::gaem_t(bool& fail):
    m_hscene{ pi_create_scene(this, true) },
    m_htriangle{ pi_create_mesh(this, PI_ARRAYSIZE(TRIANGLE), TRIANGLE) },
    m_hinstance{ pi_add_scene_mesh(m_hscene, m_htriangle, NULL) }, // `instance` can be NULL, in that case, use a default instance with an identity transform and a white albedo color
    m_hgroundinstance{ pi_add_scene_mesh(m_hscene, m_htriangle, &GROUND_INSTANCE) }
{
    PI_MATRIX_4X4 p = pi_matrix_perspective(70_degf, 1.0f, 0.1f, 3.0f);
    pi_set_scene_projection(m_hscene, PI_MFLOATP(p));
    //pi_set_scene_view(m_hscene, i.m);
}

gaem_t::~gaem_t() {
    //pi_remove_scene_mesh(m_hgroundinstance);
    pi_remove_scene_mesh(m_hinstance);
    pi_release_scene(m_hscene);
    pi_destroy_mesh(m_htriangle);
}

bool gaem_t::event(PI_EVENT* event)
{
    constexpr float SENSIBILITY = 1_degf;
    switch (event->type)
    {
        case PI_EVENT_MOUSEMOVE:
            if (!(event->motion.state & PI_MSTATE_LEFT)) {
                break;
            }

            m_dirty.inst = true;
            m_tri_rotation.x -= event->motion.delta_y * SENSIBILITY;
            m_tri_rotation.y -= event->motion.delta_x * SENSIBILITY;
            break;
        case PI_EVENT_UNKNOWN:
            std::printf("Got UNKNOWN event\n");
            break;
        default:
            break;
    }

    return true;
}

bool gaem_t::update()
{
  
    //float dt = pi_get_runtime_dtf(this);
    //std::printf("dt: %f (1/dt: %f)\n", dt, 1/dt);
    
    
    if (m_dirty.inst) {
        m_dirty.inst = false;
        auto tmp = pi_matrix_rotate_x(m_tri_rotation.x, 0, 0);
        *PI_FPTOMATRIX_4X4(m_tri_datainst.m) = tmp;
        tmp = pi_matrix_rotate_y(0, m_tri_rotation.y, 0);
        *PI_FPTOMATRIX_4X4(m_tri_datainst.m) = pi_mul_4x4(PI_FPTOCMATRIX_4X4(m_tri_datainst.m), &tmp);
   
        PI_FPTOMATRIX_4X4(m_tri_datainst.m)->wz = 2.0f;

        pi_update_mesh_instance(m_hinstance, &m_tri_datainst);
    }
    return true;
}

