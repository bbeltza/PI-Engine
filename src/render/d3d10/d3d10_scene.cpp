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

void pi_D3D10Backend::update_scene_lightproperties(PI_HRENDERBACKEND_SCENE hscene, const PI_LIGHT_PROPERTIES* properties) noexcept {
    HRESULT hr;
    if (!hscene->dxcbpsbuffer) {
        D3D10_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(PI_LIGHT_PROPERTIES);
        desc.Usage = D3D10_USAGE_DYNAMIC;
        desc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
        D3D10_SUBRESOURCE_DATA initialdata = {};
        initialdata.pSysMem = properties;
        COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, &initialdata, &hscene->dxcbpsbuffer));
        return;
    }

    PI_LIGHT_PROPERTIES* pproperties = NULL;
    COMCHECKCALL(hscene->dxcbpsbuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void**>(&pproperties)));
    PI_ASSERT(SUCCEEDED(hr));
        *pproperties = *properties;
    hscene->dxcbpsbuffer->Unmap();
    
}

void pi_D3D10Backend::bind_scene(PI_HRENDERBACKEND_SCENE hscene) noexcept
{
    if (hscene != m_lastscene) {
        ID3D10Buffer* const vsbuffers[] = {
            hscene ? hscene->dxcbvbuffer : NULL,
            hscene ? hscene->dxcblightbuffer : NULL
        };
        ID3D10Buffer* const psbuffers[] = {
            hscene ? hscene->dxcbpsbuffer : NULL,
            hscene ? hscene->dxcblightbuffer : NULL
        };
        m_dxdevice->VSSetConstantBuffers(0, PI_ARRAYSIZE(vsbuffers), vsbuffers);
        m_dxdevice->PSSetConstantBuffers(0, PI_ARRAYSIZE(psbuffers), psbuffers);
        
        m_lastscene = hscene;
    }
}

void pi_D3D10Backend::begin_scene(PI_HRENDERBACKEND_SCENE hscene, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_VECTOR4* clear_rgba) noexcept
{
    PI_ASSERT((hscene && hworldenv) || (!hscene && !hworldenv));
    PI_ASSERT(hscene->dxcblightbuffer || hworldenv->shader == &m_shaders[0]);

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

//

#include "shader_properties.h"

struct PI_LIGHTGROUP {
    PI_DIRECTIONAL_LIGHT dirlight;
    PI_POINT_LIGHT ptlight;
};

struct PI_LIGHTCB {
    unsigned num_dirlights;
    unsigned num_ptlights;
    unsigned _padding[2];

    PI_LIGHTGROUP lights[MAX_COMBINED_LIGHTS];
};

void pi_D3D10Backend::update_lights(PI_HRENDERBACKEND_SCENE hscene, PI_LIGHTTYPE type, unsigned update_start, unsigned update_count, const PI_LIGHT lights[], unsigned full_light_count) noexcept
{
    HRESULT hr;

    if (!hscene->dxcblightbuffer) {
        PI_ASSERT(hscene->dxcblightstagingbuffer == NULL);
        PI_ASSERT(update_start == 0); // The first call to `update_start` should update from the beginning of the light buffer
        
        D3D10_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(PI_LIGHTCB);
        desc.Usage = D3D10_USAGE_DEFAULT;
        desc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, NULL, &hscene->dxcblightbuffer));

        desc.Usage = D3D10_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
        COMCHECKCALL(m_dxdevice->CreateBuffer(&desc, NULL, &hscene->dxcblightstagingbuffer));
    }

    PI_ASSUME(hscene->dxcblightbuffer != NULL);
    PI_ASSUME(hscene->dxcblightstagingbuffer != NULL);

    PI_LIGHTCB* plcb = NULL;
    COMCHECKCALL(hscene->dxcblightstagingbuffer->Map(D3D10_MAP_WRITE, 0, reinterpret_cast<void**>(&plcb)));
        switch (type) {
            case PI_LIGHT_DIRECTIONAL:
                if (hscene->last_dirlightcount != full_light_count)
                    plcb->num_dirlights = full_light_count;
                for (unsigned i = 0, ii = update_start; i < update_count; i++, ii++) {
                    plcb->lights[ii].dirlight = lights[i].directional;
                }
                break;
            case PI_LIGHT_POINT:
                if (hscene->last_ptlightcount != full_light_count)
                    plcb->num_ptlights = full_light_count;
                for (unsigned i = 0, ii = update_start; i < update_count; i++, ii++) {
                    plcb->lights[ii].ptlight = lights[i].point;
                }
                break;
            default:
                PI_ASSUME(0);
        }
    hscene->dxcblightstagingbuffer->Unmap();

    UINT firstx = UINT_MAX;
    switch (type) {
        case PI_LIGHT_DIRECTIONAL: if (hscene->last_dirlightcount != full_light_count) {
            firstx = offsetof(PI_LIGHTCB, num_dirlights);
            hscene->last_dirlightcount = full_light_count;
        } break;
        case PI_LIGHT_POINT: if (hscene->last_ptlightcount != full_light_count) {
            firstx = offsetof(PI_LIGHTCB, num_ptlights);
            hscene->last_ptlightcount = full_light_count;
        } break;
        default: PI_ASSUME(0);
    }

    if (firstx != UINT_MAX) {
        D3D10_BOX box;
        box.left = firstx;
        box.right = firstx + sizeof(unsigned);
        box.top = 0;
        box.bottom = 1;
        box.front = 0;
        box.back = 1;
        
        m_dxdevice->CopySubresourceRegion(
                hscene->dxcblightbuffer, 0,
                firstx, 0, 0,
                hscene->dxcblightstagingbuffer, 0,
                &box);
    }
    
    D3D10_BOX box;
    box.left = offsetof(PI_LIGHTCB, lights) + update_start*sizeof(PI_LIGHTGROUP);
    box.right = box.left + update_count*sizeof(PI_LIGHTGROUP);
    box.top = 0;
    box.bottom = 1;
    box.front = 0;
    box.back = 1;
    m_dxdevice->CopySubresourceRegion(
        hscene->dxcblightbuffer, 0,
        box.left, 0, 0,
        hscene->dxcblightstagingbuffer, 0,
        &box);
}
