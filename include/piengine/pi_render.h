#ifndef PI_RENDER_H
#define PI_RENDER_H

#include <piengine/pi_common.h>

typedef struct PI_SCENE_IMPL *PI_HSCENE;
typedef struct PI_MESH_IMPL *PI_HMESH;
typedef struct PI_INSTANCE_IMPL *PI_HINSTANCE;

typedef struct PI_VERTEX {
    float x, y, z;
    float r, g, b;
} PI_VERTEX;

typedef struct PI_INSTANCE {
    float m[16];
    float r, g, b, _a;
} PI_INSTANCE;

typedef enum PI_INDEX {
    PI_INDEX_UNKNOWN = 0,
    PI_INDEX_U16 = 1,
    PI_INDEX_U32 = 2
} PI_INDEX;

PI_EXTERN_C PI_HMESH pi_create_mesh_indexed(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type);
PI_EXTERN_C PI_HMESH pi_create_mesh(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[]);
PI_EXTERN_C void pi_destroy_mesh(PI_HMESH hmesh);

PI_EXTERN_C PI_HSCENE pi_create_scene(PI_HGAME hgame);
PI_EXTERN_C void pi_release_scene(PI_HSCENE hscene);

PI_EXTERN_C PI_HSCENE pi_set_current_scene(PI_HGAME hgame, PI_HSCENE hscene);
PI_EXTERN_C PI_HSCENE pi_get_current_scene(PI_HGAME hgame);

PI_EXTERN_C bool pi_set_scene_clear_color(PI_HSCENE hscene, float r, float g, float b, float a);
PI_EXTERN_C bool pi_set_scene_projection(PI_HSCENE hscene, const float mproj[16]);
PI_EXTERN_C bool pi_set_scene_view(PI_HSCENE hscene, const float mview[16]);

PI_EXTERN_C PI_HINSTANCE pi_add_scene_mesh(PI_HSCENE hscene, PI_HMESH hmesh, const PI_INSTANCE* instance);
PI_EXTERN_C void pi_remove_scene_mesh(PI_HINSTANCE hmeshinst);
PI_EXTERN_C bool pi_update_mesh_instance(PI_HINSTANCE hmeshinst, const PI_INSTANCE* instance);

#endif

/* Include backend part! */

#if defined(PI_INCLUDE_BACKEND_API) && !defined(PI_RENDER_BACKEND_H)
#define PI_RENDER_BACKEND_H

typedef struct PI_RENDERBACKEND *PI_HRENDERBACKEND;
typedef struct PI_RENDERBACKEND_MESH *PI_HRENDERBACKEND_MESH;
typedef struct PI_RENDERBACKEND_INSTANCE *PI_HRENDERBACKEND_INSTBUFFER;

typedef void (*PI_PFN_BACKEND_RENDER_CLOSE)(PI_HRENDERBACKEND hbackend);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, const PI_VERTEX* vertices, unsigned num_vertices, const void* indices, unsigned num_indices, PI_INDEX index_type);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh);

typedef bool (*PI_PFN_BACKEND_RENDER_CREATE_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned initial_instance_count);
typedef void (*PI_PFN_BACKEND_RENDER_DESTROY_INSTBUFFER)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer);

typedef void (*PI_PFN_BACKEND_RENDER_UPDATE_SCENE_VP)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const float vpmatrix[16]);
typedef void (*PI_PFN_BACKEND_RENDER_BEGIN_SCENE)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_SCENE hscene, const float clear_rgba[4]);
typedef void (*PI_PFN_BACKEND_RENDER_END_SCENE)(PI_HRENDERBACKEND hbackend);

typedef void (*PI_PFN_BACKEND_RENDER_SUBMIT_MESH)(PI_HRENDERBACKEND hbackend, PI_HRENDERBACKEND_MESH hmesh, PI_HRENDERBACKEND_INSTBUFFER hinstbuffer, unsigned instance_count);

typedef struct PI_BACKEND_RENDER_CALLBACKS {
    PI_PFN_BACKEND_RENDER_CLOSE pfn_close;

    PI_PFN_BACKEND_RENDER_CREATE_SCENE pfn_create_scene;
    PI_PFN_BACKEND_RENDER_DESTROY_SCENE pfn_destroy_scene;
} PI_BACKEND_RENDER_CALLBACKS;

// Declare SDL_Widnow to avoid including "SDL_video.h" (shouldn't it be fine though to include it?)
struct SDL_Window;
typedef struct PI_BACKEND_RENDER_DESC {
    struct SDL_Window* sdl_window;
} PI_BACKEND_RENDER_DESC;

typedef bool (*PI_PFN_BACKEND_RENDER_ENTRY)(PI_HRENDERBACKEND hbackend, const PI_BACKEND_RENDER_DESC* pdesc, PI_BACKEND_RENDER_CALLBACKS* pcallbacks);
typedef struct PI_BACKEND_RENDER {
    const char* name;
    size_t structsize;

    PI_PFN_BACKEND_RENDER_ENTRY pfn_entry;
} PI_BACKEND_RENDER;

#endif
