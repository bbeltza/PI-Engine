#ifndef PI_RENDER_INTERNAL_HPP
#define PI_RENDER_INTERNAL_HPP

#define PI_INCLUDE_BACKEND_API 1
#include <piengine/pi_render.h>
#include "../pi_internal.h"

#include <memory>

#include <vector>
#include <list>
#include <map>

/* Helper to remove an element from a container if their address is the same. This avoids having to declare an `operator ==()` in the T class */
/* `T::operator &()` CAN be overloaded, it'll be fine as long as the return type of the overload can be compared using `operator ==()` */
template <typename T>
inline void REMOVE_CMP_PTR(T& l, typename T::const_reference v) {
    l.remove_if([&](typename T::const_reference other) { return &v == &other; });
}

template <typename T>
struct PI_BACKEND_ALLOC
{
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using size_type = std::size_t;
    
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_copy_assignment = std::true_type;

    template< class U >
    struct rebind {
        using other = PI_BACKEND_ALLOC<U>;
    };
    
    constexpr PI_BACKEND_ALLOC(size_t backend_size):
        m_backendsize(backend_size) {}

    template <typename U>
    friend struct PI_BACKEND_ALLOC;

    template <typename U>
    constexpr PI_BACKEND_ALLOC(const PI_BACKEND_ALLOC<U>& other) noexcept:
        m_backendsize(other.m_backendsize) {}

    constexpr size_type backend_size() const noexcept {
        return m_backendsize;
    }
public:
    template <typename U>
    constexpr bool operator ==(const PI_BACKEND_ALLOC<U>& other) {
        return this->m_backendsize == other.m_backendsize;
    }
    template <typename U>
    constexpr bool operator !=(const PI_BACKEND_ALLOC<U>& other) {
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


template <typename T>
using PI_VECTOR = std::vector<T>;
template <typename T, typename Allocator=std::allocator<T>>
using PI_LIST = std::list<T, Allocator>;
template <typename K, typename T, typename Allocator = std::allocator<std::pair<const K, T>>>
using PI_MAP = std::map<K, T, std::less<K>, Allocator>;

template <typename T>
using PI_BACKEND_LIST = std::list<T, PI_BACKEND_ALLOC<T>>;

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

    PI_VECTOR<PI_INSTANCE> bufdata;
    PI_LIST<PI_INSTANCE_IMPL> instlist;
};

using PI_INSTMAP_ALLOC = PI_BACKEND_ALLOC<std::pair<PI_HMESH, PI_INSTBUFFER_IMPL>>;
using PI_INSTMAP = PI_MAP<
        PI_HMESH,
        PI_INSTBUFFER_IMPL,
        PI_INSTMAP_ALLOC
    >;

struct PI_LIGHT_IMPL
{
    PI_LIGHT_IMPL(PI_HSYSTEMSCENE hscene, PI_LIGHTTYPE type, unsigned index) noexcept;
    ~PI_LIGHT_IMPL() noexcept;
    
    PI_HSYSTEMSCENE hscene;
    PI_LIGHTTYPE type = PI_LIGHT_NONE;
    unsigned index;
};

struct PI_WORLDENV_IMPL
{
    static constexpr unsigned REF_MASK = 0x80000000;

    PI_WORLDENV_IMPL(PI_SYSTEM_RENDER* _renderer, const PI_WORLDENV_DESC* pdesc) noexcept;
    ~PI_WORLDENV_IMPL() noexcept;

    PI_SYSTEM_RENDER* renderer;
    PI_WORLDENV_EFFECTS effects;

    PI_HRENDERBACKEND_WORLDENV get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND_WORLDENV>(this + 1);
    }
public:
    constexpr unsigned ref() const { return m_refcount & REF_MASK; }
    inline unsigned acquire() noexcept {
        return (m_refcount++) & REF_MASK;
    }
    inline unsigned release() noexcept {
        unsigned r = ref();
        if (!(--m_refcount))
            destroy();
        return r;
    }

    inline bool detach_from_user() noexcept {
        m_refcount &= ~REF_MASK;
        if (!m_refcount) {
            destroy();
            return true;
        }
        return false;
    }
private:
    void destroy() noexcept;

    unsigned m_refcount = REF_MASK; // Reference count, currently unused. World environments will be internally reference counted!!! (You can only create and release them once, but the object won't be internally destroyed until all of the acquired scenes are destroyed. Hence the "internally")
                                 // Bit REF_MASK (0x80000000) is set upon creation when the world environment hasn't been released using `pi_release_worldenvironment`
};

