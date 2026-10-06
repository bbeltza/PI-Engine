#include "render_gl.h"

// Something is giving me GL_INVALID_ENUM...

bool fn_render_gl_create_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer)
{
    hinstbuffer->vbomesh = 0;
    gl.GenVertexArrays(1, &hinstbuffer->vao);
    gl.GenBuffers(1, &hinstbuffer->vbo);

    gl.BindVertexArray(hinstbuffer->vao);

    gl.EnableVertexAttribArray(0);
    gl.EnableVertexAttribArray(1);
    gl.EnableVertexAttribArray(2);
    gl.EnableVertexAttribArray(3);
    gl.EnableVertexAttribArray(4);
    gl.EnableVertexAttribArray(5);
    gl.EnableVertexAttribArray(6);

    gl.BindBuffer(GL_ARRAY_BUFFER, hinstbuffer->vbo);
    
    gl.VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.xx));
    gl.VertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.yx));
    gl.VertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.zx));
    gl.VertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.wx));

    gl.VertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, col));

    gl.VertexAttribDivisor(2, 1);
    gl.VertexAttribDivisor(3, 1);
    gl.VertexAttribDivisor(4, 1);
    gl.VertexAttribDivisor(5, 1);
    gl.VertexAttribDivisor(6, 1);

    return true;
}
void fn_render_gl_destroy_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer)
{
    gl.DeleteBuffers(1, &hinstbuffer->vbo);
    gl.DeleteVertexArrays(1, &hinstbuffer->vao);
}
bool fn_render_gl_alloc_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned size, const PI_INSTANCE initial_data[]
                                    )
{
    gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->vbo);
    gl.BufferData(GL_COPY_WRITE_BUFFER, size*sizeof(*initial_data), initial_data, GL_DYNAMIC_DRAW);

    return true;
}
bool fn_render_gl_update_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned pos, unsigned count,
                                        const PI_INSTANCE data[]
                                    )
{
    /* BufferSubData... For now??... */
    gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->vbo);
    gl.BufferSubData(GL_COPY_WRITE_BUFFER, pos*sizeof(*data), count*sizeof(*data), data);
    
    return true;
}
