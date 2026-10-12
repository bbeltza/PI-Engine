#ifndef PI_RENDER_H
#define PI_RENDER_H

#include <piengine/pi_common.h>
#include <piengine/pi_scalars.h>

typedef struct PI_WORLDENV_IMPL *PI_HWORLDENV;
typedef struct PI_MESH_IMPL *PI_HMESH;
typedef struct PI_INSTANCE_IMPL *PI_HINSTANCE;
typedef struct PI_LIGHT_IMPL *PI_HLIGHT;

#define PI_HMESH_DESTRUCTOR pi_destroy_mesh
#define PI_HWORLDENV_DESTRUCTOR pi_release_worldenvironment
#define PI_HINSTANCE_DESTRUCTOR pi_remove_scene_mesh
#define PI_HLIGHT_DESTRUCTOR pi_remove_light

typedef struct PI_VERTEX {
    PI_VECTOR3 pos;
    union {
        PI_VECTOR3 col;
        PI_VECTOR3 normal;
    };
} PI_VERTEX;

typedef struct PI_INSTANCE {
    PI_MATRIX_4X4 transform
        PI_MEMBER_DEFAULT({
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            });
    PI_VECTOR4 col
        PI_MEMBER_DEFAULT({ 1.0f, 1.0f, 1.0f, 1.0f });
} PI_INSTANCE;

typedef struct PI_LIGHT_PROPERTIES
{
    PI_VECTOR3X ambient_color PI_MEMBER_DEFAULT( { 0.0f, 0.0f, 0.0f, 0.0f } );
        /* Only used when specular lighting is on */
    PI_VECTOR3 view_position;
    float specular_exponent PI_MEMBER_DEFAULT( 22.0f );
    PI_VECTOR4 specular_color PI_MEMBER_DEFAULT( { 1.0f, 1.0f, 1.0f, 1.0f } );
} PI_LIGHT_PROPERTIES;

typedef struct PI_DIRECTIONAL_LIGHT {
    PI_VECTOR3X direction;
    PI_VECTOR4 color;
} PI_DIRECTIONAL_LIGHT;

typedef struct PI_POINT_LIGHT {
    PI_VECTOR4 position;
    PI_VECTOR4 color;
    float range;
    float attenuation;

    float _reserved1;
    float _reserved2;
} PI_POINT_LIGHT;

typedef union PI_LIGHT {
    PI_DIRECTIONAL_LIGHT directional;
    PI_POINT_LIGHT point;
} PI_LIGHT;

typedef enum PI_LIGHTTYPE {
    PI_LIGHT_NONE = 0,
    PI_LIGHT_DIRECTIONAL = 1,
    PI_LIGHT_POINT = 2
} PI_LIGHTTYPE;

typedef enum PI_INDEX {
    PI_INDEX_UNKNOWN = 0,
    PI_INDEX_U16 = 1,
    PI_INDEX_U32 = 2
} PI_INDEX;

typedef enum PI_FILLMODE {
    PI_FILL_SOLID = 0,
    PI_FILL_WIREFRAME = 1
} PI_FILLMODE;

typedef enum PI_CULLFLAG {
    PI_CULL_NONE = 0x00000,
    PI_CULL_FRONT = 0x00001,
    PI_CULL_BACK = 0x00002,
    
    PI_CULL_FRONT_COUNTERCLOCKWISE = 0x10000
} PI_CULLFLAG;
typedef uint16_t PI_CULLFLAGS;

typedef enum PI_COMPARE_FUNC {
    PI_COMPARE_NEVER = 0,
    PI_COMPARE_LESS = 1,
    PI_COMPARE_EQUAL = 2,
    PI_COMPARE_LESS_EQUAL = 3,
    PI_COMPARE_GREATER = 4,
    PI_COMPARE_NOT_EQUAL = 5,
    PI_COMPARE_GREATER_EQUAL = 6,
    PI_COMPARE_ALWAYS = 7
} PI_COMPARE_FUNC;

