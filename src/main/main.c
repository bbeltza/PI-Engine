#include <piengine/pi_main.h>
#include "../pi_internal.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_assert.h>

#define CHECK_CALLBACK(c) SDL_assert(c != NULL)

#include <stdio.h>

#define PI_LOG(fmt, ...) printf(fmt "\n",##__VA_ARGS__)

PI_EXTERN_C int pi_run_exv(const PI_APPINFO* appinfo, va_list args)
{
    if (!appinfo) {
        return -1;
    }

    CHECK_CALLBACK(appinfo->callbacks->appallocdata);
    CHECK_CALLBACK(appinfo->callbacks->appstart);
    CHECK_CALLBACK(appinfo->callbacks->appclose);
    CHECK_CALLBACK(appinfo->callbacks->appupdate);
    CHECK_CALLBACK(appinfo->callbacks->apprender);

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

    #define SYSTEM_COUNT SDL_arraysize(systems)
    size_t system_sizes[SYSTEM_COUNT] = { 0 };
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        const PI_INIT_SYSTEM* s = systems[i];
        CHECK_CALLBACK(s->open);
    
        if (!s->open(&system_sizes[i])) { // TODO: Handle failure
            return -1;
        }

        SDL_assert(system_sizes[i] != 0);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    size_t engine_size = sizeof(PI_ENGINEDATA) + sizeof(PI_ENGINEDATA*); // Include a pointer right behind the given HGAME struct
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        engine_size += system_sizes[i];
    }

    engine_size = PI_ALIGN(engine_size, PI_DEFAULT_STRUCT_ALIGNMENT);
    gamesize = PI_ALIGN(gamesize, PI_DEFAULT_STRUCT_ALIGNMENT);

// Initializing PI_ENGINEDATA
    PI_ENGINEDATA* engine = calloc(1, engine_size + gamesize);
    SDL_assert(engine != NULL);

    engine->running = true;
    engine->callbacks = *appinfo->callbacks;
    engine->sdlwnd = SDL_CreateWindow(appinfo->name, 800, 600, 0);
    if (!engine->sdlwnd) {
        goto QUIT_FAIL;
    }

    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        void* hsystem = (char*)(engine) + system_sizes[i];
        systems[i]->start(engine, hsystem);
        // TODO: Handle failure

        engine->subsystems.data[i] = hsystem;
    }

    PI_HGAME hgame = (PI_HGAME)((char*)(engine) + engine_size);
    PI_ENGINE_FROM_HGAME(hgame) = engine;
    callres = engine->callbacks.appstart(hgame, args);
    if (!callres) { // TODO: Handle failure
        goto QUIT_FAIL;
    }

    int run = 1;
    SDL_Event ev;
    while(run) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type)
            {
                case SDL_EVENT_QUIT:
                    run = 0;
                    break;
            }
        }

        if (!engine->callbacks.appupdate(hgame)) {
            continue;
        }

        if (!engine->callbacks.apprender(hgame)) {
            continue;
        }
    }

    SDL_DestroyWindow(engine->sdlwnd);
    SDL_Quit();
    free(engine);
    return 0;

QUIT_FAIL:
    SDL_Quit();
    free(engine);
    return -1;
}
