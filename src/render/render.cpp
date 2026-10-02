#include "render_internal.hpp"
#include <piengine/pi_math.h>

PI_EXTERN_C const PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10;

static const PI_BACKEND_RENDER* backends[] = {
    #if PI_HAVE_D3D10
        &_PI_BACKEND_RENDER_D3D10
    #endif
};

static constexpr size_t NUM_BACKENDS = PI_ARRAYSIZE(backends);
static const PI_BACKEND_RENDER* chosen_backend;

const PI_INIT_SYSTEM _PI_INIT_RENDER = {
    [](size_t* allocsize) noexcept -> bool {
        *allocsize = sizeof(PI_SYSTEM_RENDER);
        for (size_t i = 0; i < NUM_BACKENDS; i++) {
            auto backend = backends[i];
            PI_ASSERT(backend->pfn_calcsize != NULL);
            PI_ASSERT(backend->pfn_entry != NULL);

            size_t backendsize = backend->pfn_calcsize();
            if (backendsize) {
                *allocsize += backendsize;
                chosen_backend = backend;
                return true;
            }
        }

        *allocsize = 0;
        return false;
    },
    [](void* hsystem, PI_ENGINEDATA* hengine) -> bool {
        bool succeed = false;
        auto renderer = new (hsystem) PI_SYSTEM_RENDER{ hengine, succeed };
        return succeed;
    },
    [](void* hsystem) -> void {
        auto renderer = static_cast<PI_SYSTEM_RENDER*>(hsystem);
        renderer->~PI_SYSTEM_RENDER();
    },
    [](void* hsystem) -> void {
        auto renderer = static_cast<PI_SYSTEM_RENDER*>(hsystem);
        renderer->update();
    }
};

PI_SYSTEM_RENDER::PI_SYSTEM_RENDER(PI_ENGINEDATA* hengine, bool& succeed) noexcept:
    m_sdlwnd(hengine->sdlwnd)
{
    PI_BACKEND_RENDER_DESC backenddesc = {
        hengine->sdlwnd
    };

    auto backend = chosen_backend;
    PI_ASSERT(backend);

    std::printf("Initializing backend \"%s\"...\n", backend->name);
    PI_HRENDERBACKEND hbackend = get_backend();

    if (!backend->pfn_entry(hbackend, &backenddesc, &this->backend)) {
        return;
    }

    SDL_AddEventWatch(_s_event_watch, this);
    {
        int w, h;
        if (SDL_GetWindowSizeInPixels(m_sdlwnd, &w, &h)) {
            m_configureinfo.w = w;
            m_configureinfo.h = h;
            this->backend.callbacks.pfn_configure(hbackend, &m_configureinfo);
        }
    }
    succeed = true;
}

PI_SYSTEM_RENDER::~PI_SYSTEM_RENDER() noexcept
{
    SDL_RemoveEventWatch(_s_event_watch, this);

    m_objects.scenes.clear();
    m_objects.meshes.clear();

    backend.callbacks.pfn_close(get_backend());
}

bool PI_SYSTEM_RENDER::_s_event_watch(void* userdata, SDL_Event* event) noexcept
{
    auto renderer = static_cast<PI_SYSTEM_RENDER*>(userdata);
    PI_ASSERT(renderer != NULL);

    SDL_Window* sdlwnd = SDL_GetWindowFromEvent(event);
    if (renderer->m_sdlwnd != sdlwnd) {
        return true;
    }

    switch (event->type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            if (!event->window.data1 || !event->window.data2) {
                break;
            }
            if (renderer->m_configureinfo.w == event->window.data1 &&
                renderer->m_configureinfo.h == event->window.data2) {
                break;
            }

            renderer->m_configureinfo.w = event->window.data1;
            renderer->m_configureinfo.h = event->window.data2;
            renderer->backend.callbacks.pfn_configure(renderer->get_backend(), &renderer->m_configureinfo);
            break;
        default:
            break;
    }

    return true;
}

