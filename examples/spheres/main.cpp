#ifndef APPNAME
    #define APPNAME "Sphere Example!"
#endif

#include <piengine/pi_main.h>
#include "spheres.hpp"

int main(int argc, char* argv[])
{
    PI_APPCALLBACKS callbacks = {
        [](size_t* size) noexcept -> bool {
            *size = sizeof(PI_GAME);
            return true;
        },
        [](PI_HGAME hgame, va_list) -> bool {
            new (hgame) PI_GAME{};
            if (hgame->ok())
                return true;

            hgame->~PI_GAME();
            return false;
        },
        [](PI_HGAME hgame) noexcept -> void {
            hgame->~PI_GAME();
        },
        [](PI_HGAME hgame, PI_EVENT* event) noexcept -> bool {
            return hgame->app_event(*event);
        },
        [](PI_HGAME hgame) noexcept -> bool {
            return hgame->app_update();
        }
    };
    PI_APPINFO appinfo = {
        APPNAME,
        &callbacks
    };
    

    return pi_run(&appinfo);
}
