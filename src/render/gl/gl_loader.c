#include "render_gl.h"

#if __has_include(<sys/mman.h>)
    #include <sys/mman.h>
#endif
#if __has_include(<sys/cachectl.h>)
    #include <sys/cachectl.h>
    #define HAS_CACHECTL_H 1
#endif

#define CASESTR(x) case x: return #x

static const char* glerrortostr(GLenum err)
{
    switch (err)
    {
        CASESTR(GL_NO_ERROR);
        CASESTR(GL_INVALID_ENUM);
        CASESTR(GL_INVALID_VALUE);
        CASESTR(GL_INVALID_OPERATION);
        CASESTR(GL_INVALID_FRAMEBUFFER_OPERATION);
        CASESTR(GL_OUT_OF_MEMORY);
        CASESTR(GL_STACK_UNDERFLOW);
        CASESTR(GL_STACK_OVERFLOW);
        default: return NULL;
    }
}

static void glerrorcheck(PFNGLGETERRORPROC pfn_glGetError, GLenum* lasterror, const char* name) {
    GLenum err = pfn_glGetError();
    if (err == GL_NO_ERROR) {
        *lasterror = err;
        return;
    }
    while (err != GL_NO_ERROR) {
        fprintf(stderr, "GL ERROR at %s: \"%s\" (%X)\n", name, glerrortostr(err), err);
        *lasterror = err;
        err = pfn_glGetError();
    }
}

#if USE_GL_DBGTRAMPOLINE
// This excludes the glGetError at the start of the structure
#define GLFUNCS_SIZE (sizeof(pi_GLfuncs) - sizeof(void*))
#define GLFUNCS_COUNT (GLFUNCS_SIZE / sizeof(void*))
#define GLFUNCS_BEGIN(glfuncs) ((void**)(glfuncs) + 1)

#if _WIN32

#define EMITOFFSET_RETADDRESS1 4
#define EMITOFFSET_RETADDRESS2 29

#define EMITOFFSET_GLFUNC 17

#define EMITOFFSET_NAME 50
#define EMITOFFSET_LASTERROR 60
#define EMITOFFSET_GETERROR 70
#define EMITOFFSET_ERRORCHECK 80

const GLubyte EMIT_CODE_TEMPLATE[] = {
    /* pop r10 */
    0x41, 0x5A,
    /* mov rax, 0xXXXXXXXXXXXXXXXX (return address pointer) */
    0x48, 0xB8, 
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* mov [rax], r10 */
    0x4C, 0x89, 0x10,
    /* mov rax, 0xXXXXXXXXXXXXXXXX (GL function pointer) */
    0x48, 0xB8,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,

    /* call rax (Jump to the real function pointer) */
    0xFF, 0xD0,

    /* mov rcx, 0xXXXXXXXXXXXXXXXX (return address pointer again) */
    0x48, 0xB9,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* push [rcx] */
    0xFF, 0x31,

    /* sub rsp, 0x28 */
    0x48, 0x83, 0xEC, 0x28, 
    /* mov [rsp+0x20], rax */
    0x48, 0x89,
    0x44, 0x24, 0x20,

    /* mov r8, 0xXXXXXXXXXXXXXXXX (`name` parameter) */
    0x4F, 0xB8,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* mov rdx, 0xXXXXXXXXXXXXXXXX (Pointer to the last error variable) */
    0x48, 0xBA,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* mov rcx, 0xXXXXXXXXXXXXXXXX (pfn_glGetError) */
    0x48, 0xB9,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* mov rax, 0xXXXXXXXXXXXXXXXX (Pointer to `glerrorcheck`) */
    0x48, 0xB8,
    0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
    /* call rax (Jump to `glerrorcheck`) */
    0xFF, 0xD0,

    /* mov rax, [rsp+0x20] */
    0x48, 0x8B,
    0x44, 0x24, 0x20,
    /* add rsp, 0x28 */
    0x48, 0x83, 0xC4, 0x28, 
    /* ret */
    0xC3
};

