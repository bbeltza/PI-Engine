#include "render_gl.h"

static unsigned get_shader_index(PI_WORLDENV_EFFECTS effects)
{
    switch (effects) {
        case PI_WORLDEFFECT_NONE: return 0;
        case PI_WORLDEFFECT_LIGHTING: return 1;
        case PI_WORLDEFFECT_LIGHTING | PI_WORLDEFFECT_SPECULAR: return 2;
        default: PI_ASSUME(0);
    }
}

bool fn_render_gl_create_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv, const PI_WORLDENV_DESC* pdesc)
{
/* Shader state */
    hworldenv->shader = hbackend->shaders[get_shader_index(pdesc->effects)];

/* Rasterizer state */
    switch (pdesc->rasterizer->fillmode)
    {
        case PI_FILL_SOLID:
            hworldenv->polygonmode = GL_FILL;
            break;
        case PI_FILL_WIREFRAME:
            hworldenv->polygonmode = GL_LINE;
            break;
    }

    switch (pdesc->rasterizer->cullflags & (PI_CULL_BACK | PI_CULL_FRONT)) {
        case PI_CULL_BACK:
            hworldenv->cullface = GL_BACK;
            break;
        case PI_CULL_FRONT:
            hworldenv->cullface = GL_FRONT;
            break;
        case 0:
            hworldenv->cullface = 0;
            break;
        default:
            PI_ASSUME(0);
    }
    if (pdesc->rasterizer->cullflags & PI_CULL_FRONT_COUNTERCLOCKWISE)
        hworldenv->frontface = GL_CCW;
    else
        hworldenv->frontface = GL_CW;

/* Depth state */
    if (!pdesc->depth) {
        hworldenv->depthenable = GL_FALSE;
    }
    else {
        switch (pdesc->depth->depthcompare) {
            case PI_COMPARE_NEVER:
                hworldenv->depthfunc = GL_NEVER;
                break;
            case PI_COMPARE_LESS:
                hworldenv->depthfunc = GL_LESS;
                break;
            case PI_COMPARE_EQUAL:
                hworldenv->depthfunc = GL_EQUAL;
                break;
            case PI_COMPARE_LESS_EQUAL:
                hworldenv->depthfunc = GL_LEQUAL;
                break;
            case PI_COMPARE_GREATER:
                hworldenv->depthfunc = GL_GREATER;
                break;
            case PI_COMPARE_NOT_EQUAL:
                hworldenv->depthfunc = GL_NOTEQUAL;
                break;
            case PI_COMPARE_GREATER_EQUAL:
                hworldenv->depthfunc = GL_GEQUAL;
                break;
            case PI_COMPARE_ALWAYS:
                hworldenv->depthfunc = GL_ALWAYS;
                break;
        };

        hworldenv->depthmask = pdesc->depth->depthwrite;
        hworldenv->depthenable = GL_TRUE;
    }

/* ~Stencil state~ */
    /* ... */
/* Multisample state */

    hworldenv->samples = pdesc->multisample->samples == 1 ? 0 :
                    pdesc->multisample->samples;
    return true;
}
void fn_render_gl_destroy_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv)
{
    
}
