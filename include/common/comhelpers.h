#ifndef COMHELPERS_H
#define COMHELPERS_H
#include <winerror.h>

#ifndef COM_LOGF
    #define COM_LOGF(x, ...) printf(x "\n", ##__VA_ARGS__)
#endif

#ifdef __cplusplus
    #define COMCALLFUNC(p, n, ...) (p)->n(__VA_ARGS__)
#else
    #define COMCALLFUNC(p, n, ...) (p)->lpVtbl->n(p, ##__VA_ARGS__)
#endif

#define COMRELEASE(p) COMCALLFUNC(p, Release)
#if !defined(__cplusplus) || defined(COMRELEASESAFE_USE_CMACRO)
    #define COMRELEASESAFE(p) do {              \
                            if (p) {            \
                                COMRELEASE(p);  \
                                p = NULL;       \
                            }                   \
                        } while (0)
#else
    template<typename T>
    inline void COMRELEASESAFE(T*& p) {
        if (p) {
            p->Release();
            p = NULL;
        }
    }
#endif

// Convert HRESULT error codes into a constant string
static inline const char* COMHRTOSTRING(HRESULT hr)
{
    #define _FMT_CASE(x) case x: return #x
    switch (hr)
    {
        #ifdef D3DERR_NOTAVAILABLE
            _FMT_CASE(D3DERR_NOTAVAILABLE);
            _FMT_CASE(D3DERR_DEVICELOST);
            _FMT_CASE(D3DERR_INVALIDCALL);
            _FMT_CASE(D3DERR_OUTOFVIDEOMEMORY);
        #endif

        #ifdef DXGI_ERROR_ACCESS_DENIED
            _FMT_CASE(DXGI_ERROR_ACCESS_DENIED);
            _FMT_CASE(DXGI_ERROR_ACCESS_LOST);
            _FMT_CASE(DXGI_ERROR_ALREADY_EXISTS);
            _FMT_CASE(DXGI_ERROR_CANNOT_PROTECT_CONTENT);
            _FMT_CASE(DXGI_ERROR_DEVICE_HUNG);
            _FMT_CASE(DXGI_ERROR_DEVICE_REMOVED);
            _FMT_CASE(DXGI_ERROR_DEVICE_RESET);
            _FMT_CASE(DXGI_ERROR_DRIVER_INTERNAL_ERROR);
            _FMT_CASE(DXGI_ERROR_FRAME_STATISTICS_DISJOINT);
            _FMT_CASE(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE);
            _FMT_CASE(DXGI_ERROR_INVALID_CALL);
            _FMT_CASE(DXGI_ERROR_MORE_DATA);
            _FMT_CASE(DXGI_ERROR_NAME_ALREADY_EXISTS);
            _FMT_CASE(DXGI_ERROR_NONEXCLUSIVE);
            _FMT_CASE(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE);
            _FMT_CASE(DXGI_ERROR_NOT_FOUND);
            _FMT_CASE(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE);
            _FMT_CASE(DXGI_ERROR_SDK_COMPONENT_MISSING);
            _FMT_CASE(DXGI_ERROR_SESSION_DISCONNECTED);
            _FMT_CASE(DXGI_ERROR_UNSUPPORTED);
            _FMT_CASE(DXGI_ERROR_WAIT_TIMEOUT);
            _FMT_CASE(DXGI_ERROR_WAS_STILL_DRAWING);
        #endif

        #ifdef D3D11_ERROR_FILE_NOT_FOUND
            _FMT_CASE(D3D11_ERROR_FILE_NOT_FOUND);
            _FMT_CASE(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
            _FMT_CASE(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS);
            _FMT_CASE(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD);
        #endif

        #ifdef D3D12_ERROR_ADAPTER_NOT_FOUND
            _FMT_CASE(D3D12_ERROR_ADAPTER_NOT_FOUND);
            _FMT_CASE(D3D12_ERROR_DRIVER_VERSION_MISMATCH);
        #endif

        _FMT_CASE(E_FAIL);
        _FMT_CASE(E_INVALIDARG);
        _FMT_CASE(E_OUTOFMEMORY);
        _FMT_CASE(E_NOTIMPL);
        _FMT_CASE(E_NOINTERFACE);
        _FMT_CASE(S_FALSE);
        _FMT_CASE(S_OK);

        #ifdef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
            _FMT_CASE(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED);
            _FMT_CASE(DXGI_ERROR_REMOTE_OUTOFMEMORY);
        #endif
        default: return 0;
    }
    #undef _FMT_CASE
}

#define COMLOGFAILHR(hr, call) COM_LOGF("[COMLOGFAILHR]: \"%s\" failed: \"%s\" (code: 0x%08lx) (file: \"%s\" | line: %d)", call, COMHRTOSTRING(hr), hr, __FILE__, __LINE__)

#ifndef COMCHECKHR_EX /* "#define COMCHECKHR_EX(x, hr) " -> Define this before including this header if you don't want any debug checks when using the macro */
    #define COMCHECKHR_EX(hr, call) do { if (FAILED(hr)) { \
            COMLOGFAILHR(hr, call);                        \
        } } while (0)
#endif

#define COMCHECKCALL_EX(x, hr) hr = x; COMCHECKHR_EX(hr, #x)

/* Assume that an HRESULT variable named "hr" is defined */
#define COMCHECKCALL(x) COMCHECKCALL_EX(x, hr)
#define COMCHECKHR(call) COMCHECKHR_EX(hr, call)

#endif
