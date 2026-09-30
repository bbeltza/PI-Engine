#include "render_d3d10.hpp"

bool pi_D3D10Backend::create_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept
{
    new (hinstbuffer) pi_D3D10InstBuffer{};
    return true;
}

void pi_D3D10Backend::destroy_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept
{
    hinstbuffer->~pi_D3D10InstBuffer();
}

pi_D3D10InstBuffer::~pi_D3D10InstBuffer() {
    COMRELEASESAFE(dxbuffer);
    COMRELEASESAFE(dxstagingbuffer);
}


bool pi_D3D10Backend::alloc_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned size, const PI_INSTANCE initial_data[]) noexcept
{
    HRESULT hr;
    ID3D10Buffer* dxbuffer = NULL;
    ID3D10Buffer* dxstagingbuffer = NULL;
    D3D10_BUFFER_DESC desc = {};
    desc.ByteWidth = size * sizeof(PI_INSTANCE);
    desc.Usage = D3D10_USAGE_DEFAULT;
    desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;

    D3D10_SUBRESOURCE_DATA sinitial_data{};
    sinitial_data.pSysMem = initial_data;
    COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, &sinitial_data, &dxbuffer));
    if (FAILED(hr)) {
        return false;
    }

    desc.Usage = D3D10_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, NULL, &dxstagingbuffer));
    if (FAILED(hr)) {
        COMRELEASE(dxbuffer);
        return false;
    }

    COMRELEASESAFE(hinstbuffer->dxbuffer);
    COMRELEASESAFE(hinstbuffer->dxstagingbuffer);
    hinstbuffer->dxbuffer = dxbuffer;
    hinstbuffer->dxstagingbuffer = dxstagingbuffer;
    return true;
}

bool pi_D3D10Backend::update_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned pos, unsigned size, const PI_INSTANCE data[]) noexcept
{
    HRESULT hr;
    PI_INSTANCE* pinstancedata = NULL;
    COMCHECKCALL(hinstbuffer->dxstagingbuffer->Map(D3D10_MAP_WRITE, 0, reinterpret_cast<void**>(&pinstancedata)));
    if (FAILED(hr)) {
        return false;
    }
        std::memcpy(pinstancedata + pos, data, size * sizeof(PI_INSTANCE));
    hinstbuffer->dxstagingbuffer->Unmap();

    D3D10_BOX box{};
    box.left = pos * sizeof(PI_INSTANCE);
    box.right = box.left + size * sizeof(PI_INSTANCE);
    box.bottom = 1;
    box.back = 1;

    m_dxdevice->CopySubresourceRegion(hinstbuffer->dxbuffer, 0, pos * sizeof(PI_INSTANCE), 0, 0, hinstbuffer->dxstagingbuffer, 0, &box );
    return true;
}
