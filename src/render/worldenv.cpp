#include "render_internal.hpp"

PI_HWORLDENV PI_SYSTEM_RENDER::create_worldenv(const PI_WORLDENV_DESC* pdesc) noexcept
{
    // TODO: Perform validation to the desc
    PI_WORLDENV_DESC realdesc = *pdesc;
    if (!realdesc.rasterizer) {
        static const PI_WORLDENV_RASTERIZER DEFAULT_RASTERIZER{};
        realdesc.rasterizer = &DEFAULT_RASTERIZER;
    }
        // Both depth and stencil CAN be seen NULL by the backend. So be careful...
    if (!realdesc.depth) {
        
    }
    /*if (!realdesc.stencil) */ {
        
    }

    if (!realdesc.multisample) {
        static const PI_WORLDENV_MSAA DEFAULT_MULTISAMPLE{};
        realdesc.multisample = &DEFAULT_MULTISAMPLE;
    }

    PI_HWORLDENV hworldenv;
    m_objects.worldenvs.emplace_front(this, &realdesc);
    hworldenv = &m_objects.worldenvs.front();
    if (!hworldenv->renderer) {
        REMOVE_CMP_PTR(m_objects.worldenvs, *hworldenv);
        return NULL;
    }
    return hworldenv;
}

void PI_SYSTEM_RENDER::remove_worldenv(PI_HWORLDENV hworldenv) noexcept {
    REMOVE_CMP_PTR(m_objects.worldenvs, *hworldenv);
}

//

PI_WORLDENV_IMPL::PI_WORLDENV_IMPL(PI_SYSTEM_RENDER* _renderer, const PI_WORLDENV_DESC* pdesc) noexcept:
    renderer(_renderer)
{
    if (!renderer->backend.callbacks.pfn_create_worldenvironment(renderer->get_backend(), this->get_backend(), pdesc)) {
        this->renderer = NULL;
        return;
    }
}

PI_WORLDENV_IMPL::~PI_WORLDENV_IMPL() noexcept {
    PI_ASSERT(m_refcount == 0 || !renderer);

    if (renderer) {
        renderer->backend.callbacks.pfn_destroy_worldenvironment(renderer->get_backend(), this->get_backend());
    }
}

void PI_WORLDENV_IMPL::destroy() noexcept {
    PI_ASSERT(renderer);
    PI_ASSERT(m_refcount == 0);

    renderer->remove_worldenv(this);
}
