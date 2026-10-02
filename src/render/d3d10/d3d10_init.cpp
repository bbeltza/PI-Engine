#include "render_d3d10.hpp"

#include <SDL3/SDL_video.h>

#include <cstdio>

namespace PI_D3D10SHADERS {
    #include "shaders.h"
}

//

PFN_D3D10_CREATE_DEVICE pi_D3D10Backend::_s_pD3D10CreateDevice;

PI_DLL& pi_D3D10Backend::_s_get_dll() noexcept {
    static PI_DLL dll{"d3d10.dll"};
    return dll;
}

constexpr UINT DEVICE_FLAGS = D3D10_CREATE_DEVICE_SINGLETHREADED;
constexpr UINT DEVICE_BACKBUFFERS = 1; // 1 for now should be fine right??
constexpr DXGI_FORMAT DEVICE_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;

size_t pi_D3D10Backend::s_calcsize() noexcept
{
    auto& dll = _s_get_dll();
    // _s_pD3D10CreateDevice = D3D10CreateDevice;
    _s_pD3D10CreateDevice = reinterpret_cast<PFN_D3D10_CREATE_DEVICE>(GetProcAddress(dll, "D3D10CreateDevice"));
    PI_ASSERT(_s_pD3D10CreateDevice);
    if (!_s_pD3D10CreateDevice) {
        return 0;
    }

    HRESULT hr;
    ID3D10Device* dxdevice = NULL;
    COMCHECKCALL(_s_pD3D10CreateDevice(
            NULL,
            D3D10_DRIVER_TYPE_HARDWARE,
            NULL,
            DEVICE_FLAGS,
            D3D10_SDK_VERSION,
            &dxdevice
        ));
    if (FAILED(hr)) {
        system("pause");
        return 0;
    }

    dxdevice->Release();
    return sizeof(pi_D3D10Backend);
}

//

