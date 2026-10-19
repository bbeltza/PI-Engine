#ifndef SPHERES_HPP
#define SPHERES_HPP

#include <piengine/pi_core.h>
#include <piengine/pi_render.h>
#include <piengine/pi_physics.h>
#include <piengine/pi_debug.h>

#include <piengine/pi_raii.hpp>

PI_DECLARE_RAIIH(PI_HWORLDENV);
PI_DECLARE_RAIIH(PI_HSCENE);
PI_DECLARE_RAIIH(PI_HMESH);
PI_DECLARE_RAIIH(PI_HRIGIDBODY);
PI_DECLARE_RAIIH(PI_HINSTANCE);

struct SPHERE {
    SPHERE(PI_HGAME hgame, const PI_VECTOR3X& start_position, float radius) noexcept;

    void update() noexcept;
    void set_dirty_flag() noexcept { m_dirtyflag = true; }

    PI_HRIGIDBODY_RAII hrigidbody;
    PI_HINSTANCE_RAII hinstance;
private:
    PI_INSTANCE m_instance;
    bool m_dirtyflag;
};

struct PI_GAME
{
    PI_GAME();

    bool app_event(const PI_EVENT& event) noexcept;
    bool app_update() noexcept;

    bool ok() const noexcept { return hscene; }

    PI_HWORLDENV_RAII hworldenv;
    PI_HSCENE_RAII hscene;

    PI_HMESH_RAII hsphere;

    SPHERE sphere1;
    SPHERE sphere2;

    struct {
        bool w : 1;
        bool a : 1;
        bool s : 1;
        bool d : 1;

        bool up : 1;
        bool left : 1;
        bool down : 1;
        bool right : 1;
    } keys{};
};

#endif
