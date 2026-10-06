#include "render_gl.h"


bool fn_render_gl_create_scene(pi_GLbackend* hbackend, pi_GLscene* hscene)
{
    gl.GenBuffers(1, &hscene->ubo);
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferData(GL_UNIFORM_BUFFER, sizeof(PI_SCENEENV), NULL, GL_DYNAMIC_DRAW);
    return true;
}

void fn_render_gl_destroy_scene(pi_GLbackend* hbackend, pi_GLscene* hscene)
{
    gl.DeleteBuffers(1, &hscene->ubo);
}

//

void fn_render_gl_update_scene_vp(pi_GLbackend* hbackend, pi_GLscene* hscene, const PI_MATRIX_4X4* vpmatrix)
{
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferSubData(GL_UNIFORM_BUFFER, offsetof(PI_SCENEENV, vpmatrix), sizeof(*vpmatrix), vpmatrix);
}

//

static void bind_worldenv(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv);

void fn_render_gl_begin_scene(pi_GLbackend* hbackend, pi_GLscene* hscene, pi_GLworldenv* hworldenv, const PI_VECTOR4* clear_rgba)
{
    gl.BindBufferBase(GL_UNIFORM_BUFFER, 0, hscene->ubo);
    gl.ClearBufferfv(GL_COLOR, 0, clear_rgba->arr);
    if (hworldenv->depthmask) {
        GLfloat white = 1.0f;
        gl.ClearBufferfv(GL_DEPTH, 0, &white);
    }

    bind_worldenv(hbackend, hworldenv);
}
void fn_render_gl_end_scene(pi_GLbackend* hbackend)
{
    gl.BindFramebuffer(GL_READ_FRAMEBUFFER, hbackend->fbotest);
    gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    gl.BlitFramebuffer(
            0, 0, 800, 600,
            0, 0, hbackend->wndwidth, hbackend->wndheight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
            );

    SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());

    gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, hbackend->fbotest);
}

void bind_worldenv(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv)
{
    if (hbackend->currworldenv == hworldenv) {
        return;
    }

    hbackend->currworldenv = hworldenv;

    gl.PolygonMode(GL_FRONT_AND_BACK, hworldenv->polygonmode);
    gl.FrontFace(hworldenv->frontface);
    if (hworldenv->cullface) {
        gl.Enable(GL_CULL_FACE);
        gl.CullFace(hworldenv->cullface);
    }
    else {
        gl.Disable(GL_CULL_FACE);
    }

    if (hworldenv->depthenable) {
        gl.Enable(GL_DEPTH_TEST);
        gl.DepthFunc(hworldenv->depthfunc); 
        gl.DepthMask(hworldenv->depthmask);
    }
    else {
        gl.Disable(GL_DEPTH_TEST);
    }
}
