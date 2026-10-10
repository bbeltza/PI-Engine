#ifndef PI_SCALARS_H
#define PI_SCALARS_H

#ifndef PI_MATH_USE_GENERIC
    #if defined(__AVX512F__)
        #error "Need to implement math library with AVX512 intrinsics!"
        #define PI_MATH_USE_AVX512F 1
    #endif
    #if defined(__AVX__)
        #define PI_MATH_USE_AVX 1
    #endif
   #if defined(__SSE4_1__) || (_M_IX86 >= 2)
        #define PI_MATH_USE_SSE4_1 1
    #endif
    #if defined(__SSE__) || defined(_M_X64) || (defined (_M_IX86) && defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
        #define PI_MATH_USE_SSE 1
    #endif
    #if defined(__ARM_NEON) || defined(_M_ARM_64)
        #error "Need to implement math library with NEON intrinsics!"
        #define PI_MATH_USE_NEON 1
    #endif
#endif

#if PI_MATH_USE_AVX512F || PI_MATH_USE_AVX || PI_MATH_USE_SSE
    #include <immintrin.h>
#elif PI_MATH_USE_NEON
    #include <arm_neon.h>
#endif

#include <piengine/pi_common.h>

#ifdef PI_CPP
    #define PI_INIT_STRUCT(T) T
#else
    #define PI_INIT_STRUCT(T) (T)
#endif

/* Vectors */

typedef union PI_VECTOR4 {
    #if PI_MATH_USE_SSE
        __m128 __xmm;
    #endif
    struct {
        float x;
        float y;
        float z;
        float w;
    };
    float arr[4];
} PI_VECTOR4;

typedef union PI_VECTOR3 {
    struct {
        float x;
        float y;
        float z;
    };
    float arr[3];
} PI_VECTOR3;

typedef union PI_VECTOR2 {
    #if PI_MATH_USE_NEON
        float32x2_t __f32x2
    #endif
    struct {
        float x;
        float y;
    };
    float arr[2];
} PI_VECTOR2;

/* An actual VECTOR4, that is meant to be used as a 3 component vector, instead of a 4 component one, and is SIMD/SSE friendly. */
typedef PI_VECTOR4 PI_VECTOR3X;

/* Matrices */

typedef union PI_MATRIX_4X4 {
    struct {
        float xx, xy, xz, xw;
        float yx, yy, yz, yw;
        float zx, zy, zz, zw;
        float wx, wy, wz, ww;
    };
    struct {
        PI_VECTOR4 x;
        PI_VECTOR4 y;
        PI_VECTOR4 z;
        PI_VECTOR4 w;
    } v;

    float arr[4*4];
    float m[4][4];
    #if PI_MATH_USE_SSE
        __m128 __xmm[4];
    #endif
    #if PI_MATH_USE_AVX
        __m256 __ymm[2];
    #endif
} PI_MATRIX_4X4;

typedef union PI_MATRIX_3X3 {
    struct {
        float xx, xy, xz;
        float yx, yy, yz;
        float zx, zy, zz;
    };
    struct {
        PI_VECTOR3 x;
        PI_VECTOR3 y;
        PI_VECTOR3 z;
    } v;
    float arr[3*3];
    float m[3][3];
} PI_MATRIX_3X3;

typedef union PI_MATRIX_2X2 {
    struct {
        float xx, xy;
        float yx, yy;
    };
    struct {
        PI_VECTOR2 x;
        PI_VECTOR2 y;
    } v;
    float arr[2*2];
    float m[2][2];
    #if PI_MATH_USE_SSE
        __m128 __xmm[1];
    #endif
} PI_MATRIX_2X2;


#endif
