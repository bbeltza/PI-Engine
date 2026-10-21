#ifndef PI_MATH_HPP
#define PI_MATH_HPP

#include <piengine/pi_math.h>

#ifndef PI_CPP
    #error "Can't include .hpp header. File must be compiled with a C++ compiler."
#endif

namespace pi
{
    struct VECTOR4
    {
        VECTOR4() = default;
        constexpr VECTOR4(float x, float y, float z, float w):
            _v{ x, y, z, w } {}
        constexpr VECTOR4(float s):
            _v{ s, s, s, s } {}
        constexpr VECTOR4(const PI_VECTOR3& v, float w=0.0f):
            VECTOR4(v.x, v.y, v.z, w) {}
        constexpr VECTOR4(const PI_VECTOR4& v):
            _v{ v } {}
    public:
        constexpr PI_VECTOR4& ref() { return _v; }
        constexpr const PI_VECTOR4& ref() const { return _v; }

        constexpr PI_VECTOR4* ptr() { return &_v; }
        constexpr const PI_VECTOR4* ptr() const { return &_v; }

        inline const PI_VECTOR3& to_vec3() const {
            return reinterpret_cast<const PI_VECTOR3&>(_v);
        }

        constexpr bool zero() const {
            return !_v.x && !_v.y && !_v.z && !_v.w;
        }
    public:
        constexpr operator PI_VECTOR4&() { return _v; }
        constexpr operator const PI_VECTOR4&() const { return _v; }

        constexpr PI_VECTOR4* operator ->() { return &_v; }
        constexpr const PI_VECTOR4* operator ->() const { return &_v; }

        constexpr PI_VECTOR4* operator &() { return &_v; }
        constexpr const PI_VECTOR4* operator &() const { return &_v; }

        constexpr operator bool() const { return !zero(); }
    public:
        constexpr float& operator [](size_t i) {
            return _v.arr[i];
        }
        constexpr const float& operator [](size_t i) const {
            return _v.arr[i];
        }
    private:
        PI_VECTOR4 _v;
    };

    struct MATRIX_4X4 {
        MATRIX_4X4() = default;
        constexpr MATRIX_4X4(
                float xx, float xy, float xz, float xw,
                float yx, float yy, float yz, float yw,
                float zx, float zy, float zz, float zw,
                float wx, float wy, float wz, float ww
            ):
            _m{ {
                xx, xy, xz, xw,
                yx, yy, yz, yw,
                zx, zy, zz, zw,
                wx, wy, wz, ww
                }
            } {}
        constexpr MATRIX_4X4(
                const PI_VECTOR4& vx,
                const PI_VECTOR4& vy,
                const PI_VECTOR4& vz,
                const PI_VECTOR4& vw=PI_VECTOR4{}
                ):
           MATRIX_4X4{
                vx.x, vx.y, vx.z, vx.w,
                vy.x, vy.y, vy.z, vy.w,
                vz.x, vz.y, vz.z, vz.w,
                vw.x, vw.y, vw.z, vw.w
           } {}
        constexpr MATRIX_4X4(const PI_MATRIX_4X4& m):
            _m{ m } {}
    public:
        constexpr PI_MATRIX_4X4& ref() { return _m; }
        constexpr const PI_MATRIX_4X4& ref() const { return _m; }

        constexpr PI_MATRIX_4X4* ptr() { return &_m; }
        constexpr const PI_MATRIX_4X4* ptr() const { return &_m; }

        inline PI_MATRIX_3X3 to_mat3x3() const {
            return PI_MATRIX_3X3{ {
                    _m.xx, _m.xy, _m.xz,
                    _m.yx, _m.yy, _m.yz,
                    _m.zx, _m.zy, _m.zz
                }
            };
        }
    public:
        constexpr operator PI_MATRIX_4X4&() { return _m; }
        constexpr operator const PI_MATRIX_4X4&() const { return _m; }

        constexpr PI_MATRIX_4X4* operator ->() { return &_m; }
        constexpr const PI_MATRIX_4X4* operator ->() const { return &_m; }

        constexpr PI_MATRIX_4X4* operator &() { return &_m; }
        constexpr const PI_MATRIX_4X4* operator &() const { return &_m; }
    public:
        template <typename Fn, typename... Args>
        inline PI_MATRIX_4X4* operator()(Fn&& fn, Args&&... args) {
            return fn(&_m, std::forward<Args>(args)...);
        }


