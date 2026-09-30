#include "render_internal.hpp"

PI_HINSTANCE PI_SCENE_IMPL::add_scene_mesh(PI_HMESH hmesh, const PI_INSTANCE* instance) noexcept
{
    if (m_instance_set.find(hmesh) == m_instance_set.end()) {
        /* Allocate Instance buffer for the mesh if it hasn't been allocated yet */
        auto inserted = m_instance_set.emplace(hmesh, this->renderer);
        PI_ASSERT(inserted.second == true);

        if (!inserted.first->second.renderer) {
            m_instance_set.erase(hmesh);
            return NULL;
        }
    }

    auto& instbuffer = m_instance_set.at(hmesh);
    instbuffer.instlist.emplace_front(&instbuffer, instance);
    return &instbuffer.instlist.front();
}

PI_INSTANCE_IMPL::PI_INSTANCE_IMPL(PI_INSTBUFFER_IMPL* _instbuffer, const PI_INSTANCE* instance):
    instbuffer(_instbuffer),
    index(instbuffer->bufdata.size())
{
    instbuffer->bufdata.push_back(*instance);
    if (!instbuffer->dirty.size) {
        instbuffer->dirty.begin = index;
        instbuffer->dirty.size = 1;
    }
    else {
        instbuffer->dirty.size = instbuffer->bufdata.size();
    }
}

PI_INSTANCE_IMPL::~PI_INSTANCE_IMPL() noexcept
{
    for (auto& i : instbuffer->instlist) {
        PI_ASSERT(&i == this || i.index != index);
        if (i.index <= index) {
            break;
        }
        i.index--;
    }
    
    instbuffer->bufdata.erase(instbuffer->bufdata.cbegin() + index);
    if (instbuffer->dirty.size) {
        instbuffer->dirty.begin = std::min(instbuffer->dirty.begin, index);
    }
    else {
        instbuffer->dirty.begin = index;
    }
    instbuffer->dirty.size = instbuffer->bufdata.size() - instbuffer->dirty.size;
}

bool PI_INSTANCE_IMPL::update_instance(const PI_INSTANCE* instance) noexcept
{
    instbuffer->bufdata.at(index) = *instance;
    if (instbuffer->dirty.size) {
        auto inc = instbuffer->dirty.begin + 1;
        instbuffer->dirty.begin = std::min(instbuffer->dirty.begin, index);
        instbuffer->dirty.size += inc - instbuffer->dirty.begin;
    }
    else {
        instbuffer->dirty.begin = index;
        instbuffer->dirty.size = 1;
    }

    return true;
}

//

PI_INSTBUFFER_IMPL::PI_INSTBUFFER_IMPL(PI_SYSTEM_RENDER* _renderer) noexcept:
    renderer(_renderer)
{ 
    if (!renderer->backend.callbacks.pfn_create_instbuffer(renderer->get_backend(), this->get_backend())) {
        this->renderer = NULL;
        return;
    }
}

PI_INSTBUFFER_IMPL::~PI_INSTBUFFER_IMPL() noexcept
{
    if (renderer) {
        instlist.clear();
        renderer->backend.callbacks.pfn_destroy_instbuffer(renderer->get_backend(), this->get_backend());
    }
    else {
        PI_ASSERT(instlist.empty());
        PI_ASSERT(bufdata.empty());
    }
}
