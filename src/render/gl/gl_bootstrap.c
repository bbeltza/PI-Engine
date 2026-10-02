#include "render_gl.h"

static size_t s_gl_calcsize(void);
static bool s_gl_entry(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinfo);

const PI_BACKEND_RENDER _PI_BACKEND_RENDER_GL = {
    "OpenGL Core",

    s_gl_calcsize,
    s_gl_entry
};

static size_t s_gl_calcsize(void) {
    return sizeof(pi_GLbackend);
}

static bool s_gl_entry(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinfo)
{
    if (!__pi_render_gl_initialize(hbackend, pdesc)) {
        return false;
    }

    pinfo->callbacks.pfn_close = __pi_render_gl_close;

    ASSERT_BACKEND_CALLBACKS(pinfo->callbacks);
    DEFINE_BACKEND_STRUCTURES(pinfo->structures);

    return true;
}
