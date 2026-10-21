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

    inline void set_color(const PI_VECTOR4& color) {
        m_instance.col = color;
        m_dirtyflag = true;
    }

    constexpr PI_VECTOR4 get_color() const { return m_instance.col; }

    void update() noexcept;
    void set_dirty_flag() noexcept { m_dirtyflag = true; }

    PI_HRIGIDBODY_RAII hrigidbody;
    PI_HINSTANCE_RAII hinstance;
private:
    PI_INSTANCE m_instance;
    bool m_dirtyflag = true;
};

struct CUBE {
    CUBE(PI_HGAME hgame, const PI_VECTOR3X& position, const PI_VECTOR3X& scale, const PI_VECTOR4& color={ 1.0f, 1.0f, 1.0f, 1.0f }) noexcept;
private:
    PI_HRIGIDBODY_RAII m_hrigidbody;
    PI_HINSTANCE_RAII m_hinstance;
};
