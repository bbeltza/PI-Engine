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
    
    {
        D3D10_RASTERIZER_DESC desc = {};
        switch (pdesc->rasterizer.fillmode) {
            case PI_FILL_SOLID: desc.FillMode = D3D10_FILL_SOLID; break;
            case PI_FILL_WIREFRAME: desc.FillMode = D3D10_FILL_WIREFRAME; break;
        }
        if (pdesc->rasterizer.cullflags & PI_CULL_FRONT_COUNTERCLOCKWISE) {
            desc.FrontCounterClockwise = TRUE;
        }

        if (!(pdesc->rasterizer.cullflags & (PI_CULL_FRONT | PI_CULL_BACK)))
            desc.CullMode = D3D10_CULL_NONE;
        else if (pdesc->rasterizer.cullflags & PI_CULL_BACK)
            desc.CullMode = D3D10_CULL_BACK;
        else if (pdesc->rasterizer.cullflags & PI_CULL_FRONT)
            desc.CullMode = D3D10_CULL_FRONT;
        else
            __assume(0);

        COMCHECKCALL(m_dxdevice->CreateRasterizerState(&desc, &hworldenv->dxrasterizer));
        if (FAILED(hr)) {
            return false;
        }
    }

    {
        D3D10_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = TRUE; // Temporarely always TRUE, until I set up a way to disable depth testing... (specially by default)
        desc.DepthWriteMask = pdesc->depth.writeenable ? D3D10_DEPTH_WRITE_MASK_ALL : D3D10_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = d3d10_compare_from_pi(pdesc->depth.depthcompare);

        COMCHECKCALL(m_dxdevice->CreateDepthStencilState(&desc, &hworldenv->dxdepthstencil));
        if (FAILED(hr)) {
            COMRELEASE(hworldenv->dxrasterizer);
            return false;
        }
    }

    return true;
}

void pi_D3D10Backend::bind_worldenv(PI_HRENDERBACKEND_WORLDENV hworldenv) noexcept {
    if (m_lastworldenv != hworldenv)
    {
        m_dxdevice->RSSetState(hworldenv ? hworldenv->dxrasterizer : NULL);
        m_dxdevice->OMSetDepthStencilState(hworldenv ? hworldenv->dxdepthstencil : NULL, 0);
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
