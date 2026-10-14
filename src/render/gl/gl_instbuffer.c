#include "render_gl.h"

bool fn_render_gl_create_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer)
{
    hinstbuffer->vbomesh = 0;
    gl.GenVertexArrays(1, &hinstbuffer->vao);
    gl.GenBuffers(PI_ARRAYSIZE(hinstbuffer->vbos), hinstbuffer->vbos);

    gl.BindVertexArray(hinstbuffer->vao);

    gl.EnableVertexAttribArray(0);
    gl.EnableVertexAttribArray(1);
    
    gl.EnableVertexAttribArray(2);
    gl.EnableVertexAttribArray(3);
    gl.EnableVertexAttribArray(4);
    gl.EnableVertexAttribArray(5);

    gl.EnableVertexAttribArray(6);

    gl.EnableVertexAttribArray(7);
    gl.EnableVertexAttribArray(8);
    gl.EnableVertexAttribArray(9);


    gl.BindBuffer(GL_ARRAY_BUFFER, hinstbuffer->vbo);
    
    gl.VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.xx));
    gl.VertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.yx));
    gl.VertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.zx));
    gl.VertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, transform.wx));

    gl.VertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(PI_INSTANCE), (void*)offsetof(PI_INSTANCE, col));

    gl.BindBuffer(GL_ARRAY_BUFFER, hinstbuffer->normalvbo);

    gl.VertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(PI_MATRIX_3X3), (void*)offsetof(PI_MATRIX_3X3, xx));
    gl.VertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(PI_MATRIX_3X3), (void*)offsetof(PI_MATRIX_3X3, yx));
    gl.VertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(PI_MATRIX_3X3), (void*)offsetof(PI_MATRIX_3X3, zx));

    gl.VertexAttribDivisor(2, 1);
    gl.VertexAttribDivisor(3, 1);
    gl.VertexAttribDivisor(4, 1);
    gl.VertexAttribDivisor(5, 1);
    gl.VertexAttribDivisor(6, 1);
    gl.VertexAttribDivisor(7, 1);
    gl.VertexAttribDivisor(8, 1);
    gl.VertexAttribDivisor(9, 1);

    return true;
}
void fn_render_gl_destroy_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer)
{
    gl.DeleteBuffers(1, &hinstbuffer->vbo);
    gl.DeleteVertexArrays(1, &hinstbuffer->vao);
}
bool fn_render_gl_alloc_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned size, const PI_INSTANCE initial_data[], const PI_MATRIX_3X3* normal_transform_data
                                    )
{
    gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->vbo);
    gl.BufferData(GL_COPY_WRITE_BUFFER, size*sizeof(*initial_data), initial_data, GL_DYNAMIC_DRAW);
    if (normal_transform_data) {
        gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->normalvbo);
        gl.BufferData(GL_COPY_WRITE_BUFFER, size*sizeof(*normal_transform_data), normal_transform_data, GL_DYNAMIC_DRAW);
    }

    return true;
}
bool fn_render_gl_update_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned pos, unsigned count,
                                        const PI_INSTANCE data[], const PI_MATRIX_3X3* normal_transform_data
                                    )
{
    /* Just BufferSubData... For now??... */
    gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->vbo);
    gl.BufferSubData(GL_COPY_WRITE_BUFFER, pos*sizeof(*data), count*sizeof(*data), data);
    if (normal_transform_data) {
        gl.BindBuffer(GL_COPY_WRITE_BUFFER, hinstbuffer->normalvbo);
        gl.BufferSubData(GL_COPY_WRITE_BUFFER, pos*sizeof(*normal_transform_data), count*sizeof(*normal_transform_data), normal_transform_data);
    }

    return true;
}
