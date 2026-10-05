#ifndef PI_INTERNAL_H
#define PI_INTERNAL_H

#include <piengine/pi_main.h>
#include <piengine/pi_core.h>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

typedef struct PI_ENGINEDATA PI_ENGINEDATA;
typedef struct PI_SYSTEM_SCENE *PI_HSYSTEMSCENE;

/* Subsystem internal definitions */

typedef void (*PI_PFN_SYSTEM_CLOSE)(void* hsystem);
typedef void (*PI_PFN_SYSTEM_SCENE_DEINIT)(void* hsystem, PI_HSYSTEMSCENE hscene);
typedef void (*PI_PFN_SYSTEM_UPDATE)(void* hsystem, PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount);
typedef struct PI_SYSTEM_CALLBACKS {
    PI_PFN_SYSTEM_CLOSE pfn_close;
    PI_PFN_SYSTEM_SCENE_DEINIT pfn_scene_deinit; // There's no pfn_sceneinit function! It's up to the system to initialize the scene with its corresponding pi_init_xxx_scene call!!!
    
    PI_PFN_SYSTEM_UPDATE pfn_preupdate;
    PI_PFN_SYSTEM_UPDATE pfn_postupdate;
} PI_SYSTEM_CALLBACKS;

typedef struct PI_SYSTEM_STRUCTURES {
    size_t SCENE_size;
} PI_SYSTEM_STRUCTURES;

typedef struct PI_SYSTEM_DESC {
    PI_SYSTEM_CALLBACKS callbacks;
    PI_SYSTEM_STRUCTURES structures;
} PI_SYSTEM_DESC;

typedef size_t (*PI_PFN_SYSTEM_OPEN)();
typedef bool (*PI_PFN_SYSTEM_START)(void* hsystem, PI_ENGINEDATA* hengine, PI_SYSTEM_DESC* pdesc);
typedef struct PI_INIT_SYSTEM {
    PI_PFN_SYSTEM_OPEN pfn_open;
    PI_PFN_SYSTEM_START pfn_start;
} PI_INIT_SYSTEM;

/* Internal engine definitions */

#define SYSTEM_COUNT 2

typedef struct PI_SYSTEM_RENDER PI_SYSTEM_RENDER;
typedef struct PI_SYSTEM_PHYSICS PI_SYSTEM_PHYSICS;

struct PI_ENGINEDATA {
    int running;
    PI_APPCALLBACKS appcallbacks;

    SDL_Window* sdlwnd;

    Uint64 dt_value;
    double dt_double;


    struct PI_SCENE_IMPL* scene_head;
    // struct PI_SCENE_IMPL* scene_tail;

    PI_HSCENE* bound_scenes;
    unsigned bound_scene_capacity;
    unsigned bound_scene_last;

    size_t sizeof_scene;
    PI_SYSTEM_DESC system_descs[SYSTEM_COUNT];

    union {
        struct {
            PI_SYSTEM_RENDER* render;
            PI_SYSTEM_PHYSICS* physics;
        };
        void* data[SYSTEM_COUNT];
    } subsystems;
};

/* Core engine implementation definitions */

struct PI_WND_IMPL {
    SDL_Window* sdlwnd;
    // More stuff to come :eyes:
};

struct PI_SCENE_IMPL {
    struct PI_SCENE_IMPL* next;
    struct PI_SCENE_IMPL* prev;
    PI_ENGINEDATA* engine; 

    int boundindex;

    union {
        struct {
            bool render;
            bool physics;
        };
        bool data[SYSTEM_COUNT];
    } initsubsystems;

    union {
        struct {
            struct PI_SYSTEM_SCENE* render;
            struct PI_SYSTEM_SCENE* physics;
        };
        struct PI_SYSTEM_SCENE* data[SYSTEM_COUNT];
    } subsystems;
};

#ifdef PI_CPP
    static_assert(sizeof(PI_ENGINEDATA::subsystems)/sizeof(void*) == SYSTEM_COUNT, "Subsystem count mismatch?");
    static_assert(sizeof(PI_SCENE_IMPL::subsystems)/sizeof(void*) == SYSTEM_COUNT, "Subsystem count mismatch?");
    static_assert(sizeof(PI_SCENE_IMPL::initsubsystems)/sizeof(bool) == SYSTEM_COUNT, "Subsystem count mismatch?");
#endif

PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_RENDER;
PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_PHYSICS;

#define PI_ENGINE_FROM_HGAME(hgame) ( *(PI_ENGINEDATA**) (((void**)(hgame)) - 1) )

#endif
