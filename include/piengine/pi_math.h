#ifndef PI_MATH_H
#define PI_MATH_H

#include <piengine/pi_common.h>

#ifdef PI_CPP
    #define PI_INIT_STRUCT(T) T
    #define PI_ALIGN_STRUCT(n) typedef struct PI_ALIGN_AS(n)
#else
    #define PI_INIT_STRUCT(T) (T)
    #define PI_ALIGN_STRUCT(n) PI_ALIGN_AS(n) typedef struct
#endif

/* Base matrix API */

PI_ALIGN_STRUCT(16) PI_MATRIX_4X4 {
     float xx, xy, xz, xw;
     float yx, yy, yz, yw;
     float zx, zy, zz, zw;
     float wx, wy, wz, ww;
} PI_MATRIX_4X4;

typedef struct PI_MATRIX_3X3 {
     float xx, xy, xz;
     float yx, yy, yz;
     float zx, zy, zz;
} PI_MATRIX_3X3;

PI_ALIGN_STRUCT(16) PI_MATRIX_2X2 {
     float xx, xy;
     float yx, yy;
} PI_MATRIX_2X2;

    /* Query a pointer to a `float` pointing to the first element of a `PI_MATRIX_nXn` */

#define PI_MFLOATP(m) (&(m).xx)
#define PI_MPFLOATP(m) (&(m)->xx)

PI_CONSTEXPR_F       PI_MATRIX_4X4* PI_FPTOMATRIX_4X4 (float m[4*4])       { return (PI_MATRIX_4X4*)m; }
PI_CONSTEXPR_F const PI_MATRIX_4X4* PI_FPTOCMATRIX_4X4(const float m[4*4]) { return (const PI_MATRIX_4X4*)m; }
PI_CONSTEXPR_F       PI_MATRIX_3X3* PI_FPTOMATRIX_3X3 (float m[3*3])       { return (PI_MATRIX_3X3*)m; }
PI_CONSTEXPR_F const PI_MATRIX_3X3* PI_FPTOCMATRIX_3X3(const float m[3*3]) { return (const PI_MATRIX_3X3*)m; }
PI_CONSTEXPR_F       PI_MATRIX_2X2* PI_FPTOMATRIX_2X2 (float m[2*2])       { return (PI_MATRIX_2X2*)m; }
PI_CONSTEXPR_F const PI_MATRIX_2X2* PI_FPTOCMATRIX_2X2(const float m[2*2]) { return (const PI_MATRIX_2X2*)m; }



PI_INLINE_C PI_MATRIX_4X4 _pi_mul_4x4_generic(const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r)
{
    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        l->xx*r->xx + l->yx*r->xy + l->zx*r->xz + l->wx*r->xw,
        l->xy*r->xx + l->yy*r->xy + l->zy*r->xz + l->wy*r->xw,
        l->xz*r->xx + l->yz*r->xy + l->zz*r->xz + l->wz*r->xw,
        l->xw*r->xx + l->yw*r->xy + l->zw*r->xz + l->ww*r->xw,

        l->xx*r->yx + l->yx*r->yy + l->zx*r->yz + l->wx*r->yw,
        l->xy*r->yx + l->yy*r->yy + l->zy*r->yz + l->wy*r->yw,
        l->xz*r->yx + l->yz*r->yy + l->zz*r->yz + l->wz*r->yw,
        l->xw*r->yx + l->yw*r->yy + l->zw*r->yz + l->ww*r->yw,

        l->xx*r->zx + l->yx*r->zy + l->zx*r->zz + l->wx*r->zw,
        l->xy*r->zx + l->yy*r->zy + l->zy*r->zz + l->wy*r->zw,
        l->xz*r->zx + l->yz*r->zy + l->zz*r->zz + l->wz*r->zw,
        l->xw*r->zx + l->yw*r->zy + l->zw*r->zz + l->ww*r->zw,

        l->xx*r->wx + l->yx*r->wy + l->zx*r->wz + l->wx*r->ww,
        l->xy*r->wx + l->yy*r->wy + l->zy*r->wz + l->wy*r->ww,
        l->xz*r->wx + l->yz*r->wy + l->zz*r->wz + l->wz*r->ww,
        l->xw*r->wx + l->yw*r->wy + l->zw*r->wz + l->ww*r->ww
    };
}


#define SHUF_XCOL 0x00
#define SHUF_YCOL 0x55
#define SHUF_ZCOL 0xAA
#define SHUF_WCOL 0xFF

