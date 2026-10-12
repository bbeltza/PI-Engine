#include "render_internal.hpp"
#include <piengine/pi_math.h>

PI_EXTERN_C const PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10;
PI_EXTERN_C const PI_BACKEND_RENDER _PI_BACKEND_RENDER_GL;

//#undef PI_HAVE_D3D10

static const PI_BACKEND_RENDER* backends[] = {
    #if PI_HAVE_D3D10
        &_PI_BACKEND_RENDER_D3D10,
    #endif
    #if PI_HAVE_GL
        &_PI_BACKEND_RENDER_GL,
    #endif
};

static constexpr size_t NUM_BACKENDS = PI_ARRAYSIZE(backends);
static const PI_BACKEND_RENDER* chosen_backend;

const PI_INIT_SYSTEM _PI_INIT_RENDER = {
    []() noexcept -> size_t {
        size_t size = sizeof(PI_SYSTEM_RENDER);
        for (size_t i = 0; i < NUM_BACKENDS; i++) {
            auto backend = backends[i];
            PI_ASSERT(backend->pfn_calcsize != NULL);
            PI_ASSERT(backend->pfn_entry != NULL);

            size_t backendsize = backend->pfn_calcsize();
            if (backendsize) {
                size += backendsize;
                chosen_backend = backend;
                return size;
            }
        }
        return 0;
    },
    [](void* hsystem, PI_ENGINEDATA* hengine, PI_SYSTEM_DESC* pdesc) noexcept -> bool {
        bool succeed = false;
        auto renderer = new (hsystem) PI_SYSTEM_RENDER{ hengine, succeed };
        
        pdesc->structures.SCENE_size = sizeof(PI_SYSTEM_SCENE);
        pdesc->structures.SCENE_size += renderer->backend.structures.SCENE_size;

        pdesc->callbacks.pfn_close = [](void* hsystem) noexcept -> void {
            auto renderer = static_cast<PI_SYSTEM_RENDER*>(hsystem);
            renderer->~PI_SYSTEM_RENDER();
        };
        pdesc->callbacks.pfn_scene_deinit = [](void* hsystem, PI_HSYSTEMSCENE hscene) noexcept -> void {
            hscene->~PI_SYSTEM_SCENE();
        };
        pdesc->callbacks.pfn_postupdate = [](void* hsystem, PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount) noexcept -> void {
            auto renderer = static_cast<PI_SYSTEM_RENDER*>(hsystem);
            renderer->update(hsysscenes, scenecount); // Rename it to "post_update"?
        };

        return succeed;
    }
};

template <typename Alloc>
using propagate_on_container_move_assignment = typename std::allocator_traits<Alloc>::propagate_on_container_move_assignment;

PI_SYSTEM_RENDER::PI_SYSTEM_RENDER(PI_ENGINEDATA* hengine, bool& succeed) noexcept:
    m_sdlwnd(hengine->sdlwnd)
{
    PI_BACKEND_RENDER_DESC backenddesc = {
        hengine->sdlwnd
    };

    auto backend = chosen_backend;
    PI_ASSERT(backend);

    //std::printf("Initializing backend \"%s\"...\n", backend->name);
    PI_HRENDERBACKEND hbackend = get_backend();

    if (!backend->pfn_entry(hbackend, &backenddesc, &this->backend)) {
        return;
    }

    m_objects.worldenvs = WORLDENV_LIST{ WORLDENV_ALLOC{ this->backend.structures.WORLDENV_size } };
    m_objects.meshes = MESH_LIST{ MESH_ALLOC{ this->backend.structures.MESH_size } };

    #define PROPAGATE_ERROR(n) "propagate_on_container_move_assignment for \"" #n "\" is not present."
    static_assert(propagate_on_container_move_assignment<WORLDENV_ALLOC>::value == true, PROPAGATE_ERROR(WORLDENV_ALLOC));
    static_assert(propagate_on_container_move_assignment<MESH_ALLOC>::value == true, PROPAGATE_ERROR(MESH_LIST));

    PI_ASSERT(m_objects.worldenvs.get_allocator().backend_size() > 0);
    PI_ASSERT(m_objects.meshes.get_allocator().backend_size() > 0);

    PI_EVENT_HOOK hook = {
        _s_event_hook,
        this
    };
    pi_add_engine_event_hook(hengine, &hook);
    succeed = true;
}

PI_SYSTEM_RENDER::~PI_SYSTEM_RENDER() noexcept
{
    m_objects.meshes.clear();
    while (!m_objects.worldenvs.empty()) { // Detach any remaining world environments from the user, to be able to clean them up.
        bool r = m_objects.worldenvs.back().detach_from_user();
        assert(r == true);
    }

    backend.callbacks.pfn_close(get_backend());
}

