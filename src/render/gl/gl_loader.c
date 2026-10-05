#include "render_gl.h"

bool pi_gl_render_loadfuncs(PI_PFN_GLGETPROCADDRESS pfn_GetProcAddress, PI_PFN_ALLOCATOR alloc, pi_GLfuncs** pp_glfuncs)
{
    if (!pfn_GetProcAddress) {
        return false;
    }
    if (!alloc) {
        return false;
    }

    pi_GLfuncs* glfuncs = alloc(sizeof(pi_GLfuncs));
    *pp_glfuncs = glfuncs;
    if (!glfuncs) {
        return false;
    }

    PI_GLGETPROC(glfuncs, Viewport);

    PI_GLGETPROC(glfuncs, GenBuffers);
    PI_GLGETPROC(glfuncs, DeleteBuffers);
    PI_GLGETPROC(glfuncs, BindBuffer);
    PI_GLGETPROC(glfuncs, BufferData);
    PI_GLGETPROC(glfuncs, BufferSubData);
    PI_GLGETPROC(glfuncs, MapBuffer);
    PI_GLGETPROC(glfuncs, UnmapBuffer);

    PI_GLGETPROC(glfuncs, GenVertexArrays);
    PI_GLGETPROC(glfuncs, DeleteVertexArrays);
    PI_GLGETPROC(glfuncs, BindVertexArray);
    PI_GLGETPROC(glfuncs, EnableVertexAttribArray);
    PI_GLGETPROC(glfuncs, VertexAttribPointer);

    PI_GLGETPROC(glfuncs, BindBufferBase);

    PI_GLGETPROC(glfuncs, ClearBufferfv);

    PI_GLGETPROC(glfuncs, DrawArraysInstanced);
    PI_GLGETPROC(glfuncs, DrawElementsInstanced);

    PI_GLGETPROC(glfuncs, GetError);
    return true;
}
