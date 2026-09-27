#include "render_d3d10.hpp"

class BACKEND_FAILURE {};

PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10 = {
    "Direct3D 10",
    sizeof(pi_D3D10Backend),
    [](PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_CALLBACKS* pcallbacks) -> bool {
        try {
            new (hbackend) pi_D3D10Backend{ *pdesc, pcallbacks };
        }
        catch (BACKEND_FAILURE& e) {
            return false;
        }

        return true;
    }
};

pi_D3D10Backend::pi_D3D10Backend(const PI_BACKEND_RENDER_DESC& desc, PI_BACKEND_RENDER_CALLBACKS* pcallbacks):
    m_dxdevice( NULL ),
    m_dxswapchain( NULL )
{
    if (!initialize(desc)) {
        throw BACKEND_FAILURE{};
    }

    //#define STORE_CALLBACK(n) pcallbacks->pfn_##n = _s_##n
    pcallbacks->pfn_close = [](PI_HRENDERBACKEND hbackend) noexcept -> void {
        hbackend->~PI_RENDERBACKEND();
    };
}