PI_EXTERN_C PI_HMESH pi_create_mesh_indexed(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) {
    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->subsystems.render->create_mesh_indexed(
            num_vertices, vertices,
            num_indices, indices, index_type);
}

PI_EXTERN_C PI_HMESH pi_create_mesh(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[]) {
    return pi_create_mesh_indexed(hgame, num_vertices, vertices, 0, NULL, PI_INDEX_UNKNOWN);
}

PI_EXTERN_C void pi_destroy_mesh(PI_HMESH hmesh) {
    if (!hmesh) {
        return;
    }
    hmesh->renderer->destroy_mesh(hmesh);
}

PI_EXTERN_C PI_HSCENE pi_create_scene(PI_HGAME hgame, bool set_current) {
    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    auto scn = engine->subsystems.render->create_scene();
    if (scn && set_current) {
        engine->subsystems.render->set_current_scene(scn);
    }

    return scn;
}

PI_EXTERN_C void pi_release_scene(PI_HSCENE hscene) {
    if (!hscene) {
        return;
    }
    hscene->renderer->release_scene(hscene);
}

#define HSCENE_ERROR reinterpret_cast<PI_HSCENE>(UINTPTR_MAX)

PI_EXTERN_C PI_HSCENE pi_set_current_scene(PI_HGAME hgame, PI_HSCENE hscene) {
    auto engine = hgame ? PI_ENGINE_FROM_HGAME(hgame) : NULL;
    if (!engine || (hscene && engine->subsystems.render != hscene->renderer)) { // `hscene` can beNULL, used to unbind the scene from the game
        return HSCENE_ERROR;
    }

    return engine->subsystems.render->set_current_scene(hscene);
}
PI_EXTERN_C PI_HSCENE pi_get_current_scene(PI_HGAME hgame) {
    if (!hgame) {
        return HSCENE_ERROR;
    }

    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->subsystems.render->get_current_scene();
}

PI_EXTERN_C bool pi_set_scene_clear_color(PI_HSCENE hscene, float r, float g, float b, float a) {
    PI_ASSERT(hscene != NULL);

    hscene->clear_color = PI_VECTOR4{ r, g, b, a };
    return true;
}
PI_EXTERN_C bool pi_set_scene_projection(PI_HSCENE hscene, const PI_MATRIX_4X4* mproj) {
    /* Meh just make a memcpy :D */
    hscene->projection = *mproj;
    hscene->dirty.viewproj = true; // Here we need to notify update state since the view/projection is to be updated inside a buffer (managed by the backend (D3D or GL....))
    return true;
}
PI_EXTERN_C bool pi_set_scene_view(PI_HSCENE hscene, const PI_MATRIX_4X4* mview) {
    hscene->view = *mview;
    hscene->dirty.viewproj = true;
    return true;
}

PI_EXTERN_C PI_HINSTANCE pi_add_scene_mesh(PI_HSCENE hscene, PI_HMESH hmesh, const PI_INSTANCE* instance) {
    if (!hscene) {
        return NULL;
    }
    if (!hmesh) {
        return NULL;
    }
    if (hmesh->renderer != hscene->renderer) {
        return NULL;
    }

    static const PI_INSTANCE INSTANCE_DEFAULT = {
        pi_identity_4x4(),
        { 1.0f, 1.0f, 1.0f, 1.0f }
    };
    if (!instance) {
        instance = &INSTANCE_DEFAULT;
    }

    return hscene->add_scene_mesh(hmesh, instance);
}
PI_EXTERN_C void pi_remove_scene_mesh(PI_HINSTANCE hmeshinst) {
    if (!hmeshinst) {
        return;
    }
    hmeshinst->instbuffer->instlist.remove(*hmeshinst);
}
PI_EXTERN_C bool pi_update_mesh_instance(PI_HINSTANCE hmeshinst, const PI_INSTANCE* instance) {
    return hmeshinst->update_instance(instance);
}
