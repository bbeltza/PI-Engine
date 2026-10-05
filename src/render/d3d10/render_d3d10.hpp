#ifndef PI_RENDER_D3D10_HPP
#define PI_RENDER_D3D10_HPP

#define PI_INCLUDE_BACKEND_API 1

#include <d3d10.h>
#include <piengine/pi_render.h>

#include <common/comhelpers.h>

#include <cstdio>
#include <map>

#ifndef PI_CPP
    #error "D3D10's backend is compiled in C++! Need a C++ compiler / include it in a .cpp file"
#endif

class PI_DLL
{
    HMODULE _hmodule = NULL;
public:
    PI_DLL(const PI_DLL&) = delete;
    PI_DLL(PI_DLL&& moving):
        _hmodule(moving._hmodule) {
            moving._hmodule = NULL;
        }

    constexpr PI_DLL() {}
    PI_DLL(LPCSTR lpLibFileName, HANDLE hFile = NULL, DWORD dwFlags = 0): _hmodule(LoadLibraryExA(lpLibFileName, hFile, dwFlags)) {}
    PI_DLL(LPCWSTR lpLibFileName, HANDLE hFile = NULL, DWORD dwFlags = 0): _hmodule(LoadLibraryExW(lpLibFileName, hFile, dwFlags)) {}

    ~PI_DLL() { FreeLibrary(_hmodule); }

    constexpr operator HMODULE() {
        return _hmodule;
    }
};

typedef HRESULT (WINAPI* PFN_D3D10_CREATE_DEVICE)(
        IDXGIAdapter*     pAdapter,
        D3D10_DRIVER_TYPE DriverType,
        HMODULE           Software,
        UINT              Flags,
        UINT              SDKVersion,
        ID3D10Device      **ppDevice
    );


#define pi_D3D10Backend PI_RENDERBACKEND
#define pi_D3D10WorldEnv PI_RENDERBACKEND_WORLDENV
#define pi_D3D10Scene PI_RENDERBACKEND_SCENE
#define pi_D3D10Mesh PI_RENDERBACKEND_MESH
#define pi_D3D10InstBuffer PI_RENDERBACKEND_INSTBUFFER


constexpr INT GET_SAMPLE_INDEX(UINT samples) {
    switch (samples)
    {
        case 2: return 0;
        case 4: return 1;
        case 8: return 2;
        case 16: return 3;
        case 32: return 4;

        default: return -1;
    }    
}

typedef UINT64 SAMPLE_HASH;
struct pi_D3D10sampledata
{
    ID3D10Texture2D* dxrender;
    ID3D10Texture2D* dxdepthstencil;
    ID3D10RenderTargetView* dxrtv;
    ID3D10DepthStencilView* dxdsv;

    pi_D3D10sampledata() = default;
    pi_D3D10sampledata(const pi_D3D10sampledata&) = delete;
    pi_D3D10sampledata(pi_D3D10sampledata&& other):
        dxrender(other.dxrender),
        dxdepthstencil(other.dxdepthstencil),
        dxrtv(other.dxrtv),
        dxdsv(other.dxdsv)
    {
        other.dxrender = NULL;
        other.dxdepthstencil = NULL;
        other.dxrtv = NULL;
        other.dxdsv = NULL;
    }

    ~pi_D3D10sampledata() {
        if (dxdepthstencil) {
            COMRELEASE(dxdsv);
            COMRELEASE(dxdepthstencil);
        }
        
        if (dxrender) {
            COMRELEASE(dxrtv);
            COMRELEASE(dxrender);
        }
    }
};

struct pi_D3D10Backend
{
    pi_D3D10Backend(const PI_BACKEND_RENDER_DESC& desc, PI_BACKEND_RENDER_INFO* pinfo);
    ~pi_D3D10Backend() noexcept;
private:
    bool initialize(const PI_BACKEND_RENDER_DESC& desc) noexcept;
    void release_objects() noexcept;

//

