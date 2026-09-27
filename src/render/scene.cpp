#include "render_internal.hpp"

PI_HSCENE PI_SYSTEM_RENDER::create_scene(void) noexcept
{
    // Uhh I need an alloc function
    // But I still haven't set up my annoying backend interface aaaaaaAAAAHHh

    auto scn = new PI_SCENE_IMPL{this};
    if (!m_freelists.scene.empty()) {
        size_t i = m_freelists.scene.back();
        m_freelists.scene.pop_back();

        m_objects.scenes.at(i).reset(scn);
    }
    else {
        m_objects.scenes.emplace_back(scn);
    }

    return scn;
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
    #if 0

    m_backend.switch_scene(get_backend(), hscene->get_backend());

    #endif

    return prev;
}

//

PI_SCENE_IMPL::PI_SCENE_IMPL(PI_SYSTEM_RENDER* _renderer):
    renderer(_renderer)
{
    
}