#elif defined(__unix__)
    #define EMITOFFSET_RETADDRESS1 4
    #define EMITOFFSET_RETADDRESS2 29

    #define EMITOFFSET_GLFUNC 17

    #define EMITOFFSET_NAME 42
    #define EMITOFFSET_LASTERROR 52
    #define EMITOFFSET_GETERROR 62
    #define EMITOFFSET_ERRORCHECK 72

    const GLubyte EMIT_CODE_TEMPLATE[] = {
        /* pop r11 */
        0x41, 0x5B,
        /* mov rax, 0xXXXXXXXXXXXXXXXX (return address pointer) */
        0x48, 0xB8, 
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* mov [rax], r11 */
        0x4C, 0x89, 0x18,
        /* mov rax, 0xXXXXXXXXXXXXXXXX (GL function pointer) */
        0x48, 0xB8,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,

        /* call rax (Jump to the real function pointer) */
        0xFF, 0xD0,

        /* mov rcx, 0xXXXXXXXXXXXXXXXX (return address pointer again) */
        0x48, 0xB9,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* push [rcx] */
        0xFF, 0x31,
        /* push rax */
        0x50,

        /* mov rdx, 0xXXXXXXXXXXXXXXXX (`name` parameter) */
        0x48, 0xBA,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* mov rsi, 0xXXXXXXXXXXXXXXXX (Pointer to the last error variable) */
        0x48, 0xBE,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* mov rdi, 0xXXXXXXXXXXXXXXXX (pfn_glGetError) */
        0x48, 0xBF,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* mov rax, 0xXXXXXXXXXXXXXXXX (Pointer to `glerrorcheck`) */
        0x48, 0xB8,
        0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD,
        /* call rax (Jump to `glerrorcheck`) */
        0xFF, 0xD0,

        /* pop rax */
        0x58,
        /* ret */
        0xC3
    };
#else
    #error "Wrong architecture???"
#endif

const unsigned NUMFUNCS = GLFUNCS_COUNT;
const unsigned EMITTED_CODE_SIZE = GLFUNCS_COUNT * sizeof(EMIT_CODE_TEMPLATE);

const char* FUNCTION_NAMES[GLFUNCS_COUNT];

static void emit_gl_proc_code(pi_GLbackend* hbackend, unsigned funcindex)
{
    PI_ASSERT(funcindex < GLFUNCS_COUNT);
    PI_ASSERT(hbackend->gldebugtrampolinecode != NULL);

    void** pfunc = &GLFUNCS_BEGIN(hbackend->glfuncs)[funcindex];
    GLubyte* data = hbackend->gldebugtrampolinecode + sizeof(EMIT_CODE_TEMPLATE)*funcindex;
    memcpy(data, EMIT_CODE_TEMPLATE, sizeof(EMIT_CODE_TEMPLATE));

    *(void**)(data + EMITOFFSET_RETADDRESS1) = &hbackend->trampolineretaddress;
    *(void**)(data + EMITOFFSET_RETADDRESS2) = &hbackend->trampolineretaddress;
    *(void**)(data + EMITOFFSET_GLFUNC) = *pfunc;
    *(void**)(data + EMITOFFSET_NAME) = (void*)FUNCTION_NAMES[funcindex];
    *(void**)(data + EMITOFFSET_LASTERROR) = &hbackend->gllasterror;
    *(void**)(data + EMITOFFSET_GETERROR) = hbackend->glfuncs->GetError;
    *(void**)(data + EMITOFFSET_ERRORCHECK) = glerrorcheck;

    *pfunc = data;
}

