#include "render_internal.hpp"

#include <piengine/pi_math.h>

#include <cstdio>

template <typename Vec>
void update_lights_dirty(Vec& v, unsigned lindex, unsigned& start_dirty, unsigned& count_dirty)
{
    auto count = static_cast<unsigned>(v.size());
    PI_ASSERT(lindex < count);
    if (!count_dirty) {
        count_dirty = 1;
        start_dirty = lindex;
    }
    else if (start_dirty > lindex) {
        count_dirty += start_dirty - lindex;
        start_dirty = lindex;
    }
    else if (count_dirty <= lindex) {
        PI_ASSERT(start_dirty < lindex);
        count_dirty = lindex - start_dirty + 1;
    };
}

template <typename Vec>
void update_remove_lights_dirty(PI_LIST<PI_LIGHT_IMPL>& lights, PI_HLIGHT hlight, Vec& v, unsigned& start_dirty, unsigned& count_dirty) {
    auto count = static_cast<unsigned>(v.size());
    PI_ASSERT(hlight->index < count);
    if (hlight->index == count - 1) {
        v.pop_back();
        if (!count_dirty) {
            start_dirty = -1;
        }
        return;
    }
    else if (!count_dirty) {
        count_dirty = count - hlight->index;
        start_dirty = hlight->index;
    }
    else if (start_dirty > hlight->index) {
        start_dirty = hlight->index;
    }

    v.erase(v.begin() + hlight->index);

    for (auto& l : lights)
    {
        if (l.type != hlight->type) {
            continue;
        }
        if (l.index > hlight->index) {
            l.index--;
        }
    }
}


PI_HLIGHT PI_SYSTEM_SCENE::add_light(PI_LIGHTTYPE type, const PI_LIGHT* initial_parameters) noexcept
{
    if (!(worldenv->effects & PI_WORLDEFFECT_LIGHTING)) {
        fprintf(stderr, "[ERROR]: Can't add light: World environment has lighting disabled\n");
        return NULL;
    }

    unsigned lindex = 0;
    switch (type) {
        case PI_LIGHT_DIRECTIONAL:
            lindex = static_cast<unsigned>(m_dirlight_buffer.size());
            {
                PI_DIRECTIONAL_LIGHT reallight = initial_parameters->directional;
                reallight.direction.w = 0;
                reallight.direction = pi_vec4_normalize(reallight.direction);
                m_dirlight_buffer.push_back(reallight);
            }
            update_lights_dirty(m_dirlight_buffer, lindex, m_dirlight_start_dirty, m_dirlight_count_dirty);
            break;
        case PI_LIGHT_POINT:
            lindex = static_cast<unsigned>(m_ptlight_buffer.size());
            m_ptlight_buffer.push_back(initial_parameters->point);
            update_lights_dirty(m_ptlight_buffer, lindex, m_ptlight_start_dirty, m_ptlight_count_dirty);
            break;
        default:
            return NULL;
    }

    m_lights.emplace_front(this, type, lindex);
    return &m_lights.front();
}


void PI_SYSTEM_SCENE::remove_light(PI_HLIGHT hlight) noexcept
{
    PI_ASSERT(hlight->hscene == this);
    switch (hlight->type)
    {
        case PI_LIGHT_DIRECTIONAL:
            update_remove_lights_dirty(m_lights, hlight, m_dirlight_buffer, m_dirlight_start_dirty, m_dirlight_count_dirty);
            break;
        case PI_LIGHT_POINT:
            update_remove_lights_dirty(m_lights, hlight, m_ptlight_buffer, m_ptlight_start_dirty, m_ptlight_count_dirty);
            break;
        default:
            PI_ASSUME(0);
    }

    REMOVE_CMP_PTR(m_lights, *hlight);
}





bool PI_SYSTEM_SCENE::update_light(PI_HLIGHT hlight, const PI_LIGHT* light) noexcept
{
    PI_ASSERT(hlight->hscene == this);
    switch (hlight->type) {
        case PI_LIGHT_DIRECTIONAL:
            update_lights_dirty(m_dirlight_buffer, hlight->index, m_dirlight_start_dirty, m_dirlight_count_dirty);
            m_dirlight_buffer.at(hlight->index) = light->directional;
            m_dirlight_buffer.at(hlight->index).direction.w = 0.0f;
            m_dirlight_buffer.at(hlight->index).direction = pi_vec4_normalize(light->directional.direction);
            break;
        case PI_LIGHT_POINT:
            update_lights_dirty(m_ptlight_buffer, hlight->index, m_ptlight_start_dirty, m_ptlight_count_dirty);
            m_ptlight_buffer.at(hlight->index) = light->point;
            break;
        default:
            PI_ASSUME(0);
            return false;
    }
    return true;
}


PI_LIGHT_IMPL::PI_LIGHT_IMPL(PI_HSYSTEMSCENE hscene, PI_LIGHTTYPE type, unsigned index) noexcept:
    hscene{hscene},
    type{type},
    index{index}
{
    
}

PI_LIGHT_IMPL::~PI_LIGHT_IMPL() noexcept {
    
}
