#include "render_internal.hpp"

bool PI_SYSTEM_RENDER::init_scene(PI_HSYSTEMSCENE hsysscene, PI_HWORLDENV hworldenv) noexcept
{
    new (hsysscene) PI_SYSTEM_SCENE{ this, hworldenv };
    if (!hsysscene->renderer) {
        hsysscene->~PI_SYSTEM_SCENE();
        return false;
    }

    PI_ASSERT(hsysscene->worldenv != NULL);
    return true;
}

//

PI_SYSTEM_SCENE::PI_SYSTEM_SCENE(PI_SYSTEM_RENDER* _renderer, PI_HWORLDENV _hworldenv):
    renderer(_renderer),
    worldenv(_hworldenv),

    projection{ 1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
    },
    view{ 1.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f
    },
    clear_color{ 0.7f, 0.7f, 1.0f, 1.0f },

    m_instance_set(PI_INSTMAP_ALLOC{ _renderer->backend.structures.INSTBUFFER_size })
{
    if (!renderer->backend.callbacks.pfn_create_scene(renderer->get_backend(), this->get_backend())) {
        this->renderer = NULL;
        return;
    }

    worldenv->acquire();
}

PI_SYSTEM_SCENE::~PI_SYSTEM_SCENE() {
    if (renderer) {
        renderer->backend.callbacks.pfn_destroy_scene(renderer->get_backend(), this->get_backend());
        worldenv->release();
    }
}
