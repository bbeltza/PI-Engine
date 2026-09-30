#include "render_internal.hpp"

PI_HMESH PI_SYSTEM_RENDER::create_mesh_indexed(int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) noexcept
{
    /* Checking for any invalid arguments */

    if (!vertices) {
        return NULL;
    }
    if (num_vertices <= 0) {
        return NULL;
    }

    switch (index_type)
    {
        case PI_INDEX_U16:
        case PI_INDEX_U32:
            if (!indices) {
                return NULL;
            }
            if (num_indices <= 0) {
                return NULL;
            }
            break;
        case PI_INDEX_UNKNOWN:
            // If  `index_type` is "PI_INDEX_UNKNOWN" then `indices` must be "NULL" and `num_indices` must be 0.
            if (indices || num_indices) {
                return NULL;
            }
            break;
        default:
            return NULL;
    }

    /* Allocating the object */
    PI_HMESH hmesh;
    {
        auto p_rawmesh = operator new(sizeof(PI_MESH_IMPL) + backend.structures.MESH_size, std::nothrow);
        if (!p_rawmesh) {
            return NULL;
        }

        hmesh = new(p_rawmesh) PI_MESH_IMPL(this, num_vertices, vertices, num_indices, indices, index_type);
        // PI_MESH_IMPL sets the `renderer` member to NULL if internal initialization fails
        if (!hmesh->renderer) {
            hmesh->~PI_MESH_IMPL();
            operator delete(hmesh, std::nothrow);
            return NULL;
        }
    }

    if (!m_freelists.mesh.empty()) {
        size_t index = m_freelists.mesh.back();
        m_freelists.mesh.pop_back();
        m_objects.meshes.at(index).reset(hmesh);
    }
    else {
        m_objects.meshes.emplace_back(hmesh);
    }
    return hmesh;
}

void PI_SYSTEM_RENDER::destroy_mesh(PI_HMESH hmesh) noexcept
{
    if (!hmesh) {
        return;
    }

    for (size_t i = 0; i < m_objects.meshes.size(); i++) {
        auto& pmesh = m_objects.meshes.at(i);
        if (pmesh.get() == hmesh) {
            pmesh.reset();
            break;
        }
    }
}


PI_MESH_IMPL::PI_MESH_IMPL(PI_SYSTEM_RENDER* _renderer, unsigned num_vertices, const PI_VERTEX vertices[], unsigned num_indices, const void* indices, PI_INDEX index_type) noexcept:
    renderer(_renderer)
{
    if (!renderer->backend.callbacks.pfn_create_mesh(renderer->get_backend(), this->get_backend(), vertices, num_vertices, indices, num_indices, index_type)) {
        this->renderer = NULL;
        return;
    }
}

PI_MESH_IMPL::~PI_MESH_IMPL() noexcept {
    if (renderer) {
        renderer->backend.callbacks.pfn_destroy_mesh(renderer->get_backend(), this->get_backend());
    }
}