        constexpr auto operator[](size_t i) -> decltype(ref().m[i]) {
            return _m.m[i];
        }
        constexpr auto operator[](size_t i) const -> decltype(ref().m[i]) {
            return _m.m[i];
        }

        constexpr float& operator()(size_t i) { return _m.arr[i]; }
        constexpr const float& operator()(size_t i) const { return _m.arr[i]; }
        constexpr float& operator()(size_t i, size_t j) { return _m.m[i][j]; }
        constexpr const float& operator()(size_t i, size_t j) const { return _m.m[i][j]; }
    private:
        PI_MATRIX_4X4 _m;
    };
}

/* Operator overloaders */

inline PI_VECTOR4 operator +(const PI_VECTOR4& v1, const PI_VECTOR4& v2) {
    return pi_add_4(v1, v2);
}
inline PI_VECTOR4 operator +(const PI_VECTOR4& v, float s) {
    return pi_add_4s(v, s);
}
inline PI_VECTOR4 operator +(float s, const PI_VECTOR4& v) {
    return pi_add_4s(v, s);
}

inline PI_VECTOR4 operator -(const PI_VECTOR4& v1, const PI_VECTOR4& v2) {
    return pi_sub_4(v1, v2);
}
inline PI_VECTOR4 operator -(const PI_VECTOR4& v, float s) {
    return pi_sub_4s(v, s);
}
inline PI_VECTOR4 operator -(float s, const PI_VECTOR4& v) {
    return pi_sub_4si(s, v);
}

inline PI_VECTOR4 operator *(const PI_VECTOR4& v1, const PI_VECTOR4& v2) {
    return pi_mul_4(v1, v2);
}
inline PI_VECTOR4 operator *(const PI_VECTOR4& v, float s) {
    return pi_mul_4s(v, s);
}
inline PI_VECTOR4 operator *(float s, const PI_VECTOR4& v) {
    return pi_mul_4s(v, s);
}

inline PI_VECTOR4 operator -(const PI_VECTOR4& v) {
    return pi_unary_4(v);
}


inline PI_VECTOR4& operator +=(PI_VECTOR4& v, const PI_VECTOR4& v2) {
    v = pi_add_4(v, v2);
    return v;
}
inline PI_VECTOR4& operator +=(PI_VECTOR4& v, float s) {
    v = pi_add_4s(v, s);
    return v;
}

inline PI_VECTOR4& operator -=(PI_VECTOR4& v, const PI_VECTOR4& v2) {
    v = pi_sub_4(v, v2);
    return v;
}
inline PI_VECTOR4& operator -=(PI_VECTOR4& v, float s) {
    v = pi_sub_4s(v, s);
    return v;
}

inline PI_VECTOR4& operator *=(PI_VECTOR4& v, const PI_VECTOR4& v2) {
    v = pi_mul_4(v, v2);
    return v;
}
inline PI_VECTOR4& operator *=(PI_VECTOR4& v, float s) {
    v = pi_mul_4s(v, s);
    return v;
}


inline PI_MATRIX_4X4 operator *(const PI_MATRIX_4X4& m1, const PI_MATRIX_4X4& m2) {
    PI_MATRIX_4X4 dst;
    pi_mul_4x4(&dst, &m1, &m2);
    return dst;
}
inline PI_VECTOR4 operator *(const PI_VECTOR4& v, const PI_MATRIX_4X4& m) {
    return pi_mulv_4x4(v, &m);
}
inline PI_MATRIX_4X4 operator *(const PI_MATRIX_4X4& m, float s) {
    PI_MATRIX_4X4 dst;
    pi_mul_4x4s(&dst, &m, s);
    return dst;
}
inline PI_MATRIX_4X4 operator *(float s, const PI_MATRIX_4X4& m) {
    PI_MATRIX_4X4 dst;
    pi_mul_4x4s(&dst, &m, s);
    return dst;
}

inline PI_MATRIX_4X4& operator *=(PI_MATRIX_4X4& m, const PI_MATRIX_4X4& m2) {
    return *pi_mul_4x4(&m, &m, &m2);
}
inline PI_VECTOR4& operator *=(PI_VECTOR4& v, const PI_MATRIX_4X4& m) {
    v = pi_mulv_4x4(v, &m);
    return v;
}
inline PI_MATRIX_4X4& operator *=(PI_MATRIX_4X4& m, float s) {
    return *pi_mul_4x4s(&m, &m, s);
}

//

#endif
