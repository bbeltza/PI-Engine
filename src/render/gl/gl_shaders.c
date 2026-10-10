#include "render_gl.h"

static const char SHADERS_COMMON[] =
    "#version 330 core\n"
    "\n"
    ;

static const char SHADERS_VS[] =
    "layout (std140) uniform PI_UBUFFER {\n"
        "mat4 viewproj;\n"
    "} u_scene;\n"
    "\n"
    "layout (location = 0) in vec4 v_pos;\n"
    "layout (location = 1) in vec4 v_col;\n"
    "layout (location = 2) in mat4 i_transform;\n"
                    /* 2 .. 5 */
    "layout (location = 6) in vec4 i_albedo;\n"
    "\n"
    "out vec4 f_col;\n"
    "\n"
    "void main() {\n"
        "vec4 pos = i_transform * v_pos;\n"
        "f_col = v_col * i_albedo;\n"
        "gl_Position = u_scene.viewproj * pos;\n"
        "gl_Position.z = gl_Position.z*2.0 - gl_Position.w;\n"
    "}\n"
    ;
static const char SHADERS_FS[] =
    "in vec4 f_col;\n"
    "\n"
    "layout (location = 0) out vec4 o_col;\n"
    "\n"
    "void main() {\n"
        "o_col = f_col;"
    "}\n"
    ;


bool pi_gl_evaluate_shader_status(pi_GLbackend* hbackend, GLuint shader, const char* tag)
{
    GLint compile_status = 0;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_TRUE) {
        return true;
    }

    GLsizei len = 0;
    GLchar buffer[512];
    gl.GetShaderInfoLog(shader, sizeof(buffer), &len, buffer);
    fwrite(buffer, len, 1, stderr);
    return false;
}

bool pi_gl_evaluate_program_status(pi_GLbackend* hbackend, GLuint program, const char* tag)
{
    GLint link_status = 0;
    gl.GetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_TRUE) {
        return true;
    }

    GLsizei len = 0;
    GLchar buffer[512];
    gl.GetProgramInfoLog(program, sizeof(buffer), &len, buffer);
    fwrite(buffer, len, 1, stderr);
    return false;
}
GLuint pi_gl_compile_shader(pi_GLbackend* hbackend, PI_WORLDENV_EFFECTS effects)
{
    if (effects != PI_WORLDEFFECT_NONE) {
        fputs("pi_gl_compile_shader(): Effects are not yet supported, `effects` must for now be \"PI_WORLDEFFECT_NONE\".\n", stderr);
        return 0;
    }

    GLuint vs = gl.CreateShader(GL_VERTEX_SHADER);
    GLuint fs = gl.CreateShader(GL_FRAGMENT_SHADER);
    PI_ASSERT(vs != 0);
    PI_ASSERT(fs != 0);

    const GLchar* shadersources[2] = {
        SHADERS_COMMON
    };
    shadersources[1] = SHADERS_VS;        
    gl.ShaderSource(vs, PI_ARRAYSIZE(shadersources), shadersources, NULL);
    shadersources[1] = SHADERS_FS;
    gl.ShaderSource(fs, PI_ARRAYSIZE(shadersources), shadersources, NULL);

    gl.CompileShader(vs);
    if (!pi_gl_evaluate_shader_status(hbackend, vs, "vs")) {
        gl.DeleteShader(vs);
        gl.DeleteShader(fs);
        return 0;
    }
    gl.CompileShader(fs);
    if (!pi_gl_evaluate_shader_status(hbackend, fs, "fs")) {
        gl.DeleteShader(vs);
        gl.DeleteShader(fs);
        return 0;
    }

    GLuint program = gl.CreateProgram();
    PI_ASSERT(program != 0);

    gl.AttachShader(program, vs);
    gl.AttachShader(program, fs);
    gl.LinkProgram(program);

    gl.DetachShader(program, fs);
    gl.DetachShader(program, vs);
    gl.DeleteShader(fs);
    gl.DeleteShader(vs);
    if (!pi_gl_evaluate_program_status(hbackend, program, "program")) {
        gl.DeleteProgram(program);
        return 0;
    }

    GLuint ubindex = gl.GetUniformBlockIndex(program, "PI_UBUFFER");
    PI_ASSERT(ubindex != GL_INVALID_INDEX);
    gl.UniformBlockBinding(program, ubindex, 0);

    return program;
}

void pi_gl_clean_cached_shaders(pi_GLbackend* hbackend)
{
    
}
