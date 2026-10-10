#ifndef PI_RAII_HPP
#define PI_RAII_HPP

#include <piengine/pi_common.h>

#ifndef PI_CPP
    #error "Can only include \"pi_raii.hpp\" with a C++ compiler!"
#endif

#include <type_traits>

// RAII class for all handle types

template<typename T, void (D)(T handle)>
struct PI_RAIIH
{
    using type = T;

    static_assert(std::is_pointer<T>::value, "T must be a pointer (One of the PI_H~~ types)");
    static_assert(sizeof(T) == sizeof(void*), "sizeof(T) isn't equal to sizeof(void*)???");
    explicit constexpr PI_RAIIH(T ptr = NULL):
        m_ptr(ptr) {}

    constexpr PI_RAIIH(const PI_RAIIH&) = delete;
    inline void operator =(const PI_RAIIH&) = delete;

    inline PI_RAIIH(PI_RAIIH&& other) noexcept:
        m_ptr(other.m_ptr) {
            other.m_ptr = NULL;
        }
    inline PI_RAIIH& operator =(PI_RAIIH&& other) noexcept {
        this->reset(other.m_ptr);
        other.m_ptr = NULL;
        return *this;
    }
    inline PI_RAIIH& operator =(T ptr) noexcept {
        reset(ptr);
        return *this;
    }

    constexpr bool operator ==(const PI_RAIIH& other) const noexcept {
        return this == &other;
    }
    constexpr bool operator !=(const PI_RAIIH& other) const noexcept {
        return this != &other;
    }
    constexpr bool operator ==(T other) const noexcept {
        return m_ptr == other;
    }
    constexpr bool operator !=(T other) const noexcept {
        return m_ptr != other;
    }

    constexpr T get() noexcept { return m_ptr; }
    constexpr operator T() noexcept { return m_ptr; }
    
    constexpr operator bool() const noexcept { return m_ptr != nullptr; }

    inline void reset( T ptr = T{} ) noexcept {
        if (m_ptr && m_ptr != ptr)
            D(m_ptr);
        m_ptr = ptr;
    }
    inline T release() noexcept {
        T p = m_ptr;
        m_ptr = NULL;
        return p;
    }

    ~PI_RAIIH() noexcept {
        reset();
    }
private:
    T m_ptr;
};

#define PI_DECLARE_RAIIHEX(t, raii_t) using raii_t = PI_RAIIH<t, t##_DESTRUCTOR>
#define PI_DECLARE_RAIIH(t) PI_DECLARE_RAIIHEX(t, t##_RAII)

#endif
