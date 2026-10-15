#include "app.h"

#include <piengine/pi_math.h>
#include <string.h>

#define _SCALE 10.0f

const float SCALE = _SCALE;
const float INVSCALE = 1/_SCALE;

#undef _SCALE

static bool create_worldenv(PI_HGAME self);
static bool create_scene(PI_HGAME self);
static bool create_mesh(PI_HGAME self);

static void init_rectangle(PI_HGAME self, RECTANGLE* rect);

static void update_dirty(PI_HGAME self);
static bool update_rectangle(RECTANGLE* rect, const PI_VECTOR4* color);

PI_EXTERN_C bool app_start(PI_HGAME self, va_list args)
{
    if (!create_worldenv(self)) {
        return false;
    }
    if (!create_scene(self)) {
        app_close(self);
        return false;
    }

    if (!create_mesh(self)) {
        app_close(self);
        return false;
    }

    init_rectangle(self, &self->rect1);
    init_rectangle(self, &self->rect2);

    //self->htest1 = pi_add_scene_mesh(self->hscene, self->hquadmesh, NULL);
    //self->htest2 = pi_add_scene_mesh(self->hscene, self->hquadmesh, NULL);

    {
        float rect1angle = PI_DEGF(40);
        float c = cosf(rect1angle),
              s = sinf(rect1angle);
        self->rect1.x_axis.x = c;
        self->rect1.x_axis.y = -s;
        self->rect1.y_axis.x = s;
        self->rect1.y_axis.y = c;
        self->rect1.h_size.x = 2.0f;
        self->rect1.h_size.y = 2.0f;
        update_rectangle(&self->rect1, &(PI_VECTOR4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    self->hmainwnd = pi_get_main_window(self);
    PI_ASSERT(self->hmainwnd != NULL);

    memset(&self->dirty, 0xFF, sizeof(self->dirty));

    pi_enable_scene(self->hscene, 0);
    pi_set_scene_clear_color(self->hscene, 0.31f, 0.3f, 0.5f, 1.0f);
    return true;
}

PI_EXTERN_C void app_close(PI_HGAME self) {
    pi_destroy_mesh(self->hquadmesh);
    pi_destroy_scene(self->hscene);
    pi_release_worldenvironment(self->hworldenv);
}

PI_EXTERN_C bool app_event(PI_HGAME self, PI_EVENT* event)
{
    switch (event->type) {
        case PI_EVENT_WINDOW_SIZE:
            self->dirty.proj = true;
            break;
        case PI_EVENT_MOUSEMOVE: if (event->motion.state & PI_MSTATE_LEFT) {
            self->dirty.rect2 = true;
            int wh[2];
            pi_get_window_logical_size(wh, event->motion.hwindow);
            float hw = wh[0]*0.5f;
            float hh = wh[1]*0.5f;

            self->rect2.position.x = (event->motion.x - hw);
            self->rect2.position.y = (event->motion.y - hh);
            self->rect2.position.x /= hw * INVSCALE * (hh/hw);
            self->rect2.position.y /= hh * INVSCALE;
        } break;
        case PI_EVENT_MOUSEWHEEL:
            if (event->wheel.amnt_y) {
                self->dirty.rect2angle = true;
                self->rect2angle += -event->wheel.amnt_y * PI_DEGF(5);
            }
            if (event->wheel.amnt_x) {
                self->dirty.rect2 = true;
                self->rect2.h_size.x += event->wheel.amnt_x * 0.25f;
            }
            break;
        default:
            break;
    }
    return true;
}

PI_EXTERN_C bool app_update(PI_HGAME self)
{
    update_dirty(self);
    return true;
}

/**/

static bool create_worldenv(PI_HGAME self)
{
    PI_WORLDENV_RASTERIZER raster;
    raster.cullflags = PI_CULL_NONE;
    raster.depthclip = false;
    raster.fillmode = PI_FILL_SOLID;

    PI_WORLDENV_MSAA msaa;
    msaa.samples = 4;

    PI_WORLDENV_DESC desc;
    desc.effects = PI_WORLDEFFECT_NONE;
    desc.rasterizer = &raster;
    desc.depth = NULL;
    desc.multisample = &msaa;

    self->hworldenv = pi_create_worldenvironment(self, &desc);
    return self->hworldenv != NULL;
}

static bool create_scene(PI_HGAME self)
{
    self->hscene = pi_create_scene(self);
    if (!self->hscene) {
        return false;
    }

    if (!pi_init_render_scene(self->hscene, self->hworldenv)) {
        goto FAIL;
    }
    return true;
FAIL:
    pi_destroy_scene(self->hscene);
    self->hscene = NULL;
    return false;
}

static bool create_mesh(PI_HGAME self)
{
    PI_VERTEX QUAD_VERTICES[] = {
        { { -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
    };
    uint16_t QUAD_INDICES[] = {
        0, 1, 2,
        2, 3, 1
    };

    self->hquadmesh = pi_create_mesh_indexed(self,
            PI_ARRAYSIZE(QUAD_VERTICES), QUAD_VERTICES,
            PI_ARRAYSIZE(QUAD_INDICES), QUAD_INDICES, PI_INDEX_U16);
    return self->hquadmesh != NULL;
}

/**/

bool test_axis(const PI_VECTOR2* t, const PI_VECTOR2* axis, float w_or_h, const PI_VECTOR2* scaled_xaxis, const PI_VECTOR2* scaled_yaxis, float* tdot, float* rdot) {
    float dotl = pi_vector_dot2(*t, *axis);
    dotl = fabsf(dotl);
    float dotr = w_or_h +
        fabsf(pi_vector_dot2(*scaled_xaxis, *axis)) +
        fabsf(pi_vector_dot2(*scaled_yaxis, *axis));

    if (tdot)
        *tdot = dotl;
    if (rdot)
        *rdot = dotr;
    return dotl > dotr;
}

static PI_MATRIX_4X4 aabb2_to_4x4(PI_VECTOR2 min, PI_VECTOR2 max) {
    float lenx = max.x - min.x;
    float leny = max.y - min.y;

    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        lenx, 0.0f, 0.0f, 0.0f,
        0.0f, leny, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        min.x + lenx*0.5f, min.y + leny*0.5f, 0.0f, 1.0f 
    };
}

static void update_dirty(PI_HGAME self)
{
    if (self->dirty.proj) {
        int wh[2];
        pi_get_window_logical_size(wh, self->hmainwnd);
        self->dirty.proj = false;
    
        float ratio = (float)wh[0]/wh[1];
        float w = ratio * SCALE;
        float h = 1.0f * SCALE;

        PI_MATRIX_4X4 m = pi_matrix_ortho(-w, w, -h, h, 0.0f, 1.0f);
        pi_set_scene_projection(self->hscene, &m);
    }

    if (self->dirty.rect2angle) {
        self->dirty.rect2 = true;
        self->dirty.rect2angle = false;

        float c = cosf(self->rect2angle);
        float s = sinf(self->rect2angle);
        self->rect2.x_axis.x = c;
        self->rect2.x_axis.y = -s;
        self->rect2.y_axis.x = s;
        self->rect2.y_axis.y = c;
    }

    if (self->dirty.rect2) {
        self->dirty.rect2 = false;
        PI_VECTOR4 col = { 1.0f, 0.0f, 0.0f, 1.0f };

        {
            PI_VECTOR2 t = pi_vec2_sub(self->rect1.position, self->rect2.position);
            PI_VECTOR2 xaxis1 = pi_vec2_muls(self->rect1.x_axis, self->rect1.h_size.x);
            PI_VECTOR2 xaxis2 = pi_vec2_muls(self->rect2.x_axis, self->rect2.h_size.x);
            PI_VECTOR2 yaxis1 = pi_vec2_muls(self->rect1.y_axis, self->rect1.h_size.y);
            PI_VECTOR2 yaxis2 = pi_vec2_muls(self->rect2.y_axis, self->rect2.h_size.y);

            //bool test_axis(const PI_VECTOR2* t, const PI_VECTOR2* axis, float w_or_h, const PI_VECTOR2* other_axis, float* tdot, float* rdot);
            if (test_axis(&t, &self->rect1.x_axis, self->rect1.h_size.x, &xaxis2, &yaxis2, NULL, NULL) ||
                test_axis(&t, &self->rect1.y_axis, self->rect1.h_size.y, &xaxis2, &yaxis2, NULL, NULL) ||
                test_axis(&t, &self->rect2.x_axis, self->rect2.h_size.x, &xaxis1, &yaxis1, NULL, NULL) ||
                test_axis(&t, &self->rect2.y_axis, self->rect2.h_size.y, &xaxis1, &yaxis1, NULL, NULL)
                    ) {
                col.z = 1.0f;
            }
            
            {
                float dotl = pi_vector_dot2(t, self->rect1.y_axis);
                dotl = fabsf(dotl);
                float dotr = self->rect1.h_size.y +
                    fabsf(pi_vector_dot2(xaxis2, self->rect1.y_axis)) +
                    fabsf(pi_vector_dot2(yaxis2, self->rect1.y_axis));
                if (dotl > dotr) {
                    col.z = 1.0f;
                }
            } {
                float dotl = pi_vector_dot2(t, self->rect2.x_axis);
                dotl = fabsf(dotl);
                float dotr = self->rect2.h_size.x +
                    fabsf(pi_vector_dot2(xaxis1, self->rect2.x_axis)) +
                    fabsf(pi_vector_dot2(yaxis1, self->rect2.x_axis));
                if (dotl > dotr) {
                    col.z = 1.0f;
                }
            } {
                float dotl = pi_vector_dot2(t, self->rect2.y_axis);
                dotl = fabsf(dotl);
                float dotr = self->rect2.h_size.y +
                    fabsf(pi_vector_dot2(xaxis1, self->rect2.y_axis)) +
                    fabsf(pi_vector_dot2(yaxis1, self->rect2.y_axis));
                if (dotl > dotr) {
                    col.z = 1.0f;
                }
            }

        }

        update_rectangle(&self->rect2, &col);
    }
}

/**/

static void init_rectangle(PI_HGAME self, RECTANGLE* rect)
{
    rect->hinstance = pi_add_scene_mesh(self->hscene, self->hquadmesh, NULL);
    rect->x_axis = PI_INIT_STRUCT(PI_VECTOR2){ 1.0f, 0.0f };
    rect->y_axis = PI_INIT_STRUCT(PI_VECTOR2){ 0.0f, 1.0f };
    rect->h_size = PI_INIT_STRUCT(PI_VECTOR2){ 0.5f, 0.5f };
    rect->position = PI_INIT_STRUCT(PI_VECTOR2){ 0.0f, 0.0f };
}

static bool update_rectangle(RECTANGLE* rect, const PI_VECTOR4* color)
{
    PI_VECTOR2 size = {
        rect->h_size.x * 2,
        rect->h_size.y * 2
    };

    PI_INSTANCE i = {
        {
            rect->x_axis.x*size.x, rect->x_axis.y*size.x, 0.0f, 0.0f,
            rect->y_axis.x*size.y, rect->y_axis.y*size.y, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            rect->position.x, rect->position.y, 0.0f, 1.0f
        },
        *color
    };
    return pi_update_mesh_instance(rect->hinstance, &i);
}
