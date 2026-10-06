#include "render_gl.h"


PI_EXTERN_C bool fn_render_gl_initialize(pi_GLbackend* hbackend, const PI_BACKEND_RENDER_DESC* pdesc)
{
    /* Use OpenGL 3, minor version isn't included, should that be fine??? */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    hbackend->glcontext = SDL_GL_CreateContext(pdesc->sdl_window);
    if (!hbackend->glcontext) {
        return false;
    }

    if (!pi_gl_render_loadfuncs((PI_PFN_GLGETPROCADDRESS)SDL_GL_GetProcAddress, malloc, &hbackend->glfuncs)) {
        goto CLEANUPANDFAIL;
    }
    
#if PI_GL_WIN32DEBUGTRAMPOLINE
    if (!pi_gl_init_trampoline(hbackend)) {
        goto CLEANUPANDFAIL;
    }
#endif

    GLuint shader = pi_gl_compile_shader(hbackend, PI_WORLDEFFECT_NONE);
    if (shader == 0) {
        goto CLEANUPANDFAIL;
    }

    gl.UseProgram(shader);
    gl.Enable(GL_FRAMEBUFFER_SRGB);

    { // For testing only (multisample and framebuffer rendering), it is not ready to be used!!!
        GLuint fbo;
        gl.GenFramebuffers(1, &fbo);
        gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        GLuint rbos[2];
        gl.GenRenderbuffers(PI_ARRAYSIZE(rbos), rbos);
        gl.BindRenderbuffer(GL_RENDERBUFFER, rbos[0]);
        gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA, 800, 600);
        gl.BindRenderbuffer(GL_RENDERBUFFER, rbos[1]);
        gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, 800, 600);

        gl.FramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbos[0]);
        gl.FramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbos[1]);

        hbackend->fbotest = fbo;
    }

    SDL_GL_SetSwapInterval(1);
    return true;
CLEANUPANDFAIL:
    fn_render_gl_close(hbackend);
    return false;
}

PI_EXTERN_C void fn_render_gl_close(pi_GLbackend* hbackend)
{
#ifdef PI_GL_WIN32DEBUGTRAMPOLINE
    pi_gl_close_trampoline(hbackend);
#endif

    free(hbackend->glfuncs);
    SDL_GL_DestroyContext(hbackend->glcontext);
}

bool fn_render_gl_configure(pi_GLbackend* hbackend, const PI_RENDER_CONFIGURE* info)
{
    gl.Viewport(0, 0, 800, 600);
    hbackend->wndwidth = info->w;
    hbackend->wndheight = info->h;
    return true;
}
