#include "render_gl.h"

#include <piengine/pi_math.h>
#include <piengine/pi_core.h>

PI_EXTERN_C bool fn_render_gl_initialize(pi_GLbackend* hbackend, const PI_BACKEND_RENDER_DESC* pdesc)
{
    /* Use OpenGL 3, minor version isn't included, should that be fine??? */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* sdlwnd = pi_get_window_sdl_handle(pdesc->hwindow);
    hbackend->glcontext = SDL_GL_CreateContext(sdlwnd);
    if (!hbackend->glcontext) {
        return false;
    }

    if (!pi_gl_render_loadfuncs((PI_PFN_GLGETPROCADDRESS)SDL_GL_GetProcAddress, malloc, &hbackend->glfuncs)) {
        goto CLEANUPANDFAIL;
    }
    
#if USE_GL_DBGTRAMPOLINE
    if (!pi_gl_init_trampoline(hbackend)) {
        goto CLEANUPANDFAIL;
    }
#endif

    {
        GLint ubosize, uboalign;
        gl.GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &ubosize);
        gl.GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboalign);

        hbackend->ubofsoffset = PI_ALIGN(sizeof(PI_SCENEENV), uboalign);
        hbackend->maxuboptlights = PI_MIN(ubosize/sizeof(PI_POINT_LIGHT), 1024); 
        hbackend->maxubodirlights = PI_MIN(ubosize/sizeof(PI_DIRECTIONAL_LIGHT), 1024); 
    }

    const char* const _defines[NUM_SHADERS] = {
        "",
        "#define WORLDEFFECT_LIGHTING\n"
            "#define MAX_DIRECTIONAL_LIGHTS %u\n"
            "#define MAX_POINT_LIGHTS %u\n",
        "%s\n"
            "#define WORLDEFFECT_SPECULAR\n"
    };
   
    char defines1buf[255];
    char defines2buf[255];

    const char* defines[NUM_SHADERS] = {
        _defines[0],
        defines1buf,
        defines2buf
    };
    snprintf(defines1buf, PI_ARRAYSIZE(defines1buf), _defines[1], hbackend->maxubodirlights, hbackend->maxuboptlights);
    snprintf(defines2buf, PI_ARRAYSIZE(defines2buf), _defines[2], defines1buf);

    for (unsigned i = 0; i < NUM_SHADERS; i++) {
        GLuint shader = pi_gl_compile_shader(hbackend, defines[i]);
        if (shader == 0) {
            goto CLEANUPANDFAIL;
        }
        hbackend->shaders[i] = shader;
    }

    gl.Enable(GL_FRAMEBUFFER_SRGB);
    gl.GetIntegerv(GL_MAX_SAMPLES, &hbackend->msaasamples);
    if (!hbackend->msaasamples) {
        hbackend->msaasamples = 0;
        hbackend->msaabuffers = NULL;
    }
    else {
        PI_ASSERT(hbackend->msaasamples > 0);
        hbackend->msaasamples++;
        hbackend->msaabuffers = calloc(hbackend->msaasamples, sizeof(GLmsaabuffer));
        if (!hbackend->msaabuffers) {
            /* Fail, or don't support mutisampling??? */
            goto CLEANUPANDFAIL;
            //hbackend->msaasamples = 0;
        }
        
        for (GLint i = 0; i < hbackend->msaasamples; i++) {
            gl.GenFramebuffers(1, &hbackend->msaabuffers[i].fbo);
        }
    }

    SDL_GL_SetSwapInterval(1);
    return true;
CLEANUPANDFAIL:
    fn_render_gl_close(hbackend);
    return false;
}

PI_EXTERN_C void fn_render_gl_close(pi_GLbackend* hbackend)
{
#if USE_GL_DBGTRAMPOLINE
    pi_gl_close_trampoline(hbackend);
#endif

    free(hbackend->glfuncs);
    SDL_GL_DestroyContext(hbackend->glcontext);
}

bool fn_render_gl_configure(pi_GLbackend* hbackend, const PI_RENDER_CONFIGURE* info)
{
    gl.Viewport(0, 0, info->w, info->h);
    hbackend->wndwidth = info->w;
    hbackend->wndheight = info->h;

    /* Delete all existing multisample buffers, they can get created back on begin_scene depending on the multisample setting of the world environment bound */
    for (GLint i = 0; i < hbackend->msaasamples; i++) {
        GLmsaabuffer* msaabuffer = &hbackend->msaabuffers[i];
        gl.DeleteRenderbuffers(PI_ARRAYSIZE(msaabuffer->rbos), msaabuffer->rbos);
        memset(msaabuffer->rbos, 0, sizeof(msaabuffer->rbos));
    }
    
    return true;
}
