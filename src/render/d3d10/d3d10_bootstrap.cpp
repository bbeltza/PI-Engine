#include "render_d3d10.hpp"

class BACKEND_FAILURE {};

PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10 = {
    "Direct3D 10",
    pi_D3D10Backend::s_calcsize,
    [](PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinfo) -> bool {
        try {
            new (hbackend) pi_D3D10Backend{ *pdesc, pinfo };
        }
        catch (BACKEND_FAILURE&) {
            return false;
        }

        return true;
    }
};

pi_D3D10Backend::pi_D3D10Backend(const PI_BACKEND_RENDER_DESC& desc, PI_BACKEND_RENDER_INFO* pinfo)
{
    if (!initialize(desc)) {
        throw BACKEND_FAILURE{};
    }

    auto pcallbacks = &pinfo->callbacks;
    
    //#define STORE_CALLBACK(n) pcallbacks->pfn_##n = _s_##n
    pcallbacks->pfn_close = [](PI_HRENDERBACKEND hbackend) noexcept -> void {
        hbackend->~PI_RENDERBACKEND();
    };

    pcallbacks->pfn_configure = [](PI_HRENDERBACKEND hbackend, const PI_RENDER_CONFIGURE* info) noexcept -> bool {
        hbackend->configure(info);
        return true;
    };

    pcallbacks->pfn_create_worldenvironment = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_WORLDENV_DESC* pdesc) noexcept -> bool {
        return hbackend->create_worldenvironment(hworldenv, pdesc);
    };
    pcallbacks->pfn_destroy_worldenvironment = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_WORLDENV hworldenv) noexcept -> void {
        hbackend->destroy_worldenvironment(hworldenv);
    };

    pcallbacks->pfn_create_scene = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene) noexcept -> bool {
        return hbackend->create_scene(hscene);
    };
    pcallbacks->pfn_destroy_scene = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene) noexcept -> void {
        hbackend->destroy_scene(hscene);
    };

    pcallbacks->pfn_create_mesh = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, const PI_VERTEX* vertices, unsigned num_vertices, const void* indices, unsigned num_indices, PI_INDEX index_type) noexcept -> bool {
        return hbackend->create_mesh(hmesh, vertices, num_vertices, indices, num_indices, index_type);
    };
    pcallbacks->pfn_destroy_mesh = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh) noexcept -> void {
        hbackend->destroy_mesh(hmesh);
    };

    pcallbacks->pfn_create_instbuffer = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept -> bool {
        return hbackend->create_instbuffer(hinstbuffer);
    };
    pcallbacks->pfn_destroy_instbuffer = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept -> void {
        hbackend->destroy_instbuffer(hinstbuffer);
    };

    pcallbacks->pfn_alloc_instbuffer = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned size, const PI_INSTANCE initial_data[], const PI_MATRIX_3X3* normal_transform_data) noexcept -> bool {
        return hbackend->alloc_instbuffer(hinstbuffer, size, initial_data, normal_transform_data);        
    };
    pcallbacks->pfn_update_instbuffer = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned pos, unsigned size, const PI_INSTANCE data[], const PI_MATRIX_3X3* normal_transform_data) noexcept -> bool {
        return hbackend->update_instbuffer(hinstbuffer, pos, size, data, normal_transform_data);        
    };

    pcallbacks->pfn_update_scene_vp = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const PI_MATRIX_4X4* vpmatrix) noexcept -> void {
        hbackend->update_scene_vp(hscene, vpmatrix);
    };
    pcallbacks->pfn_update_scene_lightproperties = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const PI_LIGHT_PROPERTIES* properties) noexcept -> void {
        hbackend->update_scene_lightproperties(hscene, properties);
    };
    pcallbacks->pfn_begin_scene = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_VECTOR4* clear_rgba) noexcept -> void {
        hbackend->begin_scene(hscene, hworldenv, clear_rgba);
    };
    pcallbacks->pfn_end_scene = [](PI_HRENDERBACKEND hbackend) noexcept -> void {
        hbackend->end_scene();
    };

    pcallbacks->pfn_submit_mesh = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned instance_count) noexcept -> void {
        hbackend->submit_mesh(hmesh, hinstbuffer, instance_count);
    };

    pcallbacks->pfn_update_lights = [](PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, PI_LIGHTTYPE type, unsigned update_start, unsigned update_count, const PI_LIGHT lights[], unsigned full_light_count) noexcept -> void {
        hbackend->update_lights(hscene, type, update_start, update_count, lights, full_light_count);
    };

    ASSERT_BACKEND_CALLBACKS(pinfo->callbacks);
    DEFINE_BACKEND_STRUCTURES(pinfo->structures);
}

pi_D3D10Backend::~pi_D3D10Backend() noexcept {
    release_objects();
}
