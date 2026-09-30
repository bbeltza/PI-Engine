#include "render_internal.hpp"
#include <piengine/pi_math.h>

void PI_SYSTEM_RENDER::update(void) noexcept
{
    if (!m_currentscn) {
        return;
    }

    PI_HRENDERBACKEND hbackend = this->get_backend();
    PI_HRENDERBACKEND_SCENE hcurrscn = m_currentscn->get_backend();

    if (m_currentscn->dirty.viewproj) {
        auto vpmatrix = pi_mul_4x4(
                PI_FPTOMATRIX_4X4(m_currentscn->view),
                PI_FPTOMATRIX_4X4(m_currentscn->projection)
            );
        backend.callbacks.pfn_update_scene_vp(
                hbackend,
                hcurrscn,
                PI_MFLOATP(vpmatrix)
            );
        m_currentscn->dirty.viewproj = false;
    }

    for (auto it = m_currentscn->begin_instbufs(); it != m_currentscn->end_instbufs(); it++)
    {
        auto& instbuf = it->second;
        if (!instbuf.dirty.size) {
            continue;
        }
        
        bool bres;
        PI_HRENDERBACKEND_INSTBUFFER hinstbuf = instbuf.get_backend();
        if (instbuf.backend_size != instbuf.bufdata.capacity()) {
            // `capacity()` can be lower than `backend_size` if `shrink_to_fit` is called. In that case, it'll shrink the buffer. But it very likely won't ever happen, this usually runs when the buffer needs to grow and thus be re-allocated
            instbuf.backend_size = instbuf.bufdata.capacity();
            bres = backend.callbacks.pfn_alloc_instbuffer(
                    hbackend,
                    hinstbuf,
                    static_cast<unsigned>(instbuf.backend_size),
                    instbuf.bufdata.data()
                );
            // Dirty regions will be allocated when `instbuf.bufdata.data()` is passed to the backend.
        }
        else {
            bres = backend.callbacks.pfn_update_instbuffer(
                    hbackend,
                    hinstbuf,
                    static_cast<unsigned>(instbuf.dirty.begin),
                    static_cast<unsigned>(instbuf.dirty.size),
                    &instbuf.bufdata.at(instbuf.dirty.begin)
                );
        }
        instbuf.dirty.begin = 0;
        instbuf.dirty.size = 0;

    }

    backend.callbacks.pfn_begin_scene(hbackend, hcurrscn, m_currentscn->clear_color);

    for (auto it = m_currentscn->begin_instbufs(); it != m_currentscn->end_instbufs(); ++it)
    {
        auto& mesh = it->first;
        auto& instbuf = it->second;

        if (!instbuf.bufdata.size()) { // Can happen, we don't wanna submit 0 instances to the pipeline.
            continue;
        }

        backend.callbacks.pfn_submit_mesh(
                hbackend,
                mesh->get_backend(),
                instbuf.get_backend(),
                static_cast<unsigned>(instbuf.bufdata.size())
            );
    }

    backend.callbacks.pfn_end_scene(hbackend);
}
