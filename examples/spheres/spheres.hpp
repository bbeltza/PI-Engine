#ifndef SPHERES_HPP
#define SPHERES_HPP

#include <piengine/pi_core.h>
#include <piengine/pi_render.h>
#include <piengine/pi_physics.h>
#include <piengine/pi_debug.h>

#include <piengine/pi_raii.hpp>

#include <vector>

PI_DECLARE_RAIIH(PI_HWORLDENV);
PI_DECLARE_RAIIH(PI_HSCENE);
PI_DECLARE_RAIIH(PI_HMESH);
PI_DECLARE_RAIIH(PI_HRIGIDBODY);
PI_DECLARE_RAIIH(PI_HINSTANCE);

struct SPHERE {
    SPHERE(PI_HGAME hgame, const PI_VECTOR3X& start_position, float radius) noexcept;
    SPHERE(const SPHERE&) = delete;
    void operator =(const SPHERE&) = delete;

    inline SPHERE(SPHERE&& right) noexcept:
       hrigidbody{ std::move(right.hrigidbody) },
       hinstance{ std::move(right.hinstance) },
       m_instance{ right.m_instance },
       m_dirtyflag{ right.m_dirtyflag } {
        pi_attach_transform(hrigidbody, &m_instance.transform, &m_dirtyflag);
    }
    inline void operator =(SPHERE&& right) = delete;

    void update() noexcept;
    void set_dirty_flag() noexcept { m_dirtyflag = true; }

    PI_HRIGIDBODY_RAII hrigidbody;
    PI_HINSTANCE_RAII hinstance;
private:
    PI_INSTANCE m_instance;
    bool m_dirtyflag = true;
};

struct CUBE {
    CUBE(PI_HGAME hgame, const PI_VECTOR3X& position, const PI_VECTOR3X& scale) noexcept;
private:
    PI_HRIGIDBODY_RAII m_hrigidbody;
    PI_HINSTANCE_RAII m_hinstance;
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
    PI_HMESH_RAII hcube;

    std::vector<SPHERE> spheres;

    CUBE ground;

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
