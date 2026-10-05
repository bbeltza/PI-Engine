#include "render_gl.h"

bool fn_render_gl_create_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv, const PI_WORLDENV_DESC* pdesc)
{
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
    
    enum {
        PI_CULL_FRONT_BACK = PI_CULL_FRONT | PI_CULL_BACK
    };

    switch (pdesc->rasterizer->cullflags & PI_CULL_FRONT_BACK) {
        case PI_CULL_BACK:
            hworldenv->cullface = GL_BACK;
            break;
        case PI_CULL_FRONT:
            hworldenv->cullface = GL_FRONT;
            break;
        case PI_CULL_FRONT_BACK:
            hworldenv->cullface = GL_FRONT_AND_BACK;
            break;
        default:
            hworldenv->cullface = 0;
            break;
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
        switch (hworldenv->depthfunc) {
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

        hworldenv->depthmask = pdesc->depth->writeenable;
    }

/* ~Stencil state~ */
    /* ... */
/* Multisample state */

    hworldenv->samples = pdesc->multisample->samples;

    return true;
}
void fn_render_gl_destroy_worldenvironment(pi_GLbackend* hbackend, pi_GLworldenv* hworldenv)
{
    
}
