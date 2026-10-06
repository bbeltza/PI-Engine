#include <piengine/pi_main.h>
#include "../pi_internal.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_timer.h>

#define CHECK_CALLBACK(c) PI_ASSERT(c != NULL)

#include <stdio.h>

#define PI_LOG(fmt, ...) printf(fmt "\n",##__VA_ARGS__)

static bool _event_watch(void* userdata, SDL_Event* event);
static void _update_scenes(PI_ENGINEDATA* engine, PI_HGAME hgame);

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
        &_PI_INIT_RENDER,
        &_PI_INIT_PHYSICS
    };

    size_t system_sizes[SYSTEM_COUNT] = { 0 };
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        const PI_INIT_SYSTEM* s = systems[i];
        CHECK_CALLBACK(s->pfn_open);
        CHECK_CALLBACK(s->pfn_start);
    
        size_t size = s->pfn_open();
        if (!size) { // TODO: Handle failure
            return -1;
        }

        system_sizes[i] = size;
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

    SDL_WindowFlags windowflags = SDL_WINDOW_RESIZABLE;
    #if PI_HAVE_GL
        windowflags |= SDL_WINDOW_OPENGL;
    #endif
    
    engine->running = true;
    engine->appcallbacks = *appinfo->callbacks;
    engine->sdlwnd = SDL_CreateWindow(appinfo->name, 800, 600, windowflags);

    if (!engine->sdlwnd) {
        goto QUIT_FAIL;
    }

    engine->sizeof_scene = sizeof(struct PI_SCENE_IMPL);
    char* last_system = (char*)(engine) + sizeof(PI_ENGINEDATA);
    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        PI_SYSTEM_DESC* pdesc = &engine->system_descs[i];
        if (!systems[i]->pfn_start((void*)last_system, engine, pdesc)) {
            // TODO: Handle failure better
            printf("SYSTEM INITIALIZATION (at index: %zu) FAILED...\n", i);
            goto QUIT_FAIL;
        }
        
        CHECK_CALLBACK(pdesc->callbacks.pfn_close);
        CHECK_CALLBACK(pdesc->callbacks.pfn_scene_deinit);
        PI_ASSERT(pdesc->structures.SCENE_size != 0);

        engine->sizeof_scene += pdesc->structures.SCENE_size;
        engine->subsystems.data[i] = (void*)last_system;
        last_system += system_sizes[i];
    }

    PI_HGAME hgame = (PI_HGAME)((char*)(engine) + engine_size);
    PI_ENGINE_FROM_HGAME(hgame) = engine;
    callres = engine->appcallbacks.appstart(hgame, args);
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
        
        _update_scenes(engine, hgame);
    }

    /* Cleanup everything... */
        engine->appcallbacks.appclose(hgame);
        
        for (struct PI_SCENE_IMPL* cur = engine->scene_head; cur != NULL;) {
            struct PI_SCENE_IMPL* next = cur->next;
            pi_destroy_scene(cur);
            cur = next;
        }
    
        free(engine->bound_scenes);

        for (size_t i = 0; i < SYSTEM_COUNT; i++) {
            void* hsystem = engine->subsystems.data[i];
            const PI_SYSTEM_CALLBACKS* pcallbacks = &engine->system_descs[i].callbacks;
            pcallbacks->pfn_close(hsystem);
        }
    /**/

    SDL_RemoveEventWatch(_event_watch, hgame);
    free(engine->eventhooks);
    engine->eventhooks = NULL;
    engine->eventhook_count = 0;
    engine->eventhook_capacity = 0;

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

    if (pievent.type == PI_EVENT_TOUCH) {
        int w = 1;
        int h = 1;
        SDL_GetWindowSizeInPixels(engine->sdlwnd, &w, &h);
        pievent.touch.x *= w;
        pievent.touch.y *= h;
        pievent.touch.delta_x *= w;
        pievent.touch.delta_y *= h;
    }

    for (unsigned i = 0; i < engine->eventhook_count; i++) {
        const PI_EVENT_HOOK* hook = &engine->eventhooks[i];
        if (!hook->pfn_callback)
            continue;
        hook->pfn_callback(hook->p_userdata, &pievent);
    }

    return engine->appcallbacks.appevent(hgame, &pievent);
}

//

static void _update_scenes(PI_ENGINEDATA* engine, PI_HGAME hgame)
{
    unsigned scenecount = engine->bound_scene_last;
    unsigned tmpscenecounts[SYSTEM_COUNT];
    PI_HSYSTEMSCENE* tmpscenes = SDL_stack_alloc(PI_HSYSTEMSCENE, SYSTEM_COUNT * scenecount);

    //if (tmpscenes)
    {
        for (size_t i = 0; i < SYSTEM_COUNT; i++) {
            const PI_SYSTEM_CALLBACKS* pcallbacks = &engine->system_descs[i].callbacks;
            if (!pcallbacks->pfn_preupdate && !pcallbacks->pfn_postupdate)
                continue;
            
            unsigned countacc = 0;
            PI_HSYSTEMSCENE* const tmpscenestart = &tmpscenes[i * scenecount];
            for (unsigned j = 0; j < scenecount; j++) {
                PI_HSCENE hscene = engine->bound_scenes[j];
                if (!hscene)
                    continue;
                if (!hscene->initsubsystems.data[i])
                    continue;
                
                tmpscenestart[countacc] = hscene->subsystems.data[i];
                countacc++;
            }
            tmpscenecounts[i] = countacc;
        }
    }

    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        void* hsystem = engine->subsystems.data[i];
        const PI_SYSTEM_CALLBACKS* pcallbacks = &engine->system_descs[i].callbacks;
        if (pcallbacks->pfn_preupdate)
            pcallbacks->pfn_preupdate(hsystem,
                    tmpscenes + i*scenecount,
                    tmpscenecounts[i]
                );
    }

    if (!engine->appcallbacks.appupdate(hgame)) {
        goto END_UPDATE;
    }

    for (size_t i = 0; i < SYSTEM_COUNT; i++) {
        void* hsystem = engine->subsystems.data[i];
        const PI_SYSTEM_CALLBACKS* pcallbacks = &engine->system_descs[i].callbacks;
        if (pcallbacks->pfn_postupdate)
            pcallbacks->pfn_postupdate(hsystem,
                    tmpscenes + i*scenecount,
                    tmpscenecounts[i]
                );
    }

    END_UPDATE:
        SDL_stack_free(tmpscenes);
}
