#include "../pi_internal.h"

#include <stdio.h>

/* Windowing API: */

PI_HWND pi_get_main_window(PI_HGAME hgame)
{
    if (!hgame) {
        return NULL;
    }

    // Unimplemented
    return NULL;
}

/* Event API: */

static PI_EVENTTYPE GET_WINDOWEVENT_TYPE_FROMSDL(PI_EVENT* event, SDL_EventType type) {
    switch (type)
    {
        case SDL_EVENT_WINDOW_SHOWN:
            return PI_EVENT_WINDOW_SHOWN;
        case SDL_EVENT_WINDOW_HIDDEN:
            return PI_EVENT_WINDOW_HIDDEN;
        case SDL_EVENT_WINDOW_MOVED:
            return PI_EVENT_WINDOW_MOVED;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            return PI_EVENT_WINDOW_SIZE;
        case SDL_EVENT_WINDOW_MINIMIZED:
            return PI_EVENT_WINDOW_MINIMIZED;
        case SDL_EVENT_WINDOW_MAXIMIZED:
            return PI_EVENT_WINDOW_MAXIMIZED;
        case SDL_EVENT_WINDOW_MOUSE_ENTER: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            event->window.data1 = (type == SDL_EVENT_WINDOW_MOUSE_ENTER);
            return PI_EVENT_WINDOW_MOUSEHOVER;
        case SDL_EVENT_WINDOW_FOCUS_GAINED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            event->window.data1 = (type == SDL_EVENT_WINDOW_FOCUS_GAINED);
            return PI_EVENT_WINDOW_FOCUS;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return PI_EVENT_WINDOW_CLOSE;
        case SDL_EVENT_WINDOW_DESTROYED:
            return PI_EVENT_WINDOW_DESTROYED;

        default:
            return PI_EVENT_UNKNOWN;
    }
}

static PI_KEYMODS GET_KEYMODS_FROMSDL(SDL_Keymod sdlmod) {
    #define MAP_MOD(modsdl, modpi) mods |= sdlmod & modsdl ? modpi : 0
    PI_KEYMODS mods = PI_KEYMOD_NONE;
    MAP_MOD(SDL_KMOD_LSHIFT, PI_KEYMOD_LSHIFT);
    MAP_MOD(SDL_KMOD_RSHIFT, PI_KEYMOD_RSHIFT);
    MAP_MOD(SDL_KMOD_LCTRL, PI_KEYMOD_LCTRL);
    MAP_MOD(SDL_KMOD_RCTRL, PI_KEYMOD_RCTRL);
    MAP_MOD(SDL_KMOD_LALT, PI_KEYMOD_LALT);
    MAP_MOD(SDL_KMOD_RALT, PI_KEYMOD_RALT);
    MAP_MOD(SDL_KMOD_LGUI, PI_KEYMOD_LWIN);
    MAP_MOD(SDL_KMOD_RGUI, PI_KEYMOD_RWIN);

    MAP_MOD(SDL_KMOD_NUM, PI_KEYMOD_NUM);
    MAP_MOD(SDL_KMOD_CAPS, PI_KEYMOD_CAPS);
    MAP_MOD(SDL_KMOD_MODE, PI_KEYMOD_ALTGR);
    MAP_MOD(SDL_KMOD_SCROLL, PI_KEYMOD_SCROLL);
    #undef MAP_MOD
    return mods;
}

PI_BUTTONSTATE GET_MOUSESTATE_FROMSDL(SDL_MouseButtonFlags state) {
    #define MAP_STATE(statesdl, statepi) s |= (state & statesdl) ? statepi : 0
    PI_BUTTONSTATE s = 0;
    MAP_STATE(SDL_BUTTON_LMASK, PI_MSTATE_LEFT);
    MAP_STATE(SDL_BUTTON_MMASK, PI_MSTATE_MIDDLE);
    MAP_STATE(SDL_BUTTON_RMASK, PI_MSTATE_RIGHT);
    MAP_STATE(SDL_BUTTON_X1MASK, PI_MSTATE_X1);
    MAP_STATE(SDL_BUTTON_X2MASK, PI_MSTATE_X2);
    #undef MAP_STATE
    return s;
}

