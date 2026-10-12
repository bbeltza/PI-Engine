#include "render_gl.h"

#define PSENV_SIZE (sizeof(PI_LIGHT_PROPERTIES) + sizeof(unsigned)*4)

bool fn_render_gl_create_scene(pi_GLbackend* hbackend, pi_GLscene* hscene)
{

    gl.GenBuffers(1, &hscene->ubo);
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferData(GL_UNIFORM_BUFFER, hbackend->ubofsoffset + PSENV_SIZE, NULL, GL_DYNAMIC_DRAW);

    memset(hscene->lightubos, 0x00, sizeof(hscene->lightubos));
    hscene->last_dirlightcount = 0;
    hscene->last_ptlightcount = 0;
    return true;
}

void fn_render_gl_destroy_scene(pi_GLbackend* hbackend, pi_GLscene* hscene)
{
    gl.DeleteBuffers(1, &hscene->ubo);
    gl.DeleteBuffers(PI_ARRAYSIZE(hscene->lightubos), hscene->lightubos);
}

//

void fn_render_gl_update_scene_vp(pi_GLbackend* hbackend, pi_GLscene* hscene, const PI_MATRIX_4X4* vpmatrix)
{
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferSubData(GL_UNIFORM_BUFFER, offsetof(PI_SCENEENV, vpmatrix), sizeof(*vpmatrix), vpmatrix);
}

void fn_render_gl_update_scene_lightproperties(pi_GLbackend* hbackend, pi_GLscene* hscene, const PI_LIGHT_PROPERTIES* properties) {
    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
    gl.BufferSubData(GL_UNIFORM_BUFFER, hbackend->ubofsoffset, sizeof(*properties), properties);
}

//

static void bind_worldenv(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv);

