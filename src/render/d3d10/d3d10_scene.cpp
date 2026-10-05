#include "render_d3d10.hpp"

pi_D3D10Scene::~pi_D3D10Scene() noexcept
{
    COMRELEASE(dxcbvbuffer);
}

bool pi_D3D10Backend::create_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept
{
    new (hscene) pi_D3D10Scene{};
    HRESULT hr;

    D3D10_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(PI_SCENEENV);
    desc.Usage = D3D10_USAGE_DYNAMIC;
    desc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, NULL, &hscene->dxcbvbuffer));
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void pi_D3D10Backend::destroy_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept {
    if (m_lastscene == hscene) {
        bind_scene(NULL);
    }
    hscene->~pi_D3D10Scene();
}

//

void pi_D3D10Backend::update_scene_vp(PI_HRENDERBACKEND_SCENE hscene, const PI_MATRIX_4X4* vpmatrix) noexcept
{
    HRESULT hr;
    PI_SCENEENV* psceneenv = NULL;
    // It's dangerous to use MAP_WRITE_OVERWRITE. Sadly, the normal MAP_WRITE doesn't work with dynamic resources
    //  So I'll either have to create a staging buffer mechanism or use MAP_WRITE_DISCARD and update the entire scene
        // environment everytime there's an update needed.
    // Regular MAP_WRITE (even in D3D11) is barely talked about at all and it's not well documented.
        // When mapping in D3D10/11 is talked about you usually talk about MAP_WRITE_DISCARD or NO_OVERWRITE
    COMCHECKCALL(hscene->dxcbvbuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>(&psceneenv)));
    PI_ASSERT(SUCCEEDED(hr));
        psceneenv->vpmatrix = *vpmatrix;
    hscene->dxcbvbuffer->Unmap();
}

void pi_D3D10Backend::bind_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept
{
    if (hscene != m_lastscene) {
        ID3D10Buffer* const buffers[] = {
            hscene ? hscene->dxcbvbuffer : NULL
        };
        m_dxdevice->VSSetConstantBuffers(0, PI_ARRAYSIZE(buffers), buffers);
        m_lastscene = hscene;
    }
}

void pi_D3D10Backend::begin_scene(PI_HRENDERBACKEND_SCENE hscene, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_VECTOR4* clear_rgba) noexcept
{
    this->bind_worldenv(hworldenv);
    this->bind_scene(hscene);

    PI_ASSERT(m_dxcurrentrtv != NULL);
    m_dxdevice->ClearRenderTargetView(m_dxcurrentrtv, clear_rgba->arr);
    if (m_dxcurrentdsv) {
        m_dxdevice->ClearDepthStencilView(
                m_dxcurrentdsv,
                D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, // Check if there's stenciling or not in the environment? Do we always need clearing in the world-env or can it be set by the user? (I think it should)
                1.0f, 0
            );
    }
}

void pi_D3D10Backend::end_scene() noexcept {
    ID3D10Resource* dxrender = NULL;
    m_dxcurrentrtv->GetResource(&dxrender);
    if (dxrender != m_dxswapchainbuffer) {
        m_dxdevice->ResolveSubresource(
                m_dxswapchainbuffer, 0,
                dxrender, 0,
                m_swapchainbufferdesc.Format
            );
    }
    else {
        m_dxcurrentrtv = NULL;
    }
    COMRELEASESAFE(dxrender);

    m_dxswapchain->Present(1, 0);
}
