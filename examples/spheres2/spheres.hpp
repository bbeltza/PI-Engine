#ifndef SPHERES_HPP
#define SPHERES_HPP

#include "../spheres/common.hh"

struct CAMERA {
    bool update(PI_HGAME hgame, const PI_VECTOR3& direction) noexcept;
    void rotate(float dtyaw, float dtpitch) noexcept;
    
    PI_VECTOR3 get_position() const { return { { m_position.x, m_position.y, m_position.z } }; }

private:
    PI_VECTOR3X m_position{};
    PI_VECTOR3X m_right,
                m_up,
                m_look;
    float m_yaw = 0.0f, m_pitch = 0.0f;
    bool m_dirtyrot{ true };
    bool m_init{ true };
};

struct PI_GAME {
    PI_GAME() noexcept;
    constexpr bool ok() const noexcept { return hscene; }

    bool app_event(const PI_EVENT& event) noexcept;
    bool app_update() noexcept;

    CAMERA camera;

    PI_HWORLDENV_RAII hworldenv;
    PI_HSCENE_RAII hscene;

    PI_HMESH_RAII hsphere;
    PI_HMESH_RAII hcube;

    std::vector<SPHERE> spheres;
    CUBE grounds[5];

    struct {
        bool w : 1,
             a : 1,
             s : 1,
             d : 1,
             q : 1,
             e : 1;
    } keys{};
};

#endif