void fn_render_gl_begin_scene(pi_GLbackend* hbackend, pi_GLscene* hscene, pi_GLworldenv* hworldenv, const PI_VECTOR4* clear_rgba)
{
    bind_worldenv(hbackend, hworldenv);

    gl.BindBufferRange(GL_UNIFORM_BUFFER, 0, hscene->ubo, 0, sizeof(PI_SCENEENV));
    if (hscene->dirlightubo) {
        PI_ASSERT(hscene->ptlightubo != 0);
        gl.BindBufferRange(GL_UNIFORM_BUFFER, 1, hscene->ubo, hbackend->ubofsoffset, PSENV_SIZE);
        if (hscene->last_dirlightcount)
            gl.BindBufferRange(GL_UNIFORM_BUFFER, 2, hscene->dirlightubo, 0, hscene->last_dirlightcount*sizeof(PI_DIRECTIONAL_LIGHT));
        if (hscene->last_ptlightcount)
            gl.BindBufferRange(GL_UNIFORM_BUFFER, 3, hscene->ptlightubo, 0, hscene->last_ptlightcount*sizeof(PI_POINT_LIGHT));
    }
    gl.ClearBufferfv(GL_COLOR, 0, clear_rgba->arr);
    if (hworldenv->depthmask) {
        GLfloat white = 1.0f;
        gl.ClearBufferfv(GL_DEPTH, 0, &white);
    }
}
void fn_render_gl_end_scene(pi_GLbackend* hbackend)
{
    if (hbackend->worldenv_state.samples) {
        gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        gl.BlitFramebuffer(
                0, 0, hbackend->wndwidth, hbackend->wndheight,
                0, 0, hbackend->wndwidth, hbackend->wndheight,
                GL_COLOR_BUFFER_BIT, GL_NEAREST
            );
    }

    SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

void bind_worldenv(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv)
{
    if (hworldenv->samples) {
        GLmsaabuffer* msaabuffer = &hbackend->msaabuffers[hworldenv->samples - 1];
        gl.BindFramebuffer(GL_FRAMEBUFFER, msaabuffer->fbo);
        if (!msaabuffer->rbocolor) {
            gl.GenRenderbuffers(1, &msaabuffer->rbocolor);
            gl.BindRenderbuffer(GL_RENDERBUFFER, msaabuffer->rbocolor);
            gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, hworldenv->samples, GL_SRGB8, hbackend->wndwidth, hbackend->wndheight);
            gl.FramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaabuffer->rbocolor);

        }

        if (hworldenv->depthenable && !msaabuffer->rbodepth) {
            gl.GenRenderbuffers(1, &msaabuffer->rbodepth);
            gl.BindRenderbuffer(GL_RENDERBUFFER, msaabuffer->rbodepth);
            gl.RenderbufferStorageMultisample(GL_RENDERBUFFER, hworldenv->samples, GL_DEPTH24_STENCIL8, hbackend->wndwidth, hbackend->wndheight);
            gl.FramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaabuffer->rbodepth);
        }
    }

    if (hbackend->currworldenv == hworldenv) {
        return;
    }

    if (hbackend->worldenv_state.shader != hworldenv->shader) {
        gl.UseProgram(hworldenv->shader);
        hbackend->worldenv_state.shader = hworldenv->shader;
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
    
    if (!hworldenv->samples) {
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    hbackend->worldenv_state = *hworldenv;
}

void fn_render_gl_update_lights(pi_GLbackend* hbackend, pi_GLscene* hscene, PI_LIGHTTYPE type, unsigned update_start, unsigned update_count, const PI_LIGHT lights[], unsigned full_light_count)
{
    if (!hscene->dirlightubo) {
        PI_ASSERT(!hscene->ptlightubo);
        PI_ASSERT(!hscene->dirlightstagingbuf);
        PI_ASSERT(!hscene->ptlightstagingbuf);

        GLsizeiptr ubosize = hbackend->maxubodirlights * sizeof(PI_DIRECTIONAL_LIGHT);
        gl.GenBuffers(PI_ARRAYSIZE(hscene->lightubos), hscene->lightubos);
        gl.BindBuffer(GL_COPY_READ_BUFFER, hscene->dirlightubo);
        gl.BindBuffer(GL_COPY_WRITE_BUFFER, hscene->dirlightstagingbuf);
        gl.BufferData(GL_COPY_READ_BUFFER, ubosize, NULL, GL_DYNAMIC_COPY);
        gl.BufferData(GL_COPY_WRITE_BUFFER, ubosize, NULL, GL_STREAM_DRAW);

        ubosize = hbackend->maxuboptlights * sizeof(PI_POINT_LIGHT);
        gl.BindBuffer(GL_COPY_READ_BUFFER, hscene->ptlightubo);
        gl.BindBuffer(GL_COPY_WRITE_BUFFER, hscene->ptlightstagingbuf);
        gl.BufferData(GL_COPY_READ_BUFFER, ubosize, NULL, GL_DYNAMIC_COPY);
        gl.BufferData(GL_COPY_WRITE_BUFFER, ubosize, NULL, GL_STREAM_DRAW);
    }


    GLintptr offset = update_start;
    GLsizeiptr size = update_count;
    switch (type)
    {
        case PI_LIGHT_DIRECTIONAL: {
                if (full_light_count != hscene->last_dirlightcount) {
                    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
                    gl.BufferSubData(GL_UNIFORM_BUFFER,
                                    hbackend->ubofsoffset + sizeof(PI_LIGHT_PROPERTIES) + 0*sizeof(unsigned),
                                    sizeof(unsigned), &full_light_count
                                );
                    hscene->last_dirlightcount = full_light_count;
                    if (update_start == -1) {
                        return;
                    }
                }

                offset *= sizeof(PI_DIRECTIONAL_LIGHT);
                size *= sizeof(PI_DIRECTIONAL_LIGHT);
                gl.BindBuffer(GL_COPY_READ_BUFFER, hscene->dirlightubo);
                gl.BindBuffer(GL_COPY_WRITE_BUFFER, hscene->dirlightstagingbuf);
                PI_DIRECTIONAL_LIGHT* mapped = gl.MapBufferRange(
                        GL_COPY_READ_BUFFER,
                        offset,
                        size,
                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
                    );
                for (unsigned i = 0; i < update_count; i++) {
                    mapped[i] = lights[i].directional;
                }
            } break;
        case PI_LIGHT_POINT:
                if (full_light_count != hscene->last_ptlightcount) {
                    gl.BindBuffer(GL_UNIFORM_BUFFER, hscene->ubo);
                    gl.BufferSubData(GL_UNIFORM_BUFFER,
                                    hbackend->ubofsoffset + sizeof(PI_LIGHT_PROPERTIES) + 1*sizeof(unsigned),
                                    sizeof(unsigned), &full_light_count
                                );
                    hscene->last_ptlightcount = full_light_count;
                    if (update_start == -1) {
                        return;
                    }
                }
                offset *= sizeof(PI_POINT_LIGHT);
                size *= sizeof(PI_POINT_LIGHT);
                gl.BindBuffer(GL_COPY_READ_BUFFER, hscene->ptlightubo);
                gl.BindBuffer(GL_COPY_WRITE_BUFFER, hscene->ptlightstagingbuf);
                PI_POINT_LIGHT* mapped = gl.MapBufferRange(
                        GL_COPY_READ_BUFFER,
                        offset,
                        size,
                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
                    );
                for (unsigned i = 0; i < update_count; i++) {
                    mapped[i] = lights[i].point;
                }
            break;
        default: PI_ASSUME(0);
    }
    gl.UnmapBuffer(GL_COPY_READ_BUFFER);
    gl.CopyBufferSubData(
            GL_COPY_READ_BUFFER,
            GL_COPY_WRITE_BUFFER,
            offset, offset, size
        );

}