typedef enum PI_WORLDEFFECT {
    PI_WORLDEFFECT_NONE = 0x00000000,
    PI_WORLDEFFECT_LIGHTING = 0x00000001, /* Enable lighting */
    PI_WORLDEFFECT_SPECULAR = 0x00000002, /* Enable specular shading (likely to be phong-shading), LIGHTING must be enabled */

    // PI_WORLDEFFECT_SHADOWS = 0x00000004, /* Enable shadow mapping */
} PI_WORLDEFFECT;
typedef uint32_t PI_WORLDENV_EFFECTS;

//


typedef struct PI_WORLDENV_RASTERIZER {
    PI_FILLMODE fillmode PI_MEMBER_DEFAULT(PI_FILL_SOLID);
    PI_CULLFLAGS cullflags PI_MEMBER_DEFAULT(PI_CULL_NONE);
    bool depthclip;
} PI_WORLDENV_RASTERIZER;
typedef struct PI_WORLDENV_DEPTH {
    bool depthwrite PI_MEMBER_DEFAULT(true);
    PI_COMPARE_FUNC depthcompare PI_MEMBER_DEFAULT(PI_COMPARE_LESS);
} PI_WORLDENV_DEPTH;
typedef struct PI_WORLDENV_STENCIL {
    uint32_t _unused;
} PI_WORLDENV_STENCIL;
typedef struct PI_WORLDENV_MSAA {
    uint8_t samples PI_MEMBER_DEFAULT(1);
    bool    best_quality;
} PI_WORLDENV_MSAA;

typedef struct PI_WORLDENV_DESC {
    PI_WORLDENV_EFFECTS effects;

    const PI_WORLDENV_RASTERIZER* rasterizer;
    const PI_WORLDENV_DEPTH* depth;
    const PI_WORLDENV_STENCIL* _reserved1;
    const PI_WORLDENV_MSAA* multisample;
} PI_WORLDENV_DESC;

#ifdef PI_CPP

namespace PI
{
    struct WORLDENV_DESC: PI_WORLDENV_DESC {
        constexpr WORLDENV_DESC(): PI_WORLDENV_DESC{} {}
        constexpr WORLDENV_DESC(
            PI_WORLDENV_EFFECTS effects,
            const PI_WORLDENV_RASTERIZER& rasterizer,
            const PI_WORLDENV_DEPTH& depth,
            const PI_WORLDENV_MSAA& multisample
        ): 
            PI_WORLDENV_DESC{
                    effects,
                    &rasterizer,
                    &depth,
                    NULL,
                    &multisample
                } {}
        constexpr WORLDENV_DESC(
            PI_WORLDENV_EFFECTS effects,
            const PI_WORLDENV_RASTERIZER& rasterizer,
            const PI_WORLDENV_DEPTH& depth
        ):
            PI_WORLDENV_DESC{
                effects,
                &rasterizer,
                &depth,
                NULL,
                NULL
            } {}
        constexpr WORLDENV_DESC(
            PI_WORLDENV_EFFECTS effects,
            const PI_WORLDENV_RASTERIZER& rasterizer
        ):
            PI_WORLDENV_DESC{
                effects,
                &rasterizer,
                NULL,
                NULL,
                NULL
            } {}
    };
}

#endif

//
//

PI_EXTERN_C PI_HMESH pi_create_mesh_indexed(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type);
PI_EXTERN_C PI_HMESH pi_create_mesh(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[]);
PI_EXTERN_C void pi_destroy_mesh(PI_HMESH hmesh);

PI_EXTERN_C PI_HWORLDENV pi_create_worldenvironment(PI_HGAME hgame, const PI_WORLDENV_DESC* pdesc);
PI_EXTERN_C void pi_release_worldenvironment(PI_HWORLDENV hworldenv);

PI_EXTERN_C PI_HSCENE pi_init_render_scene(PI_HSCENE hscene, PI_HWORLDENV hworldenv);
// void pi_deinit_render_scene(PI_HSCENE hscene);

PI_EXTERN_C bool pi_set_scene_clear_color(PI_HSCENE hscene, float r, float g, float b, float a);
PI_EXTERN_C bool pi_set_scene_projection(PI_HSCENE hscene, const PI_MATRIX_4X4* mproj);
PI_EXTERN_C bool pi_set_scene_view(PI_HSCENE hscene, const PI_MATRIX_4X4* mview);

