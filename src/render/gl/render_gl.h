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

#include <stddef.h>
#include <stdio.h>

#ifndef __has_include
    #define __has_include(n) 0
#endif

#if !defined(NDEBUG)
    #define WANT_GL_DBGTRAMPOLINE 1
#else
    //#define WANT_GL_DBGTRAMPOLINE 0
#endif

#if (_WIN32 || (defined(__unix__) && __has_include(<sys/mman.h>))) && \
    (defined(__x86_64__) || defined(_M_X64))
    #define USE_GL_DBGTRAMPOLINE 1
#endif

#define PI_GLGETPROCADDRESS pfn_GetProcAddress /* Used by `pi_gl_render_loadfuncs`, can be overridden with something else */
#define PI_GLGETPROC(glfuncs, n) (glfuncs)->n = PI_GLGETPROCADDRESS("gl" #n)

typedef struct pi_GLfuncs {
    PFNGLGETERRORPROC GetError;
    PFNGLVIEWPORTPROC Viewport;

    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLBUFFERSUBDATAPROC BufferSubData;
    PFNGLMAPBUFFERPROC MapBuffer;
    PFNGLUNMAPBUFFERPROC UnmapBuffer;

    PFNGLGENVERTEXARRAYSPROC GenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC BindVertexArray;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    PFNGLVERTEXATTRIBDIVISORPROC VertexAttribDivisor;

    PFNGLCREATESHADERPROC CreateShader;
    PFNGLCREATEPROGRAMPROC CreateProgram;
    PFNGLDELETESHADERPROC DeleteShader;
    PFNGLDELETEPROGRAMPROC DeleteProgram;
    PFNGLSHADERSOURCEPROC ShaderSource;
    PFNGLCOMPILESHADERPROC CompileShader;
    PFNGLLINKPROGRAMPROC LinkProgram;
    PFNGLUSEPROGRAMPROC UseProgram;
    PFNGLATTACHSHADERPROC AttachShader;
    PFNGLDETACHSHADERPROC DetachShader;
    
    PFNGLGETUNIFORMBLOCKINDEXPROC GetUniformBlockIndex;
    PFNGLUNIFORMBLOCKBINDINGPROC UniformBlockBinding;
    PFNGLBINDBUFFERBASEPROC BindBufferBase;

    PFNGLGETSHADERIVPROC GetShaderiv;
    PFNGLGETPROGRAMIVPROC GetProgramiv;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;

    PFNGLGENFRAMEBUFFERSPROC GenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
    PFNGLGENRENDERBUFFERSPROC GenRenderbuffers;
    PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;

    PFNGLBINDFRAMEBUFFERPROC BindFramebuffer;
    PFNGLBINDRENDERBUFFERPROC BindRenderbuffer;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC FramebufferRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC RenderbufferStorageMultisample;
    PFNGLCLEARBUFFERFVPROC ClearBufferfv;
    PFNGLBLITFRAMEBUFFERPROC BlitFramebuffer;

    PFNGLENABLEPROC Enable;
    PFNGLDISABLEPROC Disable;

    PFNGLPOLYGONMODEPROC PolygonMode;
    PFNGLCULLFACEPROC CullFace;
    PFNGLFRONTFACEPROC FrontFace;
    PFNGLDEPTHMASKPROC DepthMask;
    PFNGLDEPTHFUNCPROC DepthFunc;

    PFNGLDRAWARRAYSINSTANCEDPROC DrawArraysInstanced;
    PFNGLDRAWELEMENTSINSTANCEDPROC DrawElementsInstanced;

    PFNGLGETINTEGERVPROC GetIntegerv;
} pi_GLfuncs;

typedef void* (*PI_PFN_GLGETPROCADDRESS)(const char* procname);
typedef void* (*PI_PFN_ALLOCATOR)(size_t n);

PI_EXTERN_C bool pi_gl_render_loadfuncs(PI_PFN_GLGETPROCADDRESS pfn_GetProcAddress, PI_PFN_ALLOCATOR alloc, pi_GLfuncs** pp_glfuncs);

typedef struct GLmsaabuffer {
    GLuint fbo;

    union {
        struct {
            GLuint rbocolor;
            GLuint rbodepth;
        };
        GLuint rbos[2];       
    };
} GLmsaabuffer;

/**/

typedef struct PI_RENDERBACKEND_WORLDENV {
    GLenum polygonmode;
    GLenum cullface;
    GLenum frontface;

    GLenum depthfunc;
    GLboolean depthmask;
    GLboolean depthenable;

    GLsizei samples;
} pi_GLworldenv;

