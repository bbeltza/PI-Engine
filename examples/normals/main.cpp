#include <piengine/pi_main.h>
#include "app.hpp"

int main()
{
    PI_APPCALLBACKS callbacks = {
        [](size_t* psize) noexcept -> bool {
            *psize = sizeof(PI_GAME);
            return true;
        },
        [](PI_HGAME hgame, va_list) noexcept -> bool {
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
        "Cube Normal Example!",
        &callbacks
    };

    return pi_run(&appinfo);
}
