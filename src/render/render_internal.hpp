#ifndef PI_RENDER_INTERNAL_HPP
#define PI_RENDER_INTERNAL_HPP

#define PI_INCLUDE_BACKEND_API 1
#include <piengine/pi_render.h>
#include "../pi_internal.h"

#include <memory>

#include <vector>
#include <list>
#include <map>

template <typename T>
using PI_VECTOR = std::vector<T>;
template <typename T>
using PI_PTR = std::unique_ptr<T>;

struct PI_INSTBUFFER_IMPL
{
    PI_INSTBUFFER_IMPL(PI_SYSTEM_RENDER* _renderer) noexcept;
    ~PI_INSTBUFFER_IMPL() noexcept;

    PI_SYSTEM_RENDER* renderer = NULL;

    PI_HRENDERBACKEND_INSTBUFFER get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND_INSTBUFFER>(this + 1);
    }

    struct {
       size_t begin;
       size_t size;
    } dirty;

    size_t backend_size = 0; // Size of the buffer in the backend
                                // Inits to 0, gets updated when the capacity of the buffer (in host memory) changes during update

    std::vector<PI_INSTANCE> bufdata;
    std::list<PI_INSTANCE_IMPL> instlist;
};

template <typename T>
struct PI_INSTMAP_ALLOC_T
{
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using size_type = std::size_t;

    template< class U >
    struct rebind {
        using other = PI_INSTMAP_ALLOC_T<U>;
    };
    
    constexpr PI_INSTMAP_ALLOC_T(PI_HSCENE hscene);

    template <typename U>
    friend struct PI_INSTMAP_ALLOC_T;

    template <typename U>
    constexpr PI_INSTMAP_ALLOC_T(const PI_INSTMAP_ALLOC_T<U>& other) noexcept:
        m_backendsize(other.m_backendsize) {}
public:
    template <typename U>
    constexpr bool operator ==(const PI_INSTMAP_ALLOC_T<U>& other) {
        return this->m_backendsize == other.m_backendsize;
    }
    template <typename U>
    constexpr bool operator !=(const PI_INSTMAP_ALLOC_T<U>& other) {
        return this->m_backendsize != other.m_backendsize;
    }
public:
    pointer allocate(size_type n, const void* hint = nullptr) {
        return static_cast<pointer>(
            operator new(n * sizeof(value_type) + m_backendsize, std::nothrow)
        );
    }

    void deallocate(void* block, size_type n) {
        operator delete(block, n * sizeof(value_type) + m_backendsize);
    }
private:
    size_type m_backendsize = 0;
};

using PI_INSTMAP_ALLOC = PI_INSTMAP_ALLOC_T<
                std::pair<PI_HMESH, PI_INSTBUFFER_IMPL>
            >;
using PI_INSTMAP = std::map<
        PI_HMESH,
        PI_INSTBUFFER_IMPL,
        std::less<PI_HMESH>,
        PI_INSTMAP_ALLOC
    >;

struct PI_SCENE_IMPL
{
    PI_SCENE_IMPL(PI_SYSTEM_RENDER* _renderer);
    ~PI_SCENE_IMPL();

    PI_SYSTEM_RENDER* renderer;

    // Default values for those in the constructor's definition
    float clear_color[4];
    float projection[16];
    float view[16];

    struct {
        bool viewproj : 1;
    } dirty{ true };

    PI_HRENDERBACKEND_SCENE get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND_SCENE>(this + 1);
    }

    PI_INSTMAP::iterator begin_instbufs() noexcept {
        return m_instance_set.begin();
    }
    PI_INSTMAP::iterator end_instbufs() noexcept {
        return m_instance_set.end();
    }

public:
    PI_HINSTANCE add_scene_mesh(PI_HMESH hmesh, const PI_INSTANCE* instance) noexcept;
private:
    PI_INSTMAP m_instance_set;
};

struct PI_MESH_IMPL
{
    PI_MESH_IMPL(const PI_MESH_IMPL&) = delete;
    PI_MESH_IMPL(PI_MESH_IMPL&&) = delete;

    PI_MESH_IMPL(PI_SYSTEM_RENDER* _renderer, unsigned num_vertices, const PI_VERTEX vertices[], unsigned num_indices, const void* indices, PI_INDEX index_type) noexcept;

    ~PI_MESH_IMPL() noexcept;

    PI_SYSTEM_RENDER* renderer;

    PI_HRENDERBACKEND_MESH get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND_MESH>(this + 1);
    }
};

struct PI_INSTANCE_IMPL
{
    PI_INSTANCE_IMPL(PI_INSTBUFFER_IMPL* _instbuffer, const PI_INSTANCE* instance);
    ~PI_INSTANCE_IMPL() noexcept;

    PI_INSTBUFFER_IMPL* instbuffer;
    size_t index;

    constexpr bool operator==(const PI_INSTANCE_IMPL& other) const {
        return this == &other;
    }

public:
    bool update_instance(const PI_INSTANCE* instance) noexcept;
};

struct PI_SYSTEM_RENDER
{
    PI_SYSTEM_RENDER(PI_ENGINEDATA* hengine, bool& succeed) noexcept;
    ~PI_SYSTEM_RENDER() noexcept;

    PI_HMESH create_mesh_indexed(int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) noexcept;
    PI_HSCENE create_scene(void) noexcept;

    void destroy_mesh(PI_HMESH hmesh) noexcept;
    void release_scene(PI_HSCENE hscene) noexcept;

    PI_HSCENE set_current_scene(PI_HSCENE hscene) noexcept;
    PI_HSCENE get_current_scene() const noexcept {
        return m_currentscn;
    }
    
    PI_BACKEND_RENDER_INFO backend;
    PI_HRENDERBACKEND get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND>(this + 1);
    }

    void update(void) noexcept;
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

    PI_RENDER_CONFIGURE m_configureinfo{};

    SDL_Window* m_sdlwnd = NULL;
private:
    static bool _s_event_watch(void* userdata, SDL_Event* event) noexcept;
};

//

template <typename T>
constexpr PI_INSTMAP_ALLOC_T<T>::PI_INSTMAP_ALLOC_T(PI_HSCENE hscene):
    m_backendsize(hscene->renderer->backend.structures.INSTBUFFER_size)
{}

#endif