typedef struct PI_RENDERBACKEND {
    pi_GLfuncs* glfuncs;
    SDL_GLContext glcontext;
    #if USE_GL_DBGTRAMPOLINE
        GLenum gllasterror;
        void* trampolineretaddress;
        GLubyte* gldebugtrampolinecode;
    #endif

    pi_GLworldenv worldenv_state;
    pi_GLworldenv* currworldenv;

    GLuint wndwidth, wndheight;
    GLmsaabuffer* msaabuffers;
    GLint msaasamples;
} pi_GLbackend;

typedef struct PI_RENDERBACKEND_SCENE {
    GLuint ubo;
} pi_GLscene;
typedef struct PI_RENDERBACKEND_MESH {
    GLuint vboandibo;
    GLenum ibotype;
    GLsizei vertcount;
    GLuint indexoffset;
} pi_GLmesh;
typedef struct PI_RENDERBACKEND_INSTBUFFER {
    GLuint vao;
    GLuint vbo;

    GLuint vbomesh;
} pi_GLinstbuffer;

#if USE_GL_DBGTRAMPOLINE
    PI_EXTERN_C bool pi_gl_init_trampoline(pi_GLbackend* hbackend);
    PI_EXTERN_C void pi_gl_close_trampoline(pi_GLbackend* hbackend);
#endif


/* Compile shader corresponding to the word environment effects passed in `effects`. And return a name given by `glCreateProgram`. */
PI_EXTERN_C GLuint pi_gl_compile_shader(pi_GLbackend* hbackend, PI_WORLDENV_EFFECTS effects);
/* `pi_gl_compile_shader` CAN store cached shaders (Returned by `glCreateShader`) inside the `hbackend`'s internal storage. This function will cleanup any shader that has been cached. This might slow down compilation times if `pi_gl_compile_shader` is called once again. */
PI_EXTERN_C void pi_gl_clean_cached_shaders(pi_GLbackend* hbackend);

/**/

#define gl (*hbackend->glfuncs)

PI_EXTERN_C bool fn_render_gl_initialize(pi_GLbackend* hbackend, const PI_BACKEND_RENDER_DESC* pdesc);

PI_EXTERN_C void fn_render_gl_close(pi_GLbackend* hbackend);
PI_EXTERN_C bool fn_render_gl_configure(pi_GLbackend* hbackend, const PI_RENDER_CONFIGURE* info);
PI_EXTERN_C bool fn_render_gl_create_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv, const PI_WORLDENV_DESC* pdesc);
PI_EXTERN_C void fn_render_gl_destroy_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv);
PI_EXTERN_C bool fn_render_gl_create_scene(pi_GLbackend* hbackend, pi_GLscene* hscene);
PI_EXTERN_C void fn_render_gl_destroy_scene(pi_GLbackend* hbackend, pi_GLscene* hscene);
PI_EXTERN_C bool fn_render_gl_create_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh,
                                        const PI_VERTEX* vertices, unsigned num_vertices,
                                        const void* indices, unsigned num_indices,
                                        PI_INDEX index_type
                                    );
PI_EXTERN_C void fn_render_gl_destroy_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh);
PI_EXTERN_C bool fn_render_gl_create_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer);
PI_EXTERN_C void fn_render_gl_destroy_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer);
PI_EXTERN_C bool fn_render_gl_alloc_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned size, const PI_INSTANCE initial_data[]
                                    );
PI_EXTERN_C bool fn_render_gl_update_instbuffer(pi_GLbackend* hbackend, pi_GLinstbuffer* hinstbuffer,
                                        unsigned pos, unsigned count,
                                        const PI_INSTANCE data[]
                                    );
PI_EXTERN_C void fn_render_gl_update_scene_vp(pi_GLbackend* hbackend, pi_GLscene* hscene, const PI_MATRIX_4X4* vpmatrix);
PI_EXTERN_C void fn_render_gl_begin_scene(pi_GLbackend* hbackend, pi_GLscene* hscene, pi_GLworldenv* hworldenv, const PI_VECTOR4* clear_rgba);
PI_EXTERN_C void fn_render_gl_end_scene(pi_GLbackend* hbackend);
PI_EXTERN_C void fn_render_gl_submit_mesh(pi_GLbackend* hbackend, pi_GLmesh* hmesh, pi_GLinstbuffer* hinstbuffer, unsigned instance_count);

PI_EXTERN_C const PI_BACKEND_RENDER _PI_BACKEND_RENDER_GL;

#endif
