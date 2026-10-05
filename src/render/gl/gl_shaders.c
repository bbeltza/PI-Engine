#include "render_gl.h"

static const char SHADERS_VS[] =
    "#version 330 core\n"
    "\n"
    "uniform PI_UBUFFER {\n"
        "mat4 viewproj;\n"
    "} u_scene;\n"
    "\n"
    "layout (location = 0) in vec4 v_pos;\n"
    "layout (location = 1) in vec4 v_col;\n"
    "layout (location = 2) in mat4 i_transform;\n"
                    /* 2 .. 5 */
    "layout (location = 6) in vec4 i_albedo;\n"
    "\n"
    "layout (location = 0) out vec4 f_col;\n"
    "\n"
    "void main() {\n"
        "vec4 pos = i_transform * v_pos;\n"
        "fcol = v_col * i_albedo;\n"
        "gl_Position = u_scene.viewproj * pos;\n"
    "}\n"
    ;
static const char SHADERS_FS[] =
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec4 f_col;\n"
    "\n"
    "layout (location = 0) out vec4 o_col;\n"
    "\n"
    "void main() {\n"
        "o_col = f_col;"
    "}\n"
    ;

GLuint pi_gl_compile_shader(pi_GLbackend* hbackend, PI_WORLDENV_EFFECTS effects)
{
    return 0;
}

void pi_gl_clean_cached_shaders(pi_GLbackend* hbackend)
{
    
}
