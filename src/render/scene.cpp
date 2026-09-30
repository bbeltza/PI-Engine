#include "render_internal.hpp"

PI_HSCENE PI_SYSTEM_RENDER::create_scene(void) noexcept
{
    PI_HSCENE hscene;
    {
        auto p_rawscene = operator new(sizeof(PI_SCENE_IMPL) + backend.structures.SCENE_size, std::nothrow);
        if (!p_rawscene) {
            return NULL;
        }

        hscene = new (p_rawscene) PI_SCENE_IMPL(this);
        if (!hscene->renderer) {
            hscene->~PI_SCENE_IMPL();
            operator delete(hscene, std::nothrow);
            return NULL;
        }
    }

    if (!m_freelists.scene.empty()) {
        size_t i = m_freelists.scene.back();
        m_freelists.scene.pop_back();

        m_objects.scenes.at(i).reset(hscene);
    }
    else {
        m_objects.scenes.emplace_back(hscene);
    }

    return hscene;
}

void PI_SYSTEM_RENDER::release_scene(PI_HSCENE hscene) noexcept {
    for (size_t i = 0; i < m_objects.scenes.size(); i++ ) {
        auto& scn = m_objects.scenes.at(i);
        if (scn.get() == hscene) {
            scn.reset();
            m_freelists.scene.push_back(i);
            break;
        }
    }
}

PI_HSCENE PI_SYSTEM_RENDER::set_current_scene(PI_HSCENE hscene) noexcept
{
    auto prev = m_currentscn;
    m_currentscn = hscene;

    return prev;
}

//

PI_SCENE_IMPL::PI_SCENE_IMPL(PI_SYSTEM_RENDER* _renderer):
    renderer(_renderer),

    clear_color{ 0.7f, 0.7f, 1.0f, 1.0f },
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

    m_instance_set(PI_INSTMAP_ALLOC{ this })
{
    if (!renderer->backend.callbacks.pfn_create_scene(renderer->get_backend(), this->get_backend())) {
        this->renderer = NULL;
        return;
    }
}

PI_SCENE_IMPL::~PI_SCENE_IMPL() {
    if (renderer) {
        renderer->backend.callbacks.pfn_destroy_scene(renderer->get_backend(), this->get_backend());
    }
}
