#ifndef APP_HPP
#define APP_HPP

#include <piengine/pi_core.h>
#include <piengine/pi_render.h>

#include <piengine/pi_math.h>
#include <piengine/pi_raii.hpp>

PI_DECLARE_RAIIH(PI_HWORLDENV);
PI_DECLARE_RAIIH(PI_HSCENE);
PI_DECLARE_RAIIH(PI_HMESH);
PI_DECLARE_RAIIH(PI_HINSTANCE);

struct PI_GAME {
    PI_GAME() noexcept;
    ~PI_GAME() noexcept;

    bool ok() const noexcept { return hscene; }

    bool app_event(const PI_EVENT& event) noexcept;
    bool app_update() noexcept;
private:
    PI_HWORLDENV_RAII hworldenv;
    PI_HSCENE_RAII hscene;

    PI_HMESH_RAII hmesh;
    PI_HINSTANCE_RAII hinstance;

    float cubeyaw = 0.0f;
    float cubepitch = 0.0f;
    float realcubeyaw = 0.0f;
    float realcubepitch = 0.0f;

    struct {
        bool cube : 1;
    } dirty { true };
};

#endif
