#include "render_gl.h"


bool fn_render_gl_create_scene(pi_GLbackend* hbackend, pi_GLscene* hscene)
{
    gl.GenBuffers(1, &hscene->ubo);
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferData(GL_UNIFORM_BUFFER_BINDING, sizeof(PI_SCENEENV), NULL, GL_DYNAMIC_DRAW);
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

void fn_render_gl_begin_scene(pi_GLbackend* hbackend, pi_GLscene* hscene, pi_GLworldenv* hworldenv, const PI_VECTOR4* clear_rgba)
{
    gl.BindBufferBase(GL_UNIFORM_BUFFER, 0, hscene->ubo);
    gl.ClearBufferfv(GL_COLOR, 0, clear_rgba->arr);
    /* Do more stuff... */
}
void fn_render_gl_end_scene(pi_GLbackend* hbackend)
{
    SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}