#include <immintrin.h>
PI_INLINE_C PI_MATRIX_4X4* _pi_mul_4x4_sse(PI_MATRIX_4X4* dst, const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r)
{

    register __m128 lxrow = _mm_load_ps(&l->xx);
    register __m128 lyrow = _mm_load_ps(&l->yx);
    register __m128 lzrow = _mm_load_ps(&l->zx);
    register __m128 lwrow = _mm_load_ps(&l->wx);
    register __m128 rxrow = _mm_load_ps(&r->xx);
    register __m128 ryrow = _mm_load_ps(&r->yx);
    register __m128 rzrow = _mm_load_ps(&r->zx);
    register __m128 rwrow = _mm_load_ps(&r->wx);

    register __m128 shuf;
    register __m128 vres1;
    register __m128 vres0;
    shuf = _mm_shuffle_ps(rxrow, rxrow, SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf); 
    
    shuf = _mm_shuffle_ps(rxrow, rxrow, SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rxrow, rxrow, SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rxrow, rxrow, SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->xx, vres0);

    
    shuf = _mm_shuffle_ps(ryrow, ryrow, SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(ryrow, ryrow, SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(ryrow, ryrow, SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(ryrow, ryrow, SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->yx, vres0);


    shuf = _mm_shuffle_ps(rzrow, rzrow, SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(rzrow, rzrow, SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rzrow, rzrow, SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rzrow, rzrow, SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->zx, vres0);


    shuf = _mm_shuffle_ps(rwrow, rwrow, SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(rwrow, rwrow, SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rwrow, rwrow, SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rwrow, rwrow, SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->wx, vres0);
    return dst;
}

#define PERMUTE_LO1 0x00
#define PERMUTE_HI1 0x11
#define PERMUTE_LO2 0x22
#define PERMUTE_HI2 0x33

PI_INLINE_C PI_MATRIX_4X4* _pi_mul_4x4_avx(PI_MATRIX_4X4* dst, const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r)
{
    register __m256 lxyrow = _mm256_load_ps(&l->xx);
    register __m256 lzwrow = _mm256_load_ps(&l->zx);
    register __m256 rxyrow = _mm256_load_ps(&r->xx);
    register __m256 rzwrow = _mm256_load_ps(&r->zx);

    register __m256 duprow;
    register __m256 shuf1;
    register __m256 shuf2;
   
    register __m256 res1;
    register __m256 res2;

    register __m256 mult;

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, PERMUTE_LO1);
    shuf1 = _mm256_permute_ps(rxyrow, SHUF_XCOL);
    shuf2 = _mm256_permute_ps(rzwrow, SHUF_XCOL);
    res1 = _mm256_mul_ps(duprow, shuf1);
    res2 =_mm256_mul_ps(duprow, shuf2);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, PERMUTE_HI1);
    shuf1 = _mm256_permute_ps(rxyrow, SHUF_YCOL);
    shuf2 = _mm256_permute_ps(rzwrow, SHUF_YCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, PERMUTE_LO2);
    shuf1 = _mm256_permute_ps(rxyrow, SHUF_ZCOL);
    shuf2 = _mm256_permute_ps(rzwrow, SHUF_ZCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, PERMUTE_HI2);
    shuf1 = _mm256_permute_ps(rxyrow, SHUF_WCOL);
    shuf2 = _mm256_permute_ps(rzwrow, SHUF_WCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    _mm256_store_ps(&dst->xx, res1);
    _mm256_store_ps(&dst->zx, res2);
    return dst;
}

PI_INLINE_C PI_MATRIX_4X4 pi_mul_4x4(const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r) {

    //return _pi_mul_4x4_generic(l, r);
    PI_MATRIX_4X4 dst;
    //return *_pi_mul_4x4_sse(&dst, l, r);
    return *_pi_mul_4x4_avx(&dst, l, r);
}

//

PI_ALIGN_STRUCT(16) PI_VECTOR4 {
    float x;
    float y;
    float z;
    float w;
} PI_VECTOR4;

typedef struct PI_VECTOR3 {
    float x;
    float y;
    float z;
} PI_VECTOR3;

typedef struct PI_VECTOR2 {
    float x;
    float y;
} PI_VECTOR2;


PI_CONSTEXPR_F PI_VECTOR4 _pi_mulv_4x4_generic(const PI_VECTOR4* v, const PI_MATRIX_4X4* m) {
    return PI_INIT_STRUCT(PI_VECTOR4){
        v->x*m->xx + v->y*m->yx + v->z*m->zx + v->w*m->wx,
        v->x*m->xy + v->y*m->yy + v->z*m->zy + v->w*m->wy,
        v->x*m->xz + v->y*m->yz + v->z*m->zz + v->w*m->wz,
        v->x*m->xw + v->y*m->yw + v->z*m->zw + v->w*m->ww
    };
}

PI_INLINE_C PI_VECTOR4* _pi_mulv_4x4_sse(PI_VECTOR4* dst, const PI_VECTOR4* v, const PI_MATRIX_4X4* m)
{
    register __m128 mvec = _mm_load_ps(&v->x);
    register __m128 colx;
    register __m128 coly;
    register __m128 colz;
    register __m128 colw;
    register __m128 shuf;
    register __m128 res;
    
    colx = _mm_load_ps(&m->xx);
    coly = _mm_load_ps(&m->yx);
    colz = _mm_load_ps(&m->zx);
    colw = _mm_load_ps(&m->wx);

    shuf = _mm_shuffle_ps(mvec, mvec, SHUF_XCOL);
    res = _mm_mul_ps(shuf, colx);
    colx = res;

    shuf = _mm_shuffle_ps(mvec, mvec, SHUF_YCOL);
    res = _mm_mul_ps(shuf, coly);
    colx = _mm_add_ps(colx, res);

    shuf = _mm_shuffle_ps(mvec, mvec, SHUF_ZCOL);
    res = _mm_mul_ps(shuf, colz);
    colx = _mm_add_ps(colx, res);
    
    shuf = _mm_shuffle_ps(mvec, mvec, SHUF_WCOL);
    res = _mm_mul_ps(shuf, colw);
    colx = _mm_add_ps(colx, res);

    _mm_store_ps(&dst->x, colx);
    return dst;
}

PI_INLINE_C PI_VECTOR4 pi_mulv_4x4(const PI_VECTOR4* v, const PI_MATRIX_4X4* m) {
    return _pi_mulv_4x4_generic(v, m);
    PI_VECTOR4 dst;
    return *_pi_mulv_4x4_sse(&dst, v, m);
}

/* High Level Matrix API */

#include <math.h>

PI_CONSTEXPR_F PI_MATRIX_4X4 pi_matrix_ortho(
        float left,  float right,
        float top,   float bottom,
        float znear, float zfar
    ) {
    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        2.0f/(right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/(top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f/(zfar - znear), 0.0f,
        -(right + left)/(right - left), (bottom + top)/(bottom - top), (zfar + znear)/(zfar - znear), 1.0f
    };    
}

PI_INLINE_C PI_MATRIX_4X4 pi_matrix_perspective(float fov, float ratio, float znear, float zfar)
{
    float tanfov2 = 1.0f/tanf(fov/2.0f);
    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        tanfov2 / ratio, 0.0f, 0.0f, 0.0f,
        0.0f, tanfov2, 0.0f, 0.0f,
        0.0f, 0.0f, (-zfar - znear)/(znear - zfar), 1.0f,
        0.0f, 0.0f, (2*znear*zfar)/(znear - zfar), 0.0f
    };
}

    /* Rotations */

PI_INLINE_C PI_MATRIX_4X4 pi_matrix_rotate_x(float rx, float ry, float rz)
{
    float sx = sinf(rx);
    float cx = cosf(rx);

    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cx  , sx  , 0.0f,
        0.0f, -sx , cx  , 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

PI_INLINE_C PI_MATRIX_4X4 pi_matrix_rotate_y(float rx, float ry, float rz)
{
    float sy = sinf(ry);
    float cy = cosf(ry);

    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        cy  , 0.0f, -sy , 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sy  , 0.0f, cy  , 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}



PI_INLINE_C PI_MATRIX_4X4 pi_matrix_rotate_xyz(float rx, float ry, float rz)
{
    float sy = sinf(ry);
    float cy = cosf(ry);

    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        cy, 0.0f, -sy, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sy, 0.0f, cy, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

/* Some helper definitions */

#define PI_M_PI       3.14159265358979323846
#define PI_M_PI1_180  0.01745329251994329500
#define PI_M_SQRT2    1.41421356237309504880
#define PI_M_SQRT1_2  1.41421356237309504880

#define PI_DEG(n) ((n) * PI_M_PI1_180)
#define PI_DEGF(n) ( (float)PI_DEG(n) )

#ifdef PI_CPP

constexpr long double operator ""_deg(long double n)  { return PI_DEG(n); }
constexpr float       operator ""_degf(long double n) { return static_cast<float>(PI_DEG(n)); }
constexpr long double operator ""_deg(unsigned long long n) { return PI_DEG(n); }
constexpr float       operator ""_degf(unsigned long long n) { return static_cast<float>(PI_DEG(n)); }

#endif

#endif
