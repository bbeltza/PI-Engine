#include "render_gl.h"


bool fn_render_gl_create_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh,
                                        const PI_VERTEX* vertices, unsigned num_vertices,
                                        const void* indices, unsigned num_indices,
                                        PI_INDEX index_type
                                    )
{
    gl.GenBuffers(1, &hmesh->vboandibo);
    gl.BindBuffer(GL_COPY_WRITE_BUFFER, hmesh->vboandibo);
    
    GLsizei ibsize = 0;
    switch (index_type)
    {
        case PI_INDEX_U16:
            hmesh->ibotype = GL_UNSIGNED_SHORT;
            ibsize = num_indices*sizeof(short);
            break;
        case PI_INDEX_U32:
            hmesh->ibotype = GL_UNSIGNED_INT;
            ibsize = num_indices*sizeof(int);
            break;
        case PI_INDEX_UNKNOWN:
            hmesh->ibotype = 0;
            break;
    }

    if (ibsize) {
        hmesh->vertcount = num_indices;
        hmesh->indexoffset = sizeof(PI_VERTEX) * num_vertices;
        gl.BufferData(GL_COPY_WRITE_BUFFER, hmesh->indexoffset + ibsize, NULL, GL_STATIC_DRAW);
        PI_VERTEX* mappedvertices = gl.MapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);        
        PI_ASSERT(mappedvertices != NULL); // Doing a simple assertion for now..
        for (unsigned i = 0; i < num_vertices; i++)
            mappedvertices[i] = vertices[i];
        memcpy(mappedvertices + num_vertices, indices, ibsize);

        gl.UnmapBuffer(GL_COPY_WRITE_BUFFER);
    }
    else {
        hmesh->vertcount = num_vertices;
        gl.BufferData(GL_COPY_WRITE_BUFFER, num_vertices*sizeof(*vertices), vertices, GL_STATIC_DRAW);
    }

    return true;
}
void fn_render_gl_destroy_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh) {
    gl.DeleteBuffers(1, &hmesh->vboandibo);
}

PI_EXTERN_C void fn_render_gl_submit_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh, pi_GLinstbuffer* hinstbuffer, unsigned instance_count)
{
    gl.BindVertexArray(hinstbuffer->vao);
    if (hinstbuffer->vbomesh != hmesh->vboandibo) {
        hinstbuffer->vbomesh = hmesh->vboandibo;
        gl.BindBuffer(GL_ARRAY_BUFFER, hmesh->vboandibo);

        /* Setup per-vertex attributes */
            gl.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PI_VERTEX), (void*)offsetof(PI_VERTEX, pos));
            gl.VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PI_VERTEX), (void*)offsetof(PI_VERTEX, col));

        gl.BindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (hmesh->ibotype) {
        gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, hmesh->vboandibo);
        gl.DrawElementsInstanced(GL_TRIANGLES,
                hmesh->vertcount, hmesh->ibotype,
                (const void*)(size_t)hmesh->indexoffset,
                instance_count
            );
    }
    else {
        gl.DrawArraysInstanced(GL_TRIANGLES,
                0, hmesh->vertcount,
                instance_count
            );
    }
}