bool pi_D3D10Backend::initialize(const PI_BACKEND_RENDER_DESC& desc) noexcept
{
    PI_ASSERT(_s_pD3D10CreateDevice);
    

    HWND hwnd = NULL;
    {
        SDL_PropertiesID sdl_windowproperties = SDL_GetWindowProperties(desc.sdl_window);
        if (!sdl_windowproperties) {
            std::printf("SDL_GetWindowProperties failed: \"%s\"\n", SDL_GetError());
            return false;
        }
        void* phwnd = SDL_GetPointerProperty(sdl_windowproperties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (!phwnd) {
            return false;
        }
        hwnd = static_cast<HWND>(phwnd);
    }

    HRESULT hr;
    COMCHECKCALL(_s_pD3D10CreateDevice(
            NULL,
            D3D10_DRIVER_TYPE_HARDWARE,
            NULL,
            DEVICE_FLAGS,
            D3D10_SDK_VERSION,
            &m_dxdevice
        ));


    {
        IDXGIDevice* dxgidevice = NULL;
        IDXGIAdapter* dxgiadapter = NULL;
        IDXGIFactory* dxgifactory = NULL;
        COMCHECKCALL(m_dxdevice->QueryInterface(&dxgidevice));

        COMCHECKCALL(dxgidevice->GetAdapter(&dxgiadapter));
        COMCHECKCALL(dxgiadapter->GetParent(IID_PPV_ARGS(&dxgifactory)));

        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferDesc.Format = DEVICE_FORMAT;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
        desc.SampleDesc = { 1, 0 };
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = DEVICE_BACKBUFFERS;
        desc.OutputWindow = hwnd;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
        desc.Flags = 0;
        COMCHECKCALL(dxgifactory->CreateSwapChain(m_dxdevice, &desc, &m_dxswapchain));
        PI_ASSERT(!FAILED(hr));

        COMCHECKCALL(dxgifactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES));

        /* If the IDXGIDevice1 interface is supported... */
        #ifdef __IDXGIDevice1_INTERFACE_DEFINED__
        {
            IDXGIDevice1* dxgidevice1 = NULL;
            COMCHECKCALL(dxgidevice->QueryInterface(&dxgidevice1));
            if (dxgidevice1) {
                COMCHECKCALL(dxgidevice1->SetMaximumFrameLatency(1));
                dxgidevice1->Release();
            }
        }
        #endif

        dxgifactory->Release();
        dxgiadapter->Release();
        dxgidevice->Release();
    }

    using namespace PI_D3D10SHADERS;
    COMCHECKCALL(m_dxdevice->CreateVertexShader(vs_basic, sizeof(vs_basic), &m_dx_vs_basic));
    COMCHECKCALL(m_dxdevice->CreatePixelShader(ps_basic, sizeof(ps_basic), &m_dx_ps_basic));

    {
        static const LPCSTR POSITION = "POSITION";
        static const LPCSTR COLOR = "COLOR";

        D3D10_INPUT_ELEMENT_DESC inputs[] = {
            D3D10_INPUT_ELEMENT_DESC{ POSITION, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA },
            D3D10_INPUT_ELEMENT_DESC{ COLOR, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_VERTEX_DATA },

            D3D10_INPUT_ELEMENT_DESC{ POSITION, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
            D3D10_INPUT_ELEMENT_DESC{ POSITION, 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
            D3D10_INPUT_ELEMENT_DESC{ POSITION, 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
            D3D10_INPUT_ELEMENT_DESC{ POSITION, 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
            D3D10_INPUT_ELEMENT_DESC{COLOR, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D10_APPEND_ALIGNED_ELEMENT, D3D10_INPUT_PER_INSTANCE_DATA, 1 }
        };

        COMCHECKCALL(m_dxdevice->CreateInputLayout(inputs, PI_ARRAYSIZE(inputs), vs_basic, sizeof(vs_basic), &m_dxinputlayout));
    }

    m_dxdevice->VSSetShader(m_dx_vs_basic);
    m_dxdevice->PSSetShader(m_dx_ps_basic);
    m_dxdevice->IASetInputLayout(m_dxinputlayout);
    m_dxdevice->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}

void pi_D3D10Backend::release_objects() noexcept
{
    COMRELEASESAFE(m_dxrtv);
    COMRELEASE(m_dxinputlayout);
    COMRELEASE(m_dx_ps_basic);
    COMRELEASE(m_dx_vs_basic);
    COMRELEASE(m_dxswapchain);
    COMRELEASE(m_dxdevice);
}

void pi_D3D10Backend::configure(const PI_RENDER_CONFIGURE* info) noexcept
{
    HRESULT hr;
    COMRELEASESAFE(m_dxrtv);

    COMRELEASESAFE(m_dxdsv);
    COMRELEASESAFE(m_dxdepthstencil);

    COMCHECKCALL(m_dxswapchain->ResizeBuffers(DEVICE_BACKBUFFERS, info->w, info->h, DXGI_FORMAT_UNKNOWN, 0));
    PI_ASSERT(!FAILED(hr));

    {
        D3D10_TEXTURE2D_DESC dsdesc = {};
        dsdesc.Width = info->w;
        dsdesc.Height = info->h;
        dsdesc.MipLevels = 1;
        dsdesc.ArraySize = 1;
        dsdesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsdesc.SampleDesc = { 1, 0 };
        dsdesc.Usage = D3D10_USAGE_DEFAULT;
        dsdesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;

        COMCHECKCALL(m_dxdevice->CreateTexture2D(&dsdesc, NULL, &m_dxdepthstencil));
        PI_ASSERT(!FAILED(hr));
    }

    ID3D10Resource* backbuffer = NULL;
    COMCHECKCALL(m_dxswapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));

    D3D10_RENDER_TARGET_VIEW_DESC rtvdesc = {};
    rtvdesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
    rtvdesc.Format = DEVICE_FORMAT;
    COMCHECKCALL(m_dxdevice->CreateRenderTargetView(backbuffer, &rtvdesc, &m_dxrtv));

    COMCHECKCALL(m_dxdevice->CreateDepthStencilView(m_dxdepthstencil, NULL, &m_dxdsv));

    D3D10_VIEWPORT viewport{};
    viewport.Width = info->w;
    viewport.Height = info->h;
    viewport.MaxDepth = 1.0f;
    m_dxdevice->RSSetViewports(1, &viewport);

    COMRELEASE(backbuffer);
}