bool PI_SYSTEM_RENDER::_s_event_hook(void* userdata, PI_EVENT* event) noexcept
{
    auto renderer = static_cast<PI_SYSTEM_RENDER*>(userdata);
    PI_ASSERT(renderer != NULL);

    switch (event->type)
    {
        case PI_EVENT_WINDOW_SIZE:
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

PI_EXTERN_C PI_HWORLDENV pi_create_worldenvironment(PI_HGAME hgame, const PI_WORLDENV_DESC* pdesc)
{
    if (!hgame) {
        return NULL;
    }

    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    auto worldenv = engine->subsystems.render->create_worldenv(pdesc);
    return worldenv;
}

PI_EXTERN_C void pi_release_worldenvironment(PI_HWORLDENV hworldenv) {
    if (!hworldenv) {
        return;
    }

    hworldenv->detach_from_user();
}

PI_EXTERN_C PI_HSCENE pi_init_render_scene(PI_HSCENE hscene, PI_HWORLDENV hworldenv) {
    if (!hscene) {
        return NULL;
    }
    if (!hworldenv) {
        return NULL;
    }
    
    if (hscene->initsubsystems.render) {
        return NULL;
    }

    auto renderer = hscene->engine->subsystems.render;
    if (!renderer->init_scene(hscene->subsystems.render, hworldenv)) {
        return NULL;
    }

    hscene->initsubsystems.render = true;
    return hscene;
}

static inline bool _HSCENECHECK(PI_HSCENE hscene) {
    if (!hscene) {
        return true;
    }
    if (!hscene->initsubsystems.render) {
        return true;
    }

    PI_ASSERT(hscene->engine);
    PI_ASSERT(hscene->subsystems.render);
    return false;
}

#define HSCENE_CHECKEX(hscene, hsysscene, ret) if (_HSCENECHECK(hscene)) return ret; \
                                        auto hsysscene = (hscene)->subsystems.render
#define HSCENE_CHECK(hscene, hsysscene) HSCENE_CHECKEX(hscene, hsysscene, false)

PI_EXTERN_C bool pi_set_scene_clear_color(PI_HSCENE hscene, float r, float g, float b, float a) {
    HSCENE_CHECK(hscene, hsysscene);

    hsysscene->clear_color = PI_VECTOR4{ r, g, b, a };
    return true;
}
PI_EXTERN_C bool pi_set_scene_projection(PI_HSCENE hscene, const PI_MATRIX_4X4* mproj) {
    HSCENE_CHECK(hscene, hsysscene);

    hsysscene->projection = *mproj;
    hsysscene->dirty.viewproj = true;
    return true;
}
PI_EXTERN_C bool pi_set_scene_view(PI_HSCENE hscene, const PI_MATRIX_4X4* mview) {
    HSCENE_CHECK(hscene, hsysscene);

    hsysscene->view = *mview;
    hsysscene->dirty.viewproj = true;
    return true;
}

PI_EXTERN_C PI_HINSTANCE pi_add_scene_mesh(PI_HSCENE hscene, PI_HMESH hmesh, const PI_INSTANCE* instance) {
    if (!hscene) {
        return NULL;
    }

    HSCENE_CHECKEX(hscene, hsysscene, NULL);
    if (hscene->engine->subsystems.render != hmesh->renderer) {
        return NULL;
    }

    static const PI_INSTANCE INSTANCE_DEFAULT;
    if (!instance) {
        instance = &INSTANCE_DEFAULT;
    }

    return hsysscene->add_scene_mesh(hmesh, instance);
}
PI_EXTERN_C void pi_remove_scene_mesh(PI_HINSTANCE hmeshinst) {
    if (!hmeshinst) {
        return;
    }
    hmeshinst->instbuffer->instlist.remove(*hmeshinst);
}
PI_EXTERN_C bool pi_update_mesh_instance(PI_HINSTANCE hmeshinst, const PI_INSTANCE* instance) {
    if (!hmeshinst) {
        return false;
    }
    if (!instance) {
        return false;
    }
    return hmeshinst->update_instance(instance);
}


PI_EXTERN_C PI_HLIGHT pi_add_light(PI_HSCENE hscene, PI_LIGHTTYPE type, const PI_LIGHT* initial_parameters) {
    if (!hscene) {
        return NULL;
    }
    if (!hscene->initsubsystems.render) {
        return NULL;
    }

    return hscene->subsystems.render->add_light(type, initial_parameters);
}
PI_EXTERN_C void pi_remove_light(PI_HLIGHT hlight) {
    if (!hlight) {
        return;
    }
    hlight->hscene->remove_light(hlight);
}
PI_EXTERN_C bool pi_update_light(PI_HLIGHT hlight, const PI_LIGHT* light) {
    if (!hlight) {
        return false;
    }
    if (!light) {
        return false;
    }
    return hlight->hscene->update_light(hlight, light);
}

PI_EXTERN_C bool pi_set_light_properties(PI_HSCENE hscene, const PI_LIGHT_PROPERTIES* properties) {
    HSCENE_CHECK(hscene, hsysscene);
    if (!(hsysscene->worldenv->effects & PI_WORLDEFFECT_LIGHTING)) {
        return false;
    }

    hsysscene->lightprops = *properties;
    hsysscene->dirty.lightprops = true;
    return true;
}
