#include "render_internal.hpp"
#include <piengine/pi_math.h>

void PI_SYSTEM_RENDER::update(PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount) noexcept
{
    if (!scenecount) {
        return;
    }

    PI_ASSERT(scenecount == 1);
    for (unsigned i = 0; i < scenecount; i++) {
        PI_HSYSTEMSCENE hsysscene = hsysscenes[i];
        hsysscene->update();

        break; // Multi scenes aren't implemented yet :-)
    }
}

bool PI_SYSTEM_SCENE::update() noexcept
{
    /* Updating those things individually per loop (making 3 loops in the PI_SYSTEM_RENDER::update function should be a safer, and just better choice) */

    if (!_update_dirty()) {
        return false;
    }
    if (!_update_instances()) {
        return false;
    }
    if (!_submit_scene()) {
        return false;
    }

    return true;
}

//

bool PI_SYSTEM_SCENE::_update_dirty() noexcept
{
    PI_HRENDERBACKEND hbackend = renderer->get_backend();
    PI_HRENDERBACKEND_SCENE hscenebackend = this->get_backend();
    PI_BACKEND_RENDER_INFO& backend = renderer->backend;
    if (this->dirty.viewproj) {
        PI_MATRIX_4X4 vpmatrix;
        pi_mul_4x4(&vpmatrix, &this->projection, &this->view);
        backend.callbacks.pfn_update_scene_vp(
                hbackend,
                hscenebackend,
                &vpmatrix
            );
        this->dirty.viewproj = false;
    }
    if (this->dirty.lightprops) {
        backend.callbacks.pfn_update_scene_lightproperties(
                hbackend,
                hscenebackend,
                &lightprops
            );
        this->dirty.lightprops = false;
    }

    if (this->m_dirlight_count_dirty) {
        m_lights_dirty.clear();
        m_lights_dirty.resize(m_dirlight_count_dirty);
        for (unsigned i = 0, ii = m_dirlight_start_dirty; i < m_dirlight_count_dirty; i++, ii++) {
            m_lights_dirty.at(i).directional = m_dirlight_buffer.at(ii);
        }

        auto fullcount = static_cast<unsigned>(m_dirlight_buffer.size());
        backend.callbacks.pfn_update_lights(
                hbackend,
                hscenebackend,
                PI_LIGHT_DIRECTIONAL,
                m_dirlight_start_dirty,
                m_dirlight_count_dirty,
                m_lights_dirty.data(),
                fullcount
            );
    }
    else if (this->m_dirlight_start_dirty == -1) {
        auto fullcount = static_cast<unsigned>(m_dirlight_buffer.size());
        backend.callbacks.pfn_update_lights(
                hbackend,
                hscenebackend,
                PI_LIGHT_DIRECTIONAL,
                0, 0, NULL,
                fullcount
            );
    }
    if (this->m_ptlight_count_dirty) {
        m_lights_dirty.clear();
        m_lights_dirty.resize(m_ptlight_count_dirty);
        for (unsigned i = 0, ii = m_ptlight_start_dirty; i < m_ptlight_count_dirty; i++, ii++) {
            m_lights_dirty.at(i).point = m_ptlight_buffer.at(ii);
        }

        auto fullcount = static_cast<unsigned>(m_ptlight_buffer.size());
        backend.callbacks.pfn_update_lights(
                hbackend,
                hscenebackend,
                PI_LIGHT_POINT,
                m_ptlight_start_dirty,
                m_ptlight_count_dirty,
                m_lights_dirty.data(),
                fullcount
            );
    }
    else if (this->m_ptlight_start_dirty == -1) {
        auto fullcount = static_cast<unsigned>(m_ptlight_buffer.size());
        backend.callbacks.pfn_update_lights(
                hbackend,
                hscenebackend,
                PI_LIGHT_POINT,
                0, 0, NULL,
                fullcount
            );
    }

    return true;
}

