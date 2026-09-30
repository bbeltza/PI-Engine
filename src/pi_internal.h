#ifndef PI_INTERNAL_H
#define PI_INTERNAL_H

#include <piengine/pi_main.h>
#include <piengine/pi_core.h>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

/* Core engine implementation definitions */

struct PI_WND_IMPL {
    SDL_Window* sdlwnd;
    // More stuff to come :eyes:
};

/* Internal engine definitions */

typedef struct PI_SYSTEM_RENDER PI_SYSTEM_RENDER;
typedef struct PI_SYSTEM_PHYSICS PI_SYSTEM_PHYSICS;

typedef struct PI_ENGINEDATA {
    int running;
    PI_APPCALLBACKS callbacks;

    SDL_Window* sdlwnd;

    Uint64 dt_value;
    double dt_double;

    union {
        struct {
            PI_SYSTEM_RENDER* render;
            PI_SYSTEM_PHYSICS* physics;
        };
        void* data[2];
    } subsystems;
} PI_ENGINEDATA;


typedef bool (*PI_PFN_OPEN_SYSTEM)(size_t* allocsize);
typedef bool (*PI_PFN_START_SYSTEM)(void* hsystem, PI_ENGINEDATA* hengine);
typedef void (*PI_PFN_CLOSE_SYSTEM)(void* hsystem);
typedef void (*PI_PFN_UPDATE_SYSTEM)(void* hsystem);
typedef struct PI_INIT_SYSTEM {
    PI_PFN_OPEN_SYSTEM open;
    PI_PFN_START_SYSTEM start;
    PI_PFN_CLOSE_SYSTEM close;

    PI_PFN_UPDATE_SYSTEM update;
} PI_INIT_SYSTEM;

PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_RENDER;
PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_PHYSICS;

#define PI_ENGINE_FROM_HGAME(hgame) ( *(PI_ENGINEDATA**) (((void**)(hgame)) - 1) )

#endif