PI_MOUSEBUTTON GET_BUTTON_FROMSDL(int button) {
    switch (button) {
        case SDL_BUTTON_LEFT:
            return PI_MOUSE_LEFT;
        case SDL_BUTTON_MIDDLE:
            return PI_MOUSE_MIDDLE;
        case SDL_BUTTON_RIGHT:
            return PI_MOUSE_RIGHT;
        case SDL_BUTTON_X1:
            return PI_MOUSE_X1;
        case SDL_BUTTON_X2:
            return PI_MOUSE_X2;
        default:
            return PI_BUTTON_UNKNOWN;
    }
}

bool pi_map_sdl_event(PI_EVENT* event, const SDL_Event* sdlevent)
{
    switch (sdlevent->type)
    {
        case SDL_EVENT_QUIT:
            event->type = PI_EVENT_QUIT;
            break;
        /* Window Events */
        case SDL_EVENT_WINDOW_SHOWN: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_HIDDEN: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MOVED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MINIMIZED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MAXIMIZED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MOUSE_ENTER: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_FOCUS_GAINED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_FOCUS_LOST: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: SDL_FALLTHROUGH;
        case SDL_EVENT_WINDOW_DESTROYED:
            event->window.hwindow = NULL;
            event->window.data1 = sdlevent->window.data1;
            event->window.data2 = sdlevent->window.data2;
            event->type = GET_WINDOWEVENT_TYPE_FROMSDL(event, sdlevent->type);
            break;
        /* Keyboard Events */
        case SDL_EVENT_KEY_DOWN: SDL_FALLTHROUGH;
        case SDL_EVENT_KEY_UP:
            event->type = PI_EVENT_KEY;
            event->key.hwindow = NULL;
            event->key.code = (PI_KEYCODE)sdlevent->key.scancode;
            event->key.mods = GET_KEYMODS_FROMSDL(sdlevent->key.mod);
            event->key.rawscancode = sdlevent->key.raw;
            event->key.character = sdlevent->key.key & (SDLK_SCANCODE_MASK | SDLK_EXTENDED_MASK)
                                ? '\x00' : sdlevent->key.key;
            event->key.press = (sdlevent->key.repeat) ? (PI_PRESS_REPEAT) :
                              (sdlevent->key.down ? PI_PRESS_DOWN : PI_PRESS_UP);
            break;
        /* Mouse Events */
        case SDL_EVENT_MOUSE_MOTION:
            event->type = PI_EVENT_MOUSEMOVE;
            event->motion.hwindow = NULL;
            event->motion.state = GET_MOUSESTATE_FROMSDL(sdlevent->motion.state);
            event->motion.x = sdlevent->motion.x;
            event->motion.y = sdlevent->motion.y;
            event->motion.delta_x = sdlevent->motion.xrel;
            event->motion.delta_y = sdlevent->motion.yrel;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: SDL_FALLTHROUGH;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            event->type = PI_EVENT_MOUSEBUTTON;
            event->button.hwindow = NULL;
            event->button.code = GET_BUTTON_FROMSDL(sdlevent->button.button);
            event->button.clicks = sdlevent->button.clicks;
            event->button.isdown = sdlevent->button.down;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            event->type = PI_EVENT_MOUSEWHEEL;
            event->wheel.hwindow = NULL;
            event->wheel.amnt_x = sdlevent->wheel.x * (sdlevent->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
            event->wheel.amnt_y = sdlevent->wheel.y * (sdlevent->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
            event->wheel.x = sdlevent->wheel.mouse_x;
            event->wheel.x = sdlevent->wheel.mouse_x;
            break;
        default:
            event->type = PI_EVENT_UNKNOWN;
            return false;
    }
    return true;
}

/* Runtime API: */

double pi_get_runtime_dt(PI_HGAME hgame) {
    if (!hgame) { // Returns NAN if invalid parameters are passed.
        return NAN;
    }

    PI_ENGINEDATA* engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->dt_double;
}

/* Scene API: */

PI_HSCENE pi_create_scene(PI_HGAME hgame)
{
    if (!hgame) {
        return NULL;
    }

    PI_ENGINEDATA* engine = PI_ENGINE_FROM_HGAME(hgame);
    struct PI_SCENE_IMPL* hscene = malloc(engine->sizeof_scene);
    if (!hscene) {
        return NULL;
    }

    hscene->engine = engine;
    hscene->boundindex = -1;

    uint8_t* syssceneptr = (uint8_t*)(hscene + 1);
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        hscene->initsubsystems.data[i] = false;
        hscene->subsystems.data[i] = (struct PI_SYSTEM_SCENE*)syssceneptr;
        syssceneptr += engine->system_descs[i].structures.SCENE_size;
    }

    hscene->next = NULL;
    hscene->prev = engine->scene_head;
    engine->scene_head = hscene;
    if (hscene->prev) {
        hscene->prev->next = hscene;
    }
    
    return hscene;
}

void pi_destroy_scene(PI_HSCENE hscene)
{
    if (!hscene)
        return;

    if (hscene->next) {
        PI_ASSERT(hscene->engine->scene_head != hscene);
        PI_ASSERT(hscene->next->prev == hscene);
        hscene->next->prev = hscene->prev;
    }
    else {
        PI_ASSERT(hscene->engine->scene_head == hscene);
        hscene->engine->scene_head = hscene->prev;
    }

    if (hscene->prev) {
        hscene->prev->next = hscene->next;
    }

    for (size_t i = 0; i < SYSTEM_COUNT; i++)
    {
        if (!hscene->initsubsystems.data[i])
            continue;
        
        void* hsystem = hscene->subsystems.data[i];
        PI_SYSTEM_DESC* psystemdesc = &hscene->engine->system_descs[i];

        psystemdesc->callbacks.pfn_scene_deinit(hsystem, hscene->subsystems.data[i]);
    }

    // TODO: Change allocator function calling names
    free(hscene);
}


PI_EXTERN_C bool pi_enable_scene(PI_HSCENE hscene, int _index)
{
    if (!hscene) {
        return false;
    }

    if (_index == -1) {
        _index = hscene->engine->bound_scene_last;
    }
    else if (_index < 0) {
        return false;
    }

    unsigned index = _index;
    if (index >= hscene->engine->bound_scene_capacity)
    {
        unsigned newcap = 1 + index + hscene->engine->bound_scene_capacity;
        PI_HSCENE* newsceneptr = malloc(sizeof(PI_HSCENE) * newcap);
        if (!newsceneptr) {
            return false;
        }

        memcpy(newsceneptr, hscene->engine->bound_scenes, sizeof(PI_HSCENE) * hscene->engine->bound_scene_last);
        free(hscene->engine->bound_scenes);
        hscene->engine->bound_scenes = newsceneptr;
        hscene->engine->bound_scene_capacity = newcap;
    }

    if (index >= hscene->engine->bound_scene_last) {
        memset(hscene->engine->bound_scenes + hscene->engine->bound_scene_last, 0x00, index);
        hscene->engine->bound_scene_last = index + 1;
    }
    else if (hscene->engine->bound_scenes[index]) {
        return false;
    }

    hscene->boundindex = index;
    hscene->engine->bound_scenes[index] = hscene;

    return true;
}
PI_EXTERN_C void pi_disable_scene(PI_HSCENE hscene)
{
    if (!hscene)
        return;

    if (hscene->boundindex == -1)
        return;

    PI_ASSERT(hscene->boundindex > 0);
    PI_ASSERT((unsigned)hscene->boundindex < hscene->engine->bound_scene_last);

    unsigned lastindex = hscene->engine->bound_scene_last - 1;
    if (hscene->boundindex == lastindex) {
        hscene->engine->bound_scene_last = lastindex;
    }
    hscene->engine->bound_scenes[hscene->boundindex] = NULL;
}

PI_EXTERN_C int pi_enabled_scene_index(PI_HSCENE hscene) {
    if (!hscene) {
        return -1;
    }

    return hscene->boundindex;
}

PI_EXTERN_C PI_HSCENE pi_enabled_scene_byindex(PI_HGAME hgame, int index) {
    if (!hgame) {
        return NULL;
    }
    if (index < 0) {
        return NULL;
    }

    unsigned uindex = index;
    PI_ENGINEDATA* engine = PI_ENGINE_FROM_HGAME(hgame);
    if (uindex >= engine->bound_scene_last) {
        return NULL;
    }
    PI_ASSERT(engine->bound_scenes != NULL);
    return engine->bound_scenes[uindex];
}