static const PI_MATRIX_3X3* update_normal_matrices(PI_WORLDENV_EFFECTS effects, PI_INSTBUFFER_IMPL* instbuffer, unsigned start, unsigned size) noexcept
{
    if (!(effects & PI_WORLDEFFECT_LIGHTING)) {
        return NULL;
    }

    instbuffer->normaldata.resize(size);
    const PI_INSTANCE* inststart = instbuffer->bufdata.data() + start;
    for (unsigned i = 0; i < size; i++) {
        PI_MATRIX_3X3* nm = &instbuffer->normaldata.at(i);
        const PI_MATRIX_4X4* im = &inststart->transform;
        PI_MATRIX_3X3 n;
        pi_matrix_4x4_to_3x3(&n, im);
        PI_MATRIX_3X3* res = pi_inverse_3x3(&n, &n); 
        PI_ASSERT(res);
        *nm = PI_INIT_STRUCT(PI_MATRIX_3X3){
            n.xx, n.yx, n.zx,
            n.xy, n.yy, n.zy,
            n.xz, n.yz, n.zz
        };
    }

    return instbuffer->normaldata.data();
}

bool PI_SYSTEM_SCENE::_update_instances() noexcept
{
    PI_HRENDERBACKEND hbackend = renderer->get_backend();
    PI_BACKEND_RENDER_INFO& backend = renderer->backend;

    for (auto it = m_instance_set.begin(); it != m_instance_set.end(); it++)
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
            unsigned size = static_cast<unsigned>(instbuf.backend_size);
            const PI_MATRIX_3X3* nm = update_normal_matrices(worldenv->effects, &instbuf, 0, size);
            bres = backend.callbacks.pfn_alloc_instbuffer(
                    hbackend,
                    hinstbuf,
                    size,
                    instbuf.bufdata.data(),
                    nm
                );
            // Dirty regions will be allocated when `instbuf.bufdata.data()` is passed to the backend.
        }
        else {
            unsigned pos = static_cast<unsigned>(instbuf.dirty.begin);
            unsigned size = static_cast<unsigned>(instbuf.dirty.size);
            const PI_MATRIX_3X3* nm = update_normal_matrices(worldenv->effects, &instbuf, pos, size);
            bres = backend.callbacks.pfn_update_instbuffer(
                    hbackend,
                    hinstbuf,
                    pos,
                    size,
                    &instbuf.bufdata.at(instbuf.dirty.begin),
                   nm 
                );
        }
        if (!bres) {
            return false;
        }
        instbuf.dirty.begin = 0;
        instbuf.dirty.size = 0;

    }

    return true;
}

bool PI_SYSTEM_SCENE::_submit_scene() noexcept
{
    PI_ASSERT(dirty.viewproj == false);

    PI_HRENDERBACKEND hbackend = renderer->get_backend();
    PI_HRENDERBACKEND_WORLDENV hwebackend = worldenv->get_backend();
    PI_HRENDERBACKEND_SCENE hscenebackend = this->get_backend();
    PI_BACKEND_RENDER_INFO& backend = renderer->backend;
    
    backend.callbacks.pfn_begin_scene(hbackend, hscenebackend, hwebackend, &this->clear_color);

    for (auto it = m_instance_set.begin(); it != m_instance_set.end(); ++it)
    {
        auto& mesh = it->first;
        auto& instbuf = it->second;

        if (!instbuf.bufdata.size()) { // Can happen, we don't wanna submit 0 instances to the pipeline.
            continue;
        }

        PI_ASSERT(instbuf.dirty.begin == 0);
        PI_ASSERT(instbuf.dirty.size == 0);
        backend.callbacks.pfn_submit_mesh(
                hbackend,
                mesh->get_backend(),
                instbuf.get_backend(),
                static_cast<unsigned>(instbuf.bufdata.size())
            );
    }

    backend.callbacks.pfn_end_scene(hbackend);

    return true;
}