struct PI_SYSTEM_SCENE
{
    PI_SYSTEM_SCENE(PI_SYSTEM_RENDER* _renderer, PI_HWORLDENV _hworldenv);
    ~PI_SYSTEM_SCENE();

    PI_SYSTEM_RENDER* renderer;
    PI_WORLDENV_IMPL* worldenv;

    // Default values for those in the constructor's definition
    PI_MATRIX_4X4 projection;
    PI_MATRIX_4X4 view;
    PI_VECTOR4 clear_color;

    PI_LIGHT_PROPERTIES lightprops;

    struct {
        bool viewproj : 1;
        bool lightprops : 1;
    } dirty{ true, true };

    PI_HRENDERBACKEND_SCENE get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND_SCENE>(this + 1);
    }

    bool update() noexcept;
public:
    PI_HINSTANCE add_scene_mesh(PI_HMESH hmesh, const PI_INSTANCE* instance) noexcept;

    PI_HLIGHT add_light(PI_LIGHTTYPE type, const PI_LIGHT* initial_parameters) noexcept;
    void remove_light(PI_HLIGHT hlight) noexcept;
    bool update_light(PI_HLIGHT hlight, const PI_LIGHT* light) noexcept;
private:
    PI_INSTMAP m_instance_set;

    PI_LIST<PI_LIGHT_IMPL> m_lights;
    PI_VECTOR<PI_DIRECTIONAL_LIGHT> m_dirlight_buffer;
    PI_VECTOR<PI_POINT_LIGHT> m_ptlight_buffer;
    PI_VECTOR<PI_LIGHT> m_lights_dirty;

    unsigned m_dirlight_start_dirty = 0;
    unsigned m_dirlight_count_dirty = 0;
    unsigned m_ptlight_start_dirty = 0;
    unsigned m_ptlight_count_dirty = 0;
    
    bool _update_dirty() noexcept;
    bool _update_instances() noexcept;
    bool _update_lights() noexcept;
    bool _submit_scene() noexcept;
};

struct PI_MESH_IMPL
{
    PI_MESH_IMPL(const PI_MESH_IMPL&) = delete;
    // PI_MESH_IMPL(PI_MESH_IMPL&&) = delete;

    PI_MESH_IMPL(PI_SYSTEM_RENDER* _renderer, unsigned num_vertices, const PI_VERTEX vertices[], unsigned num_indices, const void* indices, PI_INDEX index_type) noexcept;

    // Need to define those operators for std::list to be valid
    
    void operator =(const PI_MESH_IMPL&) {
        PI_ASSERT(0);
    }
    PI_MESH_IMPL(PI_MESH_IMPL&&) {
        PI_ASSERT(0);
    }

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

    bool init_scene(PI_HSYSTEMSCENE hscene, PI_HWORLDENV hworldenv) noexcept;
    //void deinit_scene(PI_HSYSTEMSCENE hscene) noexcept;

    PI_HWORLDENV create_worldenv(const PI_WORLDENV_DESC* pdesc) noexcept;
    PI_HMESH create_mesh_indexed(int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) noexcept;

    void remove_worldenv(PI_HWORLDENV hworldenv) noexcept;
    void destroy_mesh(PI_HMESH hmesh) noexcept;
    
    PI_BACKEND_RENDER_INFO backend;
    PI_HRENDERBACKEND get_backend() noexcept {
        return reinterpret_cast<PI_HRENDERBACKEND>(this + 1);
    }

    void update(PI_HSYSTEMSCENE hsysscenes[], unsigned scenecount) noexcept;
private:
    using WORLDENV_LIST = PI_BACKEND_LIST<PI_WORLDENV_IMPL>;
    using MESH_LIST = PI_BACKEND_LIST<PI_MESH_IMPL>;
    using WORLDENV_ALLOC = WORLDENV_LIST::allocator_type;
    using MESH_ALLOC = MESH_LIST::allocator_type;
    struct {
        WORLDENV_LIST worldenvs{ WORLDENV_ALLOC{ 0 } };
        MESH_LIST meshes{ MESH_ALLOC{ 0 } };
    } m_objects;

    PI_RENDER_CONFIGURE m_configureinfo{ };

    SDL_Window* m_sdlwnd = NULL;
private:
    static bool _s_event_hook(void* userdata, PI_EVENT* event) noexcept;
};

#endif