    void configure(const PI_RENDER_CONFIGURE* info) noexcept;

    bool create_worldenvironment(PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_WORLDENV_DESC* pdesc) noexcept;
    void destroy_worldenvironment(PI_HRENDERBACKEND_WORLDENV hworldenv) noexcept;

    bool create_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept;
    void destroy_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept;

    bool create_mesh(PI_HRENDERBACKEND_MESH hmesh, const PI_VERTEX* vertices, unsigned num_vertices, const void* indices, unsigned num_indices, PI_INDEX index_type) noexcept;
    void destroy_mesh(PI_HRENDERBACKEND_MESH hmesh) noexcept;

    bool create_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept;
    void destroy_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer) noexcept;

    bool alloc_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned size, const PI_INSTANCE initial_data[]) noexcept;
    bool update_instbuffer(PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned pos, unsigned size, const PI_INSTANCE data[]) noexcept;

    void update_scene_vp(PI_HRENDERBACKEND_SCENE hscene, const PI_MATRIX_4X4* vpmatrix) noexcept;
    void begin_scene(PI_HRENDERBACKEND_SCENE hscene, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_VECTOR4* clear_rgba) noexcept;
    void end_scene() noexcept;

    void submit_mesh(PI_HRENDERBACKEND_MESH hmesh, PI_HRENDERBACKEND_INSTBUFFER, unsigned instance_count) noexcept;
private:
    ID3D10Device* m_dxdevice = NULL;
    IDXGISwapChain* m_dxswapchain = NULL;
    ID3D10Texture2D* m_dxswapchainbuffer = NULL;

    ID3D10VertexShader* m_dx_vs_basic = NULL;
    ID3D10PixelShader* m_dx_ps_basic = NULL;
    
    ID3D10InputLayout* m_dxinputlayout = NULL;

    ID3D10RenderTargetView* m_dxcurrentrtv = NULL;
    ID3D10DepthStencilView* m_dxcurrentdsv = NULL;

    D3D10_TEXTURE2D_DESC m_swapchainbufferdesc;

    UINT m_maxsamplequalitylevels[GET_SAMPLE_INDEX(D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT) + 1];
    std::map<SAMPLE_HASH, pi_D3D10sampledata> m_samplemap;
private:
    PI_HRENDERBACKEND_SCENE m_lastscene = NULL;
    PI_HRENDERBACKEND_WORLDENV m_lastworldenv = NULL;
    void bind_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept;
    void bind_worldenv(PI_HRENDERBACKEND_WORLDENV) noexcept;
public:
    static size_t s_calcsize() noexcept;
private:
    static PI_DLL& _s_get_dll() noexcept;
    static PFN_D3D10_CREATE_DEVICE _s_pD3D10CreateDevice;
};

struct pi_D3D10WorldEnv
{
    ~pi_D3D10WorldEnv() noexcept;

    ID3D10RasterizerState* dxrasterizer;
    ID3D10DepthStencilState* dxdepthstencil;

    union {
        DXGI_SAMPLE_DESC sample_desc;
        SAMPLE_HASH sample_id;
    };
};

struct pi_D3D10Scene
{
    ~pi_D3D10Scene() noexcept;

    ID3D10Buffer* dxcbvbuffer = NULL; // Constant buffer
};

struct pi_D3D10Mesh
{
    ~pi_D3D10Mesh() noexcept;

    ID3D10Buffer* dxbuffer = NULL;
    DXGI_FORMAT index_format = DXGI_FORMAT_UNKNOWN;
    UINT index_offset = 0;
    UINT vertex_count = 0;
};

struct pi_D3D10InstBuffer
{
    ~pi_D3D10InstBuffer();

    ID3D10Buffer* dxbuffer = NULL;
    ID3D10Buffer* dxstagingbuffer = NULL;
};

//

extern "C" PI_BACKEND_RENDER _PI_BACKEND_RENDER_D3D10;

#endif
