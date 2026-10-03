#include "render_d3d10.hpp"

static inline D3D10_COMPARISON_FUNC d3d10_compare_from_pi(PI_COMPARE_FUNC func)
{
    switch (func) {
        case PI_COMPARE_NEVER: return D3D10_COMPARISON_NEVER;
        case PI_COMPARE_LESS: return D3D10_COMPARISON_LESS;
        case PI_COMPARE_EQUAL: return D3D10_COMPARISON_EQUAL;
        case PI_COMPARE_LESS_EQUAL: return D3D10_COMPARISON_LESS_EQUAL;
        case PI_COMPARE_GREATER: return D3D10_COMPARISON_GREATER;
        case PI_COMPARE_NOT_EQUAL: return D3D10_COMPARISON_NOT_EQUAL;
        case PI_COMPARE_GREATER_EQUAL: return D3D10_COMPARISON_GREATER_EQUAL;
        case PI_COMPARE_ALWAYS: return D3D10_COMPARISON_ALWAYS;
    }

    return D3D10_COMPARISON_LESS;
}

bool pi_D3D10Backend::create_worldenvironment(PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_WORLDENV_DESC* pdesc) noexcept
{
    HRESULT hr;
    new (hworldenv) pi_D3D10WorldEnv;
    
    if (pdesc->multisample->samples > D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT) {
        return false;
    }

    UINT maxqualitylevel = m_maxsamplequalitylevels[pdesc->multisample->samples - 1];
    if (!maxqualitylevel) {
        return false;
    }

    hworldenv->sample_desc.Count = pdesc->multisample->samples;
    hworldenv->sample_desc.Quality = pdesc->multisample->best_quality ?
                                        maxqualitylevel - 1 :
                                        0; // If only quality 0 is supported, enabling `best_quality` has the same effect as not enabling it.

    {
        D3D10_RASTERIZER_DESC desc = {};
        switch (pdesc->rasterizer->fillmode) {
            case PI_FILL_SOLID: desc.FillMode = D3D10_FILL_SOLID; break;
            case PI_FILL_WIREFRAME: desc.FillMode = D3D10_FILL_WIREFRAME; break;
        }
            if (pdesc->rasterizer->cullflags & PI_CULL_FRONT_COUNTERCLOCKWISE) {
            desc.FrontCounterClockwise = TRUE;
        }

        if (!(pdesc->rasterizer->cullflags & (PI_CULL_FRONT | PI_CULL_BACK)))
            desc.CullMode = D3D10_CULL_NONE;
        else if (pdesc->rasterizer->cullflags & PI_CULL_BACK)
            desc.CullMode = D3D10_CULL_BACK;
        else if (pdesc->rasterizer->cullflags & PI_CULL_FRONT)
            desc.CullMode = D3D10_CULL_FRONT;
        else
            __assume(0);

        COMCHECKCALL(m_dxdevice->CreateRasterizerState(&desc, &hworldenv->dxrasterizer));
        if (FAILED(hr)) {
            return false;
        }
    }

    if (pdesc->depth)
    {
        D3D10_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = TRUE; 
        desc.DepthWriteMask = pdesc->depth->writeenable ? D3D10_DEPTH_WRITE_MASK_ALL : D3D10_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = d3d10_compare_from_pi(pdesc->depth->depthcompare);

        COMCHECKCALL(m_dxdevice->CreateDepthStencilState(&desc, &hworldenv->dxdepthstencil));
        if (FAILED(hr)) {
            COMRELEASE(hworldenv->dxrasterizer);
            return false;
        }
    }

    return true;
}

void pi_D3D10Backend::bind_worldenv(PI_HRENDERBACKEND_WORLDENV hworldenv) noexcept {
    if (!hworldenv) {
        hworldenv = NULL;
        m_dxcurrentrtv = NULL;
        m_dxcurrentdsv = NULL;
        m_dxdevice->OMSetRenderTargets(0, NULL, NULL);
        m_dxdevice->RSSetState(NULL);
        m_dxdevice->OMSetDepthStencilState(NULL, 0);

        return;
    }

    PI_ASSERT(m_dxswapchainbuffer != NULL);

    auto sampledata = m_samplemap.find(hworldenv->sample_id);
    HRESULT hr;
    if (sampledata == m_samplemap.end())
    {
        auto inserted = m_samplemap.emplace(hworldenv->sample_id, pi_D3D10sampledata{});
        sampledata = inserted.first;

        // Create the necessary resources (Only the render target here)
        auto& sd = sampledata->second;
        sd.dxdepthstencil = NULL;
        sd.dxdsv = NULL;

        if (hworldenv->sample_desc.Count == 1) { // Use the swap-chain texture directly if there's no MSAA
            sd.dxrender = m_dxswapchainbuffer;
            m_dxswapchainbuffer->AddRef();
            COMCHECKCALL(m_dxdevice->CreateRenderTargetView(sd.dxrender, NULL, &sd.dxrtv));
        }
        else {
            D3D10_TEXTURE2D_DESC desc = m_swapchainbufferdesc; 
            desc.SampleDesc = hworldenv->sample_desc;
            COMCHECKCALL(m_dxdevice->CreateTexture2D(&desc, NULL, &sd.dxrender));
            COMCHECKCALL(m_dxdevice->CreateRenderTargetView(sd.dxrender, NULL, &sd.dxrtv));
        }
    }

    auto& sd = sampledata->second;
    if (hworldenv->dxdepthstencil && !sd.dxdepthstencil) {
        D3D10_TEXTURE2D_DESC desc{};
        desc.Width = m_swapchainbufferdesc.Width;
        desc.Height = m_swapchainbufferdesc.Height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc = hworldenv->sample_desc;
        desc.Usage = D3D10_USAGE_DEFAULT;
        desc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
        COMCHECKCALL(m_dxdevice->CreateTexture2D(&desc, NULL, &sd.dxdepthstencil));
        COMCHECKCALL(m_dxdevice->CreateDepthStencilView(sd.dxdepthstencil, NULL, &sd.dxdsv));
    }

    ID3D10DepthStencilView* dsv = hworldenv->dxdepthstencil ? sd.dxdsv : NULL;
    if (m_dxcurrentrtv != sd.dxrtv || m_dxcurrentdsv != dsv) {
        m_dxcurrentrtv = sd.dxrtv;
        m_dxcurrentdsv = dsv;

        m_dxdevice->OMSetRenderTargets(1, &sd.dxrtv, dsv);
    }

    if (m_lastworldenv != hworldenv)
    {
        m_dxdevice->RSSetState(hworldenv->dxrasterizer);
        m_dxdevice->OMSetDepthStencilState(hworldenv->dxdepthstencil, 0);
        m_lastworldenv = hworldenv;
    }
}

void pi_D3D10Backend::destroy_worldenvironment(PI_HRENDERBACKEND_WORLDENV hworldenv) noexcept {
    if (hworldenv == m_lastworldenv) {
        bind_worldenv(NULL);
    }

    hworldenv->~pi_D3D10WorldEnv();
}

pi_D3D10WorldEnv::~pi_D3D10WorldEnv() noexcept
{
    COMRELEASE(dxdepthstencil);
    COMRELEASE(dxrasterizer);
}
