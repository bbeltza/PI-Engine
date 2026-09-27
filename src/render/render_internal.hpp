#ifndef PI_RENDER_INTERNAL_HPP
#define PI_RENDER_INTERNAL_HPP

#include <piengine/pi_render.h>
#include "../pi_internal.h"

#include <vector>
#include <memory>

template <typename T>
using PI_VECTOR = std::vector<T>;
template <typename T>
using PI_PTR = std::unique_ptr<T>;


struct PI_SCENE_IMPL
{
    PI_SCENE_IMPL(PI_SYSTEM_RENDER* _renderer);

    PI_SYSTEM_RENDER* renderer;

    float clear_color[4];
    float projection[16];
    float view[16];

    struct {
        bool viewproj : 1;
    } updates{ true };

public:
    PI_HINSTANCE add_scene_mesh(PI_HMESH hmesh, const PI_INSTANCE* instance) noexcept;
    void remove_scene_mesh(PI_HINSTANCE hmesh);
};

struct PI_MESH_IMPL
{
    PI_MESH_IMPL(PI_SYSTEM_RENDER* _renderer, unsigned num_vertices, const PI_VERTEX* vertices[], unsigned num_indices, const void* indices, PI_INDEX index_type);

    PI_SYSTEM_RENDER* renderer;
};

struct PI_INSTANCE_IMPL
{
    PI_INSTANCE_IMPL(PI_HSCENE _hscene, PI_HMESH _hmesh, size_t index, const PI_INSTANCE* instance);

    PI_HSCENE hscene;
    PI_HMESH hmesh;
    size_t index;

public:
    bool update_instance(const PI_INSTANCE* instance) noexcept;
};

struct PI_SYSTEM_RENDER
{
    PI_SYSTEM_RENDER(PI_ENGINEDATA* hengine);

    PI_HMESH create_mesh_indexed(int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) noexcept;
    PI_HSCENE create_scene(void) noexcept;

    void destroy_mesh(PI_HMESH hmesh) noexcept;
    void release_scene(PI_HSCENE hscene) noexcept;

    PI_HSCENE set_current_scene(PI_HSCENE hscene) noexcept;
    PI_HSCENE get_current_scene() const noexcept {
        return m_currentscn;
    }
private:
    PI_HSCENE m_currentscn = NULL;

    struct {
        PI_VECTOR<PI_PTR<PI_SCENE_IMPL>> scenes;
        PI_VECTOR<PI_PTR<PI_MESH_IMPL>> meshes;
    } m_objects;

    struct {
        PI_VECTOR<size_t> scene;
        PI_VECTOR<size_t> mesh;
    } m_freelists;
};

#endif
