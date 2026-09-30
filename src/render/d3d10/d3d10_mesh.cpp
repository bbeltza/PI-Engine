#include "render_d3d10.hpp"

bool pi_D3D10Backend::create_mesh(PI_HRENDERBACKEND_MESH hmesh, const PI_VERTEX* vertices, unsigned num_vertices, const void* indices, unsigned num_indices, PI_INDEX index_type) noexcept
{
    new (hmesh) pi_D3D10Mesh{};
    HRESULT hr;

    D3D10_BUFFER_DESC desc = {};
    desc.ByteWidth = num_vertices * sizeof(PI_VERTEX);
    desc.Usage = D3D10_USAGE_IMMUTABLE;
    desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D10_SUBRESOURCE_DATA initial_data{}; // Initial data if there's only vertex data
    BYTE* staging_buffer = NULL;

    if (index_type == PI_INDEX_UNKNOWN) { // No index buffer bound
        initial_data.pSysMem = vertices;
        hmesh->vertex_count = num_vertices;
    }
    else {
        hmesh->index_offset = desc.ByteWidth;
        hmesh->vertex_count = num_indices;
        
        switch (index_type) {
            case PI_INDEX_U16:
                desc.ByteWidth += num_indices * sizeof(UINT16);
                hmesh->index_format = DXGI_FORMAT_R16_UINT;
                break;
            case PI_INDEX_U32:
                desc.ByteWidth += num_indices * sizeof(UINT32);
                hmesh->index_format = DXGI_FORMAT_R32_UINT;
                break;
            default:
                break;
        }

        staging_buffer = new BYTE[desc.ByteWidth];
        initial_data.pSysMem = staging_buffer;
        std::memcpy(staging_buffer + 0, vertices, hmesh->index_offset);
        std::memcpy(staging_buffer + hmesh->index_offset, indices, desc.ByteWidth - hmesh->index_offset);
    }

    COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, &initial_data, &hmesh->dxbuffer));
    delete[] staging_buffer;

    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void pi_D3D10Backend::destroy_mesh(PI_HRENDERBACKEND_MESH hmesh) noexcept
{
    hmesh->~pi_D3D10Mesh();
}

pi_D3D10Mesh::~pi_D3D10Mesh() noexcept {
    COMRELEASE(dxbuffer);
}

void pi_D3D10Backend::submit_mesh(PI_HRENDERBACKEND_MESH hmesh, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned instance_count) noexcept
{
    ID3D10Buffer* const buffers[] = {
        hmesh->dxbuffer,
        hinstbuffer->dxbuffer 
    };
    UINT const strides[] = {
        sizeof(PI_VERTEX),
        sizeof(PI_INSTANCE)
    };
    UINT const offsets[] = {
        0,
        0
    };

    m_dxdevice->IASetVertexBuffers(0, PI_ARRAYSIZE(buffers), buffers, strides, offsets);
    if (hmesh->index_format != DXGI_FORMAT_UNKNOWN) {
        m_dxdevice->IASetIndexBuffer(hmesh->dxbuffer, hmesh->index_format, hmesh->index_offset);
        m_dxdevice->DrawIndexedInstanced(hmesh->vertex_count, instance_count, 0, 0, 0);
    }
    else {
        m_dxdevice->DrawInstanced(hmesh->vertex_count, instance_count, 0, 0);
    }
}