PI_EXTERN_C PI_HINSTANCE pi_add_scene_mesh(PI_HSCENE hscene, PI_HMESH hmesh, const PI_INSTANCE* instance);
PI_EXTERN_C void pi_remove_scene_mesh(PI_HINSTANCE hmeshinst);
PI_EXTERN_C bool pi_update_mesh_instance(PI_HINSTANCE hmeshinst, const PI_INSTANCE* instance);

PI_EXTERN_C PI_HLIGHT pi_add_light(PI_HSCENE hscene, PI_LIGHTTYPE type, const PI_LIGHT* initial_parameters);
PI_EXTERN_C void pi_remove_light(PI_HLIGHT hlight);
PI_EXTERN_C bool pi_update_light(PI_HLIGHT hlight, const PI_LIGHT* light);

PI_EXTERN_C bool pi_set_light_properties(PI_HSCENE hscene, const PI_LIGHT_PROPERTIES* properties);

#endif

/* Include backend part! */

#if defined(PI_INCLUDE_BACKEND_API) && !defined(PI_RENDER_BACKEND_H)
#define PI_RENDER_BACKEND_H

typedef struct PI_RENDER_CONFIGURE {
    unsigned w;
    unsigned h;
} PI_RENDER_CONFIGURE;

typedef struct PI_RENDERBACKEND *PI_HRENDERBACKEND;
typedef struct PI_RENDERBACKEND_WORLDENV *PI_HRENDERBACKEND_WORLDENV;
typedef struct PI_RENDERBACKEND_SCENE *PI_HRENDERBACKEND_SCENE;
typedef struct PI_RENDERBACKEND_MESH *PI_HRENDERBACKEND_MESH;
typedef struct PI_RENDERBACKEND_INSTBUFFER *PI_HRENDERBACKEND_INSTBUFFER;

typedef void (*PI_PFN_BACKEND_RENDER_CLOSE)(PI_HRENDERBACKEND hbackend);

typedef bool (*PI_PFN_BACKEND_RENDER_CONFIGURE)(PI_HRENDERBACKEND hbackend, const PI_RENDER_CONFIGURE* configure);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_WORLDENVIRONMENT)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_WORLDENV_DESC* pdesc);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_WORLDENVIRONMENT)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_WORLDENV hworldenv);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, const PI_VERTEX* vertices, unsigned num_vertices, const void* indices, unsigned num_indices, PI_INDEX index_type);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer);

typedef bool (*PI_PFN_BACKEND_RENDER_ALLOC_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned size, const PI_INSTANCE initial_data[]);
typedef bool (*PI_PFN_BACKEND_RENDER_UPDATE_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned pos, unsigned size, const PI_INSTANCE data[]);

typedef void (*PI_PFN_BACKEND_RENDER_UPDATE_SCENE_VP)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const PI_MATRIX_4X4* vpmatrix);
typedef void (*PI_PFN_BACKEND_RENDER_BEGIN_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, PI_HRENDERBACKEND_WORLDENV hworldenv, const PI_VECTOR4* clear_rgba);
typedef void (*PI_PFN_BACKEND_RENDER_END_SCENE)(PI_HRENDERBACKEND hbackend);

typedef void (*PI_PFN_BACKEND_RENDER_SUBMIT_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned instance_count);

typedef void (*PI_PFN_BACKEND_RENDER_UPDATE_LIGHTS)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, PI_LIGHTTYPE type, unsigned update_start, unsigned update_count, const PI_LIGHT lights[], unsigned full_light_count);

typedef void (*PI_PFN_BACKEND_RENDER_UPDATE_SCENE_LIGHTPROPERTIES)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const PI_LIGHT_PROPERTIES* properties);

