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
    m_objects.meshes.emplace_front(
            this,
            num_vertices, vertices,
            num_indices, indices, index_type
        );
    hmesh = &m_objects.meshes.front();
    if (!hmesh->renderer) {
        REMOVE_CMP_PTR(m_objects.meshes, *hmesh);
        return NULL;
    }

    return hmesh;
}

void PI_SYSTEM_RENDER::destroy_mesh(PI_HMESH hmesh) noexcept
{
    if (!hmesh) {
        return;
    }

    REMOVE_CMP_PTR(m_objects.meshes, *hmesh);
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