bool pi_gl_init_trampoline(pi_GLbackend* hbackend)
{
    PI_ASSERT(hbackend->glfuncs != NULL);

#if _WIN32
    hbackend->gldebugtrampolinecode = VirtualAlloc(NULL, EMITTED_CODE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!hbackend->gldebugtrampolinecode) {
        return false;
    }
#else
    hbackend->gldebugtrampolinecode = mmap(NULL, EMITTED_CODE_SIZE, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (hbackend->gldebugtrampolinecode == (void*)-1) {
        perror("mmap failed!");
        return false;
    }
#endif

    for (unsigned i = 0; i < NUMFUNCS; i++) {
        emit_gl_proc_code(hbackend, i);
    }

#if _WIN32
    DWORD oldprotect;
    VirtualProtect(hbackend->gldebugtrampolinecode, EMITTED_CODE_SIZE, PAGE_EXECUTE, &oldprotect);
    FlushInstructionCache(GetCurrentProcess(), hbackend->gldebugtrampolinecode, EMITTED_CODE_SIZE);
#else
    mprotect(hbackend->gldebugtrampolinecode, EMITTED_CODE_SIZE, PROT_EXEC);
    #if HAS_CACHECTL_H
        cacheflush(hbackend->gldebugtrampolinecode, EMITTED_CODE_SIZE, ICACHE);
    #endif
#endif
    return true;
}

void pi_gl_close_trampoline(pi_GLbackend* hbackend)
{
#if _WIN32
    VirtualFree(hbackend->gldebugtrampolinecode, 0, MEM_RELEASE);
#else
    munmap(hbackend->gldebugtrampolinecode, EMITTED_CODE_SIZE);
#endif
}

#endif

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

#if USE_GL_DBGTRAMPOLINE
    #define GETPROC(glfuncs, n) PI_GLGETPROC(glfuncs, n); FUNCTION_NAMES[funidx++] = "gl" #n
    unsigned funidx = 0;
#else
    #define GETPROC PI_GLGETPROC
#endif


    PI_GLGETPROC(glfuncs, GetError);
    GETPROC(glfuncs, Viewport);

    GETPROC(glfuncs, GenBuffers);
    GETPROC(glfuncs, DeleteBuffers);
    GETPROC(glfuncs, BindBuffer);
    GETPROC(glfuncs, BufferData);
    GETPROC(glfuncs, BufferSubData);
    GETPROC(glfuncs, MapBuffer);
    GETPROC(glfuncs, MapBufferRange);
    GETPROC(glfuncs, UnmapBuffer);
    GETPROC(glfuncs, CopyBufferSubData);

    GETPROC(glfuncs, GenVertexArrays);
    GETPROC(glfuncs, DeleteVertexArrays);
    GETPROC(glfuncs, BindVertexArray);
    GETPROC(glfuncs, EnableVertexAttribArray);
    GETPROC(glfuncs, VertexAttribPointer);
    GETPROC(glfuncs, VertexAttribDivisor);

    GETPROC(glfuncs, CreateShader);
    GETPROC(glfuncs, CreateProgram);
    GETPROC(glfuncs, DeleteShader);
    GETPROC(glfuncs, DeleteProgram);
    GETPROC(glfuncs, ShaderSource);
    GETPROC(glfuncs, CompileShader);
    GETPROC(glfuncs, LinkProgram);
    GETPROC(glfuncs, UseProgram);
    GETPROC(glfuncs, AttachShader);
    GETPROC(glfuncs, DetachShader);

    GETPROC(glfuncs, GetUniformBlockIndex);
    GETPROC(glfuncs, UniformBlockBinding);
    GETPROC(glfuncs, BindBufferRange);
    
    GETPROC(glfuncs, GetShaderiv);
    GETPROC(glfuncs, GetProgramiv);
    GETPROC(glfuncs, GetShaderInfoLog);
    GETPROC(glfuncs, GetProgramInfoLog);

    GETPROC(glfuncs, GenFramebuffers);
    GETPROC(glfuncs, DeleteFramebuffers);
    GETPROC(glfuncs, GenRenderbuffers);
    GETPROC(glfuncs, DeleteRenderbuffers);

    GETPROC(glfuncs, BindFramebuffer);
    GETPROC(glfuncs, BindRenderbuffer);
    GETPROC(glfuncs, FramebufferRenderbuffer);
    GETPROC(glfuncs, RenderbufferStorageMultisample);
    GETPROC(glfuncs, ClearBufferfv);
    GETPROC(glfuncs, BlitFramebuffer);

    GETPROC(glfuncs, Enable);
    GETPROC(glfuncs, Disable);

    GETPROC(glfuncs, PolygonMode);
    GETPROC(glfuncs, CullFace);
    GETPROC(glfuncs, FrontFace);
    GETPROC(glfuncs, DepthMask);
    GETPROC(glfuncs, DepthFunc);

    GETPROC(glfuncs, DrawArraysInstanced);
    GETPROC(glfuncs, DrawElementsInstanced);

    GETPROC(glfuncs, GetIntegerv);

    return true;
}
