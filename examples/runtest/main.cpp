#include <piengine/pi_main.h>
#include <piengine/pi_render.h>

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
private:
    PI_HSCENE m_hscene;
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
void appcallbacks::close(PI_HGAME hgame)
{
    hgame->~gaem_t();
}

bool appcallbacks::update(PI_HGAME hgame)
{
    return true;
}
bool appcallbacks::render(PI_HGAME hgame)
{
    return true;
}

//

gaem_t::gaem_t(bool& fail):
    m_hscene{ pi_create_scene(this) }
{
    
}

gaem_t::~gaem_t() {
    pi_release_scene(m_hscene);
}
