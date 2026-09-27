#ifndef PI_RENDER_D3D10_HPP
#define PI_RENDER_D3D10_HPP

#define PI_INCLUDE_BACKEND_API 1

#include <d3d10.h>
#include <piengine/pi_render.h>

#ifndef PI_CPP
    #error "D3D10's backend is compiled in C++! Need a C++ compiler / include it in a .cpp file"
#endif

#define pi_D3D10Backend PI_RENDERBACKEND

struct pi_D3D10Backend
{
    pi_D3D10Backend(const PI_BACKEND_RENDER_DESC& desc, PI_BACKEND_RENDER_CALLBACKS* pcallbacks);
    ~pi_D3D10Backend();
private:
    bool initialize(const PI_BACKEND_RENDER_DESC& desc);

    static void _s_close(pi_D3D10Backend* backend) noexcept;

    ID3D10Device* m_dxdevice;
    IDXGISwapChain* m_dxswapchain;
};

extern "C" PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10;

#endif
