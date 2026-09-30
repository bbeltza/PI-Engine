#include <piengine/pi_main.h>
#include <piengine/pi_render.h>
#include <piengine/pi_math.h>

#include <cstdio>

namespace appcallbacks
{
    bool alloc_data(size_t* size);
    bool start(PI_HGAME hgame, va_list args);
    void close(PI_HGAME hgame);

    bool update(PI_HGAME hgame);
    bool render(PI_HGAME hgame);
}

int main(int argc, char* argv[])
{
    PI_APPCALLBACKS callbacks{
        appcallbacks::alloc_data,
        appcallbacks::start,
        appcallbacks::close,
        appcallbacks::update,
        appcallbacks::render
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

    bool update();
    bool render();
private:
    PI_HSCENE m_hscene;
    
    PI_HMESH m_htriangle;
    PI_HINSTANCE m_hinstance;

private:
    PI_VECTOR3 m_tri_rotation = {0.0f, 0.0f, 0.0f};
    PI_INSTANCE m_tri_datainst = {
        {},
        2.0f, 2.0f, 2.0f, 1.0f
    };
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

bool appcallbacks::update(PI_HGAME hgame) {
    return hgame->update();
}
bool appcallbacks::render(PI_HGAME hgame) {
    return hgame->render();
}

//

const PI_VERTEX TRIANGLE[] = {
    PI_VERTEX{ -0.5f, -0.5, 0.0f, 1.0f, 0.0f, 0.0f },
    PI_VERTEX{  0.0f,  0.5, 0.0f, 0.0f, 1.0f, 0.0f },
    PI_VERTEX{  0.5f, -0.5, 0.0f, 0.0f, 0.0f, 1.0f }
};

gaem_t::gaem_t(bool& fail):
    m_hscene{ pi_create_scene(this, true) },
    m_htriangle{ pi_create_mesh(this, PI_ARRAYSIZE(TRIANGLE), TRIANGLE) },
    m_hinstance{ pi_add_scene_mesh(m_hscene, m_htriangle, NULL) } // `instance` can be NULL, in that case, use a default instance with an identity transform and a white albedo color
{
    PI_MATRIX_4X4 p = pi_matrix_perspective(2.0f, 1.0f, 0.1f, 2.0f);
    pi_set_scene_projection(m_hscene, PI_MFLOATP(p));
    //pi_set_scene_view(m_hscene, i.m);
}

gaem_t::~gaem_t() {
    pi_remove_scene_mesh(m_hinstance);
    pi_release_scene(m_hscene);
    pi_destroy_mesh(m_htriangle);
}

bool gaem_t::update()
{
    m_tri_rotation.y += 0.004f;

    *PI_FPTOMATRIX_4X4(m_tri_datainst.m) = pi_matrix_rotate_xyz(m_tri_rotation.x, m_tri_rotation.y, m_tri_rotation.z);
    PI_FPTOMATRIX_4X4(m_tri_datainst.m)->wz = 0.9f;
    pi_update_mesh_instance(m_hinstance, &m_tri_datainst);
    return true;
}

bool gaem_t::render()
{
    return true;
}
