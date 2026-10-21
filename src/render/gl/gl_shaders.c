#include "render_gl.h"

static const char SHADERS_COMMON[] =
    "#version 330 core\n"
    "\n"
    "layout (std140) uniform PI_FSENV {\n"
        "vec3 ambient_color;\n"
        "vec3 view_position;\n"
        "float specular_exponent;\n"
        "vec4 specular_color;\n"
        "uint num_dirlights;\n"
        "uint num_ptlights;\n"
    "} u_fsenv;\n"
    ;

static const char SHADERS_VS[] =
    "layout (std140) uniform PI_VSENV {\n"
        "mat4 viewproj;\n"
    "} u_vsenv;\n"
    "\n"
    "layout (location = 0) in vec4 v_pos;\n"
    "layout (location = 1) in vec4 v_col;\n"
    "layout (location = 1) in vec3 v_normal;\n"
    "layout (location = 2) in mat4 i_transform;\n"
                    /* 2 .. 5 */
    "layout (location = 6) in vec4 i_albedo;\n"
    "#ifdef WORLDEFFECT_LIGHTING\n"
        "layout (location = 7) in mat3 i_normalmat;\n"
        "\n"
        "struct PI_DIRECTIONAL_LIGHT {\n"
            "vec3 direction;\n"
            "vec4 color;\n"
        "};\n"
        "layout (std140) uniform PI_DIRLIGHTS {\n"
            "PI_DIRECTIONAL_LIGHT dir_lights[MAX_DIRECTIONAL_LIGHTS];\n"
        "};\n"
    "#endif\n"
    "\n"
    "out vec4 f_col;\n"
    "#ifdef WORLDEFFECT_LIGHTING\n"
        "out vec3 f_diffuse;\n"
        "out vec3 f_normal;\n"
        "out vec3 f_worldpos;\n"
    "#endif\n"
    "\n"
    "void main() {\n"
        "vec4 pos = i_transform * v_pos;\n"
        "f_col = i_albedo;\n"
        "#ifdef WORLDEFFECT_LIGHTING\n"
            "vec3 normal = i_normalmat * v_normal;\n"
            "normal = normalize(normal);\n"
            "f_normal = normal;\n"
            "f_worldpos = pos.xyz;\n"
            "f_diffuse = u_fsenv.ambient_color;"
            "for (uint i = 0u; i < u_fsenv.num_dirlights; i++) {\n"
                "PI_DIRECTIONAL_LIGHT light = dir_lights[i];\n"
                "float diffuse = dot(light.direction.xyz, -normal);\n"
                "diffuse = max(diffuse, 0.0);\n"
                "diffuse = pow(diffuse, 2.2);\n"
                "\n"
                "f_diffuse += diffuse * light.color.rgb * light.color.a;\n"
            "}\n"
        "#else\n"
            "f_col *= v_col;\n"
        "#endif\n"
        "gl_Position = u_vsenv.viewproj * pos;\n"
        "gl_Position.z = gl_Position.z*2.0 - gl_Position.w;\n"
    "}\n"
    ;
static const char SHADERS_FS[] = 
    "#ifdef WORLDEFFECT_LIGHTING\n"
        "\n"
        "struct PI_POINT_LIGHT {\n"
            "vec3 position;\n"
            "vec4 color;\n"
            "float range;\n"
            "float attenuation;\n"
        "};\n"
        "layout (std140) uniform PI_PTLIGHTS {\n"
            "PI_POINT_LIGHT pt_lights[MAX_POINT_LIGHTS];\n"
        "};\n"
        "\n"
        "in vec3 f_normal;\n"
        "in vec3 f_worldpos;\n"
        "in vec3 f_diffuse;\n"
    "#endif\n"
    "in vec4 f_col;\n"
    "\n"
    "layout (location = 0) out vec4 o_col;\n"
    "\n"
    "void main() {\n"
        "vec4 color = f_col;\n"
        "#ifdef WORLDEFFECT_LIGHTING\n"
        "{\n"
            "vec3 lcol = f_diffuse;\n"
            "#ifdef WORLDEFFECT_SPECULAR\n"
                "vec3 lspec = vec3(0.0);\n"
                "vec3 viewdir = u_fsenv.view_position - f_worldpos;\n"
                "viewdir = normalize(viewdir);\n"
            "#endif\n"
            "for (uint i = 0u; i < u_fsenv.num_ptlights; i++) {\n"
                "PI_POINT_LIGHT light = pt_lights[i];\n"
                "vec3 direction = light.position.xyz - f_worldpos;\n"
                "direction = normalize(direction);\n"
                "float diffuse = dot(direction, f_normal);\n"
                "diffuse = max(diffuse, 0.0);\n"
                "diffuse = pow(diffuse, 2.2);\n"
                "\n"
                "float d = distance(light.position.xyz, f_worldpos);\n"
                "diffuse *= 1.0 - smoothstep(light.range - light.attenuation, light.range, d);\n"
                "\n"
                "#ifdef WORLDEFFECT_SPECULAR\n"
                    "vec3 reflection = reflect(direction, viewdir);\n"
                    "reflection = normalize(reflection);\n"
                    "float spec = dot(reflection, f_normal);\n"
                    "spec = max(spec, 0.0);\n"
                    "spec = pow(spec, u_fsenv.specular_exponent);\n"
                    "lspec += diffuse * spec * u_fsenv.specular_color.rgb * u_fsenv.specular_color.a;\n"
                "#endif\n"
                "lcol += diffuse * light.color.rgb * light.color.a;\n"
            "}\n"
            "\n"
            "color.rgb *= lcol;\n"
            "#ifdef WORLDEFFECT_SPECULAR\n"
                "color.rgb += lspec;\n"
            "#endif\n"
        "}\n"
        "#endif\n"
        "o_col = color;\n"
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
    printf("ERROR: Failed compiling shader \"%s\"\n", tag),
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
GLuint pi_gl_compile_shader(pi_GLbackend* hbackend, const GLchar* defines_code)
{
    GLuint vs = gl.CreateShader(GL_VERTEX_SHADER);
    GLuint fs = gl.CreateShader(GL_FRAGMENT_SHADER);
    PI_ASSERT(vs != 0);
    PI_ASSERT(fs != 0);

    const GLchar* shadersources[3] = {
        SHADERS_COMMON,
        defines_code
    };
    shadersources[2] = SHADERS_VS;        
    gl.ShaderSource(vs, PI_ARRAYSIZE(shadersources), shadersources, NULL);
    shadersources[2] = SHADERS_FS;
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

    GLuint ubindex = gl.GetUniformBlockIndex(program, "PI_VSENV");
    PI_ASSERT(ubindex != GL_INVALID_INDEX);
    gl.UniformBlockBinding(program, ubindex, 0);
    GLuint ubfsindex = gl.GetUniformBlockIndex(program, "PI_FSENV");
    PI_ASSERT(ubfsindex != GL_INVALID_INDEX);


    GLuint ubdirlightsindex = gl.GetUniformBlockIndex(program, "PI_DIRLIGHTS");

    if (ubdirlightsindex != GL_INVALID_INDEX) {
        GLuint ubptlightsindex = gl.GetUniformBlockIndex(program, "PI_PTLIGHTS");
        //PI_ASSERT(ubdirlightsindex != GL_INVALID_INDEX);
        PI_ASSERT(ubptlightsindex != GL_INVALID_INDEX);

        gl.UniformBlockBinding(program, ubfsindex, 1);
        gl.UniformBlockBinding(program, ubdirlightsindex, 2);
        gl.UniformBlockBinding(program, ubptlightsindex, 3);
    }

    return program;
}
