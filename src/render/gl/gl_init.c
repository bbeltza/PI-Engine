#include "render_gl.h"


PI_EXTERN_C bool fn_render_gl_initialize(pi_GLbackend* hbackend, const PI_BACKEND_RENDER_DESC* pdesc)
{
    /* Use OpenGL 3, minor version isn't included, should that be fine??? */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    hbackend->glcontext = SDL_GL_CreateContext(pdesc->sdl_window);
    if (!hbackend->glcontext) {
        return false;
    }

    if (!pi_gl_render_loadfuncs((PI_PFN_GLGETPROCADDRESS)SDL_GL_GetProcAddress, malloc, &hbackend->glfuncs)) {
        goto CLEANUPANDFAIL;
    }
    
    return true;
CLEANUPANDFAIL:
    free(hbackend->glfuncs);
    SDL_GL_DestroyContext(hbackend->glcontext);
    return false;
}

PI_EXTERN_C void fn_render_gl_close(pi_GLbackend* hbackend)
{
    free(hbackend->glfuncs);
    SDL_GL_DestroyContext(hbackend->glcontext);
}

bool fn_render_gl_configure(pi_GLbackend* hbackend, const PI_RENDER_CONFIGURE* info)
{
    gl.Viewport(0, 0, info->w, info->h);
    return true;
}
