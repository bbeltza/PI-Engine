#include "render_internal.hpp"



PI_HSCENE PI_SYSTEM_RENDER::create_scene(PI_HWORLDENV hworldenv) noexcept
{
    PI_HSCENE hscene;
    m_objects.scenes.emplace_front(this, hworldenv);
    hscene = &m_objects.scenes.front();
    if (!hscene->renderer) {
        REMOVE_CMP_PTR(m_objects.scenes, *hscene);
        return NULL;
    }

    PI_ASSERT(hscene->worldenv != NULL);

    return hscene;
}

void PI_SYSTEM_RENDER::destroy_scene(PI_HSCENE hscene) noexcept {
    if (m_currentscn == hscene) {
        m_currentscn = NULL;
    }

    REMOVE_CMP_PTR(m_objects.scenes, *hscene);
}

PI_HSCENE PI_SYSTEM_RENDER::set_current_scene(PI_HSCENE hscene) noexcept
{
    auto prev = m_currentscn;
    m_currentscn = hscene;

    return prev;
}

//

PI_SCENE_IMPL::PI_SCENE_IMPL(PI_SYSTEM_RENDER* _renderer, PI_HWORLDENV _hworldenv):
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

PI_SCENE_IMPL::~PI_SCENE_IMPL() {
    if (renderer) {
        renderer->backend.callbacks.pfn_destroy_scene(renderer->get_backend(), this->get_backend());
        worldenv->release();
    }
}
