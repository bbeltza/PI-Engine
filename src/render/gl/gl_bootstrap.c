#include "render_gl.h"

static size_t s_gl_calcsize(void);
static bool s_gl_entry(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinfo);

const PI_BACKEND_RENDER _PI_BACKEND_RENDER_GL = {
    "OpenGL Core",

    s_gl_calcsize,
    s_gl_entry
};

static size_t s_gl_calcsize(void) {
    // Some attributes must be set before creating the window
    // We don't wanna pollute main.c too much either... So...
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, 1);

    return sizeof(pi_GLbackend);
}

static bool s_gl_entry(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinfo)
{
    if (!fn_render_gl_initialize(hbackend, pdesc)) {
        return false;
    }

    pinfo->callbacks.pfn_close = fn_render_gl_close;
    pinfo->callbacks.pfn_configure = fn_render_gl_configure;
    pinfo->callbacks.pfn_create_worldenvironment = fn_render_gl_create_worldenvironment;
    pinfo->callbacks.pfn_destroy_worldenvironment = fn_render_gl_destroy_worldenvironment;
    pinfo->callbacks.pfn_create_scene = fn_render_gl_create_scene;
    pinfo->callbacks.pfn_destroy_scene = fn_render_gl_destroy_scene;
    pinfo->callbacks.pfn_create_mesh = fn_render_gl_create_mesh;
    pinfo->callbacks.pfn_destroy_mesh = fn_render_gl_destroy_mesh;
    pinfo->callbacks.pfn_create_instbuffer = fn_render_gl_create_instbuffer;
    pinfo->callbacks.pfn_destroy_instbuffer = fn_render_gl_destroy_instbuffer;
    pinfo->callbacks.pfn_alloc_instbuffer = fn_render_gl_alloc_instbuffer;
    pinfo->callbacks.pfn_update_instbuffer = fn_render_gl_update_instbuffer;
    pinfo->callbacks.pfn_update_scene_vp = fn_render_gl_update_scene_vp;
    pinfo->callbacks.pfn_begin_scene = fn_render_gl_begin_scene;    
    pinfo->callbacks.pfn_end_scene = fn_render_gl_end_scene;
    pinfo->callbacks.pfn_submit_mesh = fn_render_gl_submit_mesh;    

    ASSERT_BACKEND_CALLBACKS(pinfo->callbacks);
    DEFINE_BACKEND_STRUCTURES(pinfo->structures);

    return true;
}
