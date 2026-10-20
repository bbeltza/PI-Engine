#ifndef PI_INTERNAL_H
#define PI_INTERNAL_H

#include <piengine/pi_main.h>
#include <piengine/pi_core.h>
#include <piengine/pi_debug.h>

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

typedef struct PI_EVENT_HOOK PI_EVENT_HOOK;

struct PI_WND_IMPL
{
    PI_ENGINEDATA* engine;
    PI_WND_IMPL* p_next;

    SDL_Window* sdlwnd;
    uint32_t id;
};

struct PI_ENGINEDATA {
    int running;
    PI_APPCALLBACKS appcallbacks;

    PI_WND_IMPL main_wnd;
    PI_WND_IMPL* wnd_head;

    Uint64 dt_prev;
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

    PI_EVENT_HOOK* eventhooks;
    unsigned eventhook_count;
    unsigned eventhook_capacity;
};

struct PI_EVENT_HOOK {
    PI_PFN_EVENTHOOK pfn_callback;
    void* p_userdata;
};

/* Core engine implementation definitions */

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

  /* Internal event hook API for subsystems to use */
PI_EXTERN_C bool pi_add_engine_event_hook(PI_ENGINEDATA* engine, const PI_EVENT_HOOK* p_hook);
PI_EXTERN_C bool pi_remove_engine_event_hook(PI_ENGINEDATA* engine, const PI_EVENT_HOOK* p_hook);

#define PI_SDLPROP_WINDOW_HWND_PTR "PI.window.hwnd_ptr"
#define PI_SDLPROP_WINDOW_HGAME_PTR "PI.window.hgame_ptr"

PI_EXTERN_C SDL_PropertiesID pi_init_engine_window(PI_ENGINEDATA* engine, PI_WND_IMPL* wnd, const PI_WINDOW_PARAMETERS* parameters);
PI_EXTERN_C void pi_deinit_engine_window(PI_WND_IMPL* wnd);

#ifdef PI_CPP
    static_assert(sizeof(PI_ENGINEDATA::subsystems)/sizeof(void*) == SYSTEM_COUNT, "Subsystem count mismatch?");
    static_assert(sizeof(PI_SCENE_IMPL::subsystems)/sizeof(void*) == SYSTEM_COUNT, "Subsystem count mismatch?");
    static_assert(sizeof(PI_SCENE_IMPL::initsubsystems)/sizeof(bool) == SYSTEM_COUNT, "Subsystem count mismatch?");
#endif

PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_RENDER;
PI_EXTERN_C const PI_INIT_SYSTEM _PI_INIT_PHYSICS;

#define PI_ENGINE_FROM_HGAME(hgame) ( *(PI_ENGINEDATA**) (((void**)(hgame)) - 1) )

#endif
