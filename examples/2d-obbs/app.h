#ifndef GAME_H
#define GAME_H

#include <piengine/pi_core.h>
#include <piengine/pi_render.h>

typedef struct RECTANGLE {
    PI_HINSTANCE hinstance;

    PI_VECTOR2 x_axis;
    PI_VECTOR2 y_axis;
    PI_VECTOR2 h_size;
    PI_VECTOR2 position;
} RECTANGLE;


struct PI_GAME {
    PI_HWORLDENV hworldenv;
    PI_HSCENE hscene;

    PI_HMESH hquadmesh;

    RECTANGLE rect1;
    RECTANGLE rect2;
    float rect2angle;

    PI_HWND hmainwnd;

    PI_HINSTANCE htest1;
    PI_HINSTANCE htest2;

    struct {
        bool proj : 1;
        bool rect2 : 1;
        bool rect2angle : 1;
    } dirty;
};

PI_EXTERN_C bool app_start(PI_HGAME hgame, va_list args);
PI_EXTERN_C void app_close(PI_HGAME hgame);

PI_EXTERN_C bool app_event(PI_HGAME hgame, PI_EVENT* event);
PI_EXTERN_C bool app_update(PI_HGAME hgame);

#endif
