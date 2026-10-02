#include <piengine/pi_main.h>
#include "../pi_internal.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_timer.h>

#define CHECK_CALLBACK(c) PI_ASSERT(c != NULL)

#include <stdio.h>

#define PI_LOG(fmt, ...) printf(fmt "\n",##__VA_ARGS__)

static bool _event_watch(void* userdata, SDL_Event* event);

PI_EXTERN_C int pi_run_exv(const PI_APPINFO* appinfo, va_list args)
{
    if (!appinfo) {
        return -1;
    }

    CHECK_CALLBACK(appinfo->callbacks->appallocdata);
    CHECK_CALLBACK(appinfo->callbacks->appstart);
    CHECK_CALLBACK(appinfo->callbacks->appclose);
    CHECK_CALLBACK(appinfo->callbacks->appevent);
    CHECK_CALLBACK(appinfo->callbacks->appupdate);

    size_t gamesize = 0;
    bool callres;
    callres = appinfo->callbacks->appallocdata(&gamesize);
    if (!callres) { // TODO: Handle failure
        return -1;
    }

    /* Open systems */
    static const PI_INIT_SYSTEM* systems[] = {
        &_PI_INIT_RENDER
    };

    #define SYSTEM_COUNT PI_ARRAYSIZE(systems)
    size_t system_sizes[SYSTEM_COUNT] = { 0 };
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        const PI_INIT_SYSTEM* s = systems[i];
        CHECK_CALLBACK(s->open);
    
        if (!s->open(&system_sizes[i])) { // TODO: Handle failure
            return -1;
        }

        PI_ASSERT(system_sizes[i] != 0);
    }

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        return -1;
    }

    Uint64 dt_start = SDL_GetTicksNS();
    size_t engine_size = sizeof(PI_ENGINEDATA) + sizeof(PI_ENGINEDATA*); // Include a pointer right behind the given HGAME struct
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        engine_size += system_sizes[i];
    }

    engine_size = PI_ALIGN(engine_size, PI_DEFAULT_STRUCT_ALIGNMENT);
    gamesize = PI_ALIGN(gamesize, PI_DEFAULT_STRUCT_ALIGNMENT);

// Initializing PI_ENGINEDATA
    PI_ENGINEDATA* engine = calloc(1, engine_size + gamesize);
    PI_ASSERT(engine != NULL);

    engine->running = true;
    engine->callbacks = *appinfo->callbacks;
    engine->sdlwnd = SDL_CreateWindow(appinfo->name, 800, 600, SDL_WINDOW_RESIZABLE);
    if (!engine->sdlwnd) {
        goto QUIT_FAIL;
    }


    char* last_system = (char*)(engine) + sizeof(PI_ENGINEDATA);
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        if (!systems[i]->start((void*)last_system, engine)) {
            // TODO: Handle failure better
            printf("SYSTEM INITIALIZATION (at index: %zu) FAILED...\n", i);
            goto QUIT_FAIL;
        }

        engine->subsystems.data[i] = (void*)last_system;
        last_system += system_sizes[i];
    }

    PI_HGAME hgame = (PI_HGAME)((char*)(engine) + engine_size);
    PI_ENGINE_FROM_HGAME(hgame) = engine;
    callres = engine->callbacks.appstart(hgame, args);
    if (!callres) { // TODO: Handle failure
        goto QUIT_FAIL;
    }

    { // Process existing events from the queue added before AddEventWatch gets called.
        SDL_PumpEvents();
        int currevents = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);

        SDL_Event* evs = SDL_stack_alloc(SDL_Event, currevents);
        if (SDL_PeepEvents(evs, currevents, SDL_PEEKEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) == currevents) {
            for (int i = 0; i < currevents; i++) {
                _event_watch(hgame, &evs[i]);
            }
        }
        else {
            printf("SDL_PeepEvents failed: \"%s\"", SDL_GetError());
        }
        SDL_stack_free(evs);
    }

    SDL_AddEventWatch(_event_watch, hgame);
    int run = 1;
    SDL_Event ev;
    Uint64 rticks;
    while(run) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type)
            {
                case SDL_EVENT_QUIT:
                    run = 0;
                    break;
            }
        }
        
        rticks = SDL_GetTicksNS();
        engine->dt_value = rticks - dt_start;
        engine->dt_double = SDL_NS_TO_SECONDS((double)engine->dt_value);
        dt_start = rticks;
        

        if (!engine->callbacks.appupdate(hgame)) {
            continue;
        }

        for (size_t i = 0; i < SYSTEM_COUNT; i++) {
            void* hsystem = engine->subsystems.data[i];
            systems[i]->update(hsystem);
        }
    }

    /* Cleanup all systems */
        engine->callbacks.appclose(hgame);
        
        for (size_t i = 0; i < SYSTEM_COUNT; i++) {
            void* hsystem = engine->subsystems.data[i];
            systems[i]->close(hsystem);
        }
    /**/

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    free(engine);
    return 0;

QUIT_FAIL:
    SDL_Quit();
    free(engine);
    return -1;
}

//

static bool _event_watch(void* userdata, SDL_Event* event) {
    PI_HGAME hgame = (PI_HGAME)userdata;
    PI_ASSERT(hgame != NULL);

    PI_ENGINEDATA* engine = PI_ENGINE_FROM_HGAME(hgame);
    PI_EVENT pievent;
    pi_map_sdl_event(&pievent, event);
    return engine->callbacks.appevent(hgame, &pievent);
}
