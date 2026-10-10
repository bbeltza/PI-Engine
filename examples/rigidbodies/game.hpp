#ifndef RIGIDBODIES_GAME_HPP
#define RIGIDBODIES_GAME_HPP

#include <piengine/pi_core.h>
#include <piengine/pi_physics.h>
#include <piengine/pi_render.h>

#include <piengine/pi_raii.hpp>

PI_DECLARE_RAIIH(PI_HSCENE);
PI_DECLARE_RAIIH(PI_HWORLDENV);
PI_DECLARE_RAIIH(PI_HRIGIDBODY);
PI_DECLARE_RAIIH(PI_HMESH);
PI_DECLARE_RAIIH(PI_HINSTANCE);

struct GAME_CUBE {
    GAME_CUBE(PI_HGAME hgame, const PI_VECTOR3& position, const PI_VECTOR3& scale, PI_HMESH hmesh);

    PI_INSTANCE instance;
    bool instance_dirty = false;

    PI_HMESH hmesh;
    PI_HINSTANCE_RAII hinstance;
    PI_HRIGIDBODY_RAII hrigidbody;
};

struct PI_GAME
{
    PI_GAME();

    bool ok() const noexcept { return hscene; }
    bool app_event(PI_EVENT& event) noexcept;
    bool app_update() noexcept;

    PI_HSCENE get_scene() noexcept { return hscene; }
    PI_HWORLDENV get_worldenv() noexcept { return hworldenv; }
private:
    PI_HWORLDENV_RAII hworldenv;
    PI_HSCENE_RAII hscene;

    PI_HMESH hcubemesh;

    GAME_CUBE ground;
    GAME_CUBE falling_cube;

    bool k_left : 1;
    bool k_right : 1;
    bool k_top : 1;
    bool k_bottom : 1;
    bool k_front : 1;
    bool k_back : 1;

    bool dirty_look : 1;
    bool dirty_projection : 1;
bool dirty_view : 1;

    PI_VECTOR2 camera_look = { 0.0f, 0.0f };
    PI_VECTOR4 camera_eye = { 0.0f, 0.0f, -2.0f };
    PI_MATRIX_4X4 camera_dir;

    unsigned wnd_w, wnd_h;
private:
    bool _handle_key(const PI_KEYEVENT& event) noexcept;
    bool _handle_motion(const PI_MOTIONEVENT& event) noexcept;
    bool _handle_wheel(const PI_WHEELEVENT& event) noexcept;
    void _update_dirty() noexcept;
};

#endif