typedef struct PI_BACKEND_RENDER_CALLBACKS {
    PI_PFN_BACKEND_RENDER_CLOSE pfn_close;

    PI_PFN_BACKEND_RENDER_CONFIGURE pfn_configure;

    PI_PFN_BACKEND_RENDER_CREATE_WORLDENVIRONMENT pfn_create_worldenvironment;
    PI_PFN_BACKEND_RENDER_DESTROY_WORLDENVIRONMENT pfn_destroy_worldenvironment;
    PI_PFN_BACKEND_RENDER_CREATE_SCENE pfn_create_scene;
    PI_PFN_BACKEND_RENDER_DESTROY_SCENE pfn_destroy_scene;
    PI_PFN_BACKEND_RENDER_CREATE_MESH pfn_create_mesh;
    PI_PFN_BACKEND_RENDER_DESTROY_MESH pfn_destroy_mesh;
    PI_PFN_BACKEND_RENDER_CREATE_INSTBUFFER pfn_create_instbuffer;
    PI_PFN_BACKEND_RENDER_DESTROY_INSTBUFFER pfn_destroy_instbuffer;

    PI_PFN_BACKEND_RENDER_ALLOC_INSTBUFFER pfn_alloc_instbuffer;
    PI_PFN_BACKEND_RENDER_UPDATE_INSTBUFFER pfn_update_instbuffer;

    PI_PFN_BACKEND_RENDER_UPDATE_SCENE_VP pfn_update_scene_vp;
    PI_PFN_BACKEND_RENDER_UPDATE_SCENE_LIGHTPROPERTIES pfn_update_scene_lightproperties;
    PI_PFN_BACKEND_RENDER_BEGIN_SCENE pfn_begin_scene;
    PI_PFN_BACKEND_RENDER_END_SCENE pfn_end_scene;

    PI_PFN_BACKEND_RENDER_SUBMIT_MESH pfn_submit_mesh;

    PI_PFN_BACKEND_RENDER_UPDATE_LIGHTS pfn_update_lights;
} PI_BACKEND_RENDER_CALLBACKS;

#define ASSERT_BACKEND_CALLBACKS_EX(callbacks, assertfunc) \
                                        do {                                                                                          \
                                            const void* const* p = (const void* const*)&(callbacks);                                  \
                                            for (size_t i = 0; i < (sizeof(PI_BACKEND_RENDER_CALLBACKS) / sizeof(void*)); i++, p++) { \
                                                assertfunc(*p && "Backend callback check failed! Check again whether every callback is set properly in your backend.");                                                                       \
                                            }                                                                                         \
                                        } while (0)

#define ASSERT_BACKEND_CALLBACKS(callbacks) ASSERT_BACKEND_CALLBACKS_EX(callbacks, PI_ASSERT)

typedef struct PI_BACKEND_RENDER_STRUCTURES {
    size_t WORLDENV_size;
    size_t SCENE_size;
    size_t MESH_size;
    size_t INSTBUFFER_size;
} PI_BACKEND_RENDER_STRUCTURES;

// Helpers to automatically fill in the backend object size structure using the defined types
#define DEFINE_BACKEND_STRUCTURE(structures, n) (structures).n##_size = sizeof(struct PI_RENDERBACKEND_##n)
#define DEFINE_BACKEND_STRUCTURES(structures) \
            DEFINE_BACKEND_STRUCTURE(structures, WORLDENV); \
            DEFINE_BACKEND_STRUCTURE(structures, SCENE); \
            DEFINE_BACKEND_STRUCTURE(structures, MESH);  \
            DEFINE_BACKEND_STRUCTURE(structures, INSTBUFFER)

// Declare SDL_Widnow to avoid including "SDL_video.h" (shouldn't it be fine though to include it?)
struct SDL_Window;
typedef struct PI_BACKEND_RENDER_DESC {
    struct SDL_Window* sdl_window;
} PI_BACKEND_RENDER_DESC;

typedef struct PI_BACKEND_RENDER_INFO {
    PI_BACKEND_RENDER_STRUCTURES structures;
    PI_BACKEND_RENDER_CALLBACKS callbacks;
} PI_BACKEND_RENDER_INFO;

typedef size_t (*PI_PFN_BACKEND_RENDER_CALCSIZE)();
typedef bool (*PI_PFN_BACKEND_RENDER_ENTRY)(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_INFO* pinformation);
typedef struct PI_BACKEND_RENDER {
    const char* name;

    PI_PFN_BACKEND_RENDER_CALCSIZE pfn_calcsize;
    PI_PFN_BACKEND_RENDER_ENTRY pfn_entry;
} PI_BACKEND_RENDER;

//

/* Structure data for the uniform scene buffer for backends to use */
typedef struct PI_SCENEENV {
    PI_MATRIX_4X4 vpmatrix;

    PI_VECTOR3 ambient;
} PI_SCENEENV;

#endif
