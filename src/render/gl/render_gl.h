#ifndef PI_RENDER_GL_H
#define PI_RENDER_GL_H

#define PI_INCLUDE_BACKEND_API 1
#include <piengine/pi_render.h>

/* 
 * <GL/glcorearb.h> -> "OpenGL core profile and ARB extension interfaces, as described in appendix G.2 of the OpenGL 4.3 Specification. Does not include interfaces found only in the compatibility profile."
 *
 *   This header is provided by the engine's include directories, although there's also a <glext.h> header (Post GL 1.2 & extensions header), we only need this one since we are ONLY going to use the Core profile. 
 * */

#include <GL/glcorearb.h>
#include <SDL3/SDL_video.h>

typedef struct PI_RENDERBACKEND {
    void* happy;
} pi_GLbackend;
typedef struct PI_RENDERBACKEND_WORLDENV {
    GLenum alotofenums[67];
} pi_GLworldenv;
typedef struct PI_RENDERBACKEND_SCENE {
    GLuint ubo;
} pi_GLscene;
typedef struct PI_RENDERBACKEND_MESH {
    GLuint vbo;
} pi_GLmesh;
typedef struct PI_RENDERBACKEND_INSTBUFFER {
    GLuint vbo;
    GLuint vao;
} pi_GLinstbuffer;

/**/

PI_EXTERN_C bool __pi_render_gl_initialize(pi_GLbackend* hbackend, const PI_BACKEND_RENDER_DESC* pdesc);

PI_EXTERN_C void __pi_render_gl_close(pi_GLbackend* hbackend);

EXTERN_C const PI_BACKEND_RENDER _PI_BACKEND_RENDER_GL;

#endif
