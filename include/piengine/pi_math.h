#ifndef PI_MATH_H
#define PI_MATH_H

#include <piengine/pi_scalars.h>

/* Basic scalar API */

#define _PI_LERP_RIGHT(a, b, t) ( ((b) - (a)) * (t) ) /* Right operand of the lerp operation */
#define PI_LERP(a, b, t) ( (a) + _PI_LERP_RIGHT(a, b, t) )
#define PI_LERPA(a, b, t) (a) += _PI_LERP_RIGHT(a, b, t) /* Assign linear interpolation to `a` */

#define PI_CLAMP(x, min, max) x > max ? max : ( x < min ? min : x )
#define PI_CLAMPX(x, min, max) if (x > max) x = max; else if (x < min) x = min;

/* Base matrix API */

PI_INLINE_C PI_MATRIX_4X4* _pi_mul_4x4_generic(PI_MATRIX_4X4* dst, const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r)
{
    *dst = PI_INIT_STRUCT(PI_MATRIX_4X4){
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
    return dst;
}


#if PI_MATH_USE_SSE

#define _MM_SHUF_XCOL 0x00
#define _MM_SHUF_YCOL 0x55
#define _MM_SHUF_ZCOL 0xAA
#define _MM_SHUF_WCOL 0xFF

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
    shuf = _mm_shuffle_ps(rxrow, rxrow, _MM_SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf); 
    
    shuf = _mm_shuffle_ps(rxrow, rxrow, _MM_SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rxrow, rxrow, _MM_SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rxrow, rxrow, _MM_SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->xx, vres0);

    
    shuf = _mm_shuffle_ps(ryrow, ryrow, _MM_SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(ryrow, ryrow, _MM_SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(ryrow, ryrow, _MM_SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(ryrow, ryrow, _MM_SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->yx, vres0);


    shuf = _mm_shuffle_ps(rzrow, rzrow, _MM_SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(rzrow, rzrow, _MM_SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rzrow, rzrow, _MM_SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rzrow, rzrow, _MM_SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->zx, vres0);


    shuf = _mm_shuffle_ps(rwrow, rwrow, _MM_SHUF_XCOL);
    vres0 = _mm_mul_ps(lxrow, shuf);

    shuf = _mm_shuffle_ps(rwrow, rwrow, _MM_SHUF_YCOL);
    vres1 = _mm_mul_ps(lyrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rwrow, rwrow, _MM_SHUF_ZCOL);
    vres1 = _mm_mul_ps(lzrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);

    shuf = _mm_shuffle_ps(rwrow, rwrow, _MM_SHUF_WCOL);
    vres1 = _mm_mul_ps(lwrow, shuf);
    vres0 = _mm_add_ps(vres0, vres1);
    _mm_store_ps(&dst->wx, vres0);
    return dst;
}

#endif

#if PI_MATH_USE_AVX

#define _MM_PERMUTE_LO1 0x00
#define _MM_PERMUTE_HI1 0x11
#define _MM_PERMUTE_LO2 0x22
#define _MM_PERMUTE_HI2 0x33

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

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, _MM_PERMUTE_LO1);
    shuf1 = _mm256_permute_ps(rxyrow, _MM_SHUF_XCOL);
    shuf2 = _mm256_permute_ps(rzwrow, _MM_SHUF_XCOL);
    res1 = _mm256_mul_ps(duprow, shuf1);
    res2 =_mm256_mul_ps(duprow, shuf2);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, _MM_PERMUTE_HI1);
    shuf1 = _mm256_permute_ps(rxyrow, _MM_SHUF_YCOL);
    shuf2 = _mm256_permute_ps(rzwrow, _MM_SHUF_YCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, _MM_PERMUTE_LO2);
    shuf1 = _mm256_permute_ps(rxyrow, _MM_SHUF_ZCOL);
    shuf2 = _mm256_permute_ps(rzwrow, _MM_SHUF_ZCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    duprow = _mm256_permute2f128_ps(lxyrow, lzwrow, _MM_PERMUTE_HI2);
    shuf1 = _mm256_permute_ps(rxyrow, _MM_SHUF_WCOL);
    shuf2 = _mm256_permute_ps(rzwrow, _MM_SHUF_WCOL);
    mult = _mm256_mul_ps(duprow, shuf1);
    res1 = _mm256_add_ps(res1, mult);
    mult = _mm256_mul_ps(duprow, shuf2);
    res2 = _mm256_add_ps(res2, mult);

    _mm256_store_ps(&dst->xx, res1);
    _mm256_store_ps(&dst->zx, res2);
    return dst;
}

#endif

PI_INLINE_C PI_MATRIX_4X4* pi_mul_4x4(PI_MATRIX_4X4* dst, const PI_MATRIX_4X4* l, const PI_MATRIX_4X4* r) {
    #if PI_MATH_USE_AVX
        return _pi_mul_4x4_avx(dst, l, r);
    #endif
    #if PI_MATH_USE_SSE
        return _pi_mul_4x4_sse(dst, l, r);
    #endif
    return _pi_mul_4x4_generic(dst, l, r);
}

//


PI_CONSTEXPR_F PI_VECTOR4 _pi_mulv_4x4_generic(PI_VECTOR4 v, const PI_MATRIX_4X4* m) {
    return PI_INIT_STRUCT(PI_VECTOR4){
        v.x*m->xx + v.y*m->yx + v.z*m->zx + v.w*m->wx,
        v.x*m->xy + v.y*m->yy + v.z*m->zy + v.w*m->wy,
        v.x*m->xz + v.y*m->yz + v.z*m->zz + v.w*m->wz,
        v.x*m->xw + v.y*m->yw + v.z*m->zw + v.w*m->ww
    };
}

#if PI_MATH_USE_SSE

PI_INLINE_C PI_VECTOR4 _pi_mulv_4x4_sse(PI_VECTOR4 v, const PI_MATRIX_4X4* m)
{
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

    shuf = _mm_shuffle_ps(v.__xmm, v.__xmm, _MM_SHUF_XCOL);
    res = _mm_mul_ps(shuf, colx);
    colx = res;

    shuf = _mm_shuffle_ps(v.__xmm, v.__xmm, _MM_SHUF_YCOL);
    res = _mm_mul_ps(shuf, coly);
    colx = _mm_add_ps(colx, res);

    shuf = _mm_shuffle_ps(v.__xmm, v.__xmm, _MM_SHUF_ZCOL);
    res = _mm_mul_ps(shuf, colz);
    colx = _mm_add_ps(colx, res);
    
    shuf = _mm_shuffle_ps(v.__xmm, v.__xmm, _MM_SHUF_WCOL);
    res = _mm_mul_ps(shuf, colw);
    colx = _mm_add_ps(colx, res);

    return PI_INIT_STRUCT(PI_VECTOR4){ colx };
}

#endif

PI_INLINE_C PI_VECTOR4 pi_mulv_4x4(PI_VECTOR4 v, const PI_MATRIX_4X4* m) {
    #if PI_MATH_USE_SSE
        return _pi_mulv_4x4_sse(v, m);
    #endif
    return _pi_mulv_4x4_generic(v, m);
}


#include <math.h>

/* Vector math API */

PI_INLINE_C PI_VECTOR4* _pi_vector_negative_generic(PI_VECTOR4* dst, PI_VECTOR4 v) {
    *dst = PI_INIT_STRUCT(PI_VECTOR4){
        -v.x,
        -v.y,
        -v.z,
        -v.w
    };
    return dst;
}

#if PI_MATH_USE_SSE

PI_INLINE_C PI_VECTOR4* _pi_vector_negative_sse(PI_VECTOR4* dst, PI_VECTOR4 v)
{
    /* This usually should be faster than what most compilers do:
        *  Load a constant from memory and perform a xor operation to invert the sign of the floats
        *  This one is just 4-5 instruction, it clears one of the vector register, and then it subtracts that register by the source vector, giving it's negative value.
        *  It doesn't involve any memory operations other than storing the result at the end (to `dst`) */

    register __m128 res = _mm_xor_ps(v.__xmm, v.__xmm);
    res = _mm_sub_ps(res, v.__xmm);

    _mm_store_ps(&dst->x, res);
    return dst;
}

#endif

    /* Give -v, stored in `dst` */
PI_INLINE_C PI_VECTOR4* pi_vector_negative4(PI_VECTOR4* dst, PI_VECTOR4 v)
{
    #if PI_MATH_USE_SSE
        return _pi_vector_negative_sse(dst, v);
    #endif
    return _pi_vector_negative_generic(dst, v);
}


PI_INLINE_C PI_VECTOR4* _pi_vector_normalize_generic(PI_VECTOR4* dst, PI_VECTOR4 v)
{
    float invmag = 1.0f/sqrtf( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w );
    *dst = PI_INIT_STRUCT(PI_VECTOR4){
        v.x*invmag,
        v.y*invmag,
        v.z*invmag,
        v.w*invmag
    };
    return dst;
}

#if PI_MATH_USE_SSE

PI_INLINE_C PI_VECTOR4* _pi_vector_normalize_sse(PI_VECTOR4* dst, PI_VECTOR4 v)
{
    register __m128 msq = _mm_mul_ps(v.__xmm, v.__xmm);
    register __m128 shu = _mm_shuffle_ps(msq, msq, _MM_SHUFFLE(2, 0, 0, 1));
    msq = _mm_add_ps(msq, shu);
    shu = _mm_shuffle_ps(msq, msq, _MM_SHUFFLE(0, 0, 0, 3));
    msq = _mm_add_ss(msq, shu);
    msq = _mm_rsqrt_ss(msq);
    msq = _mm_shuffle_ps(msq, msq, _MM_SHUFFLE(0, 0, 0, 0));
    msq = _mm_mul_ps(msq, v.__xmm);

    _mm_store_ps(&dst->x, msq);
    return dst;
}

#endif

PI_INLINE_C PI_VECTOR4* pi_vector_normalize4(PI_VECTOR4* dst, PI_VECTOR4 v)
{
    #if PI_MATH_USE_SSE
        return _pi_vector_normalize_sse(dst, v);
    #endif
    return _pi_vector_normalize_generic(dst, v);
}

//

#if PI_MATH_USE_SSE
#define _MM_SHUF_CROSS1 _MM_SHUFFLE(3, 0, 2, 1)
#define _MM_SHUF_CROSS2 _MM_SHUFFLE(3, 1, 0, 2)

PI_INLINE_C PI_VECTOR4* _pi_vector_cross_sse(PI_VECTOR4* dst, PI_VECTOR4 v1, PI_VECTOR4 v2)
{
    register __m128 res;
    register __m128 lmm = _mm_shuffle_ps(v1.__xmm, v1.__xmm, _MM_SHUF_CROSS1);
    register __m128 rmm = _mm_shuffle_ps(v2.__xmm, v2.__xmm, _MM_SHUF_CROSS2);
    res = _mm_mul_ps(lmm, rmm);
    lmm = _mm_shuffle_ps(v1.__xmm, v1.__xmm, _MM_SHUF_CROSS2);
    rmm = _mm_shuffle_ps(v2.__xmm, v2.__xmm, _MM_SHUF_CROSS1);
    lmm = _mm_mul_ps(lmm, rmm);
    res = _mm_sub_ps(res, lmm);
    
    _mm_store_ps(&dst->x, res);
    return dst;
}

#endif

PI_INLINE_C PI_VECTOR4* _pi_vector_cross_generic(PI_VECTOR4* dst, PI_VECTOR4 v1, PI_VECTOR4 v2)
{
    *dst = PI_INIT_STRUCT(PI_VECTOR4){
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x
    };
    return dst;
}

    /* Calculate the 3D cross product of two vectors, and store it inside a 4D vector `dst`. The "w" component of the vector is ignored and will be zeroed. (or undefined???) */
PI_INLINE_C PI_VECTOR4* pi_vector_cross4(PI_VECTOR4* dst, PI_VECTOR4 v1, PI_VECTOR4 v2)
{
    #if PI_MATH_USE_AVX
        #error "Implement AVX cross product???"
        return _pi_vector_cross_avx(dst, v1, v2);
    #endif
    #if PI_MATH_USE_SSE
        return _pi_vector_cross_sse(dst, v1, v2);
    #endif
    return _pi_vector_cross_generic(dst, v1, v2);
}

PI_CONSTEXPR_F float pi_vector_dot4(PI_VECTOR4 v1, PI_VECTOR4 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
}

PI_CONSTEXPR_F float pi_vector_dot3(PI_VECTOR3 v1, PI_VECTOR3 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

PI_CONSTEXPR_F float pi_vector_dot2(PI_VECTOR2 v1, PI_VECTOR2 v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

/* Matrix math API */

PI_CONSTEXPR_F PI_MATRIX_4X4 pi_identity_4x4() {
    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

PI_CONSTEXPR_F PI_MATRIX_4X4 pi_scale_4x4(float x, float y, float z, float w /* = 1 */) {
    return PI_INIT_STRUCT(PI_MATRIX_4X4){
        x   , 0.0f, 0.0f, 0.0f,
        0.0f, y   , 0.0f, 0.0f,
        0.0f, 0.0f, z   , 0.0f,
        0.0f, 0.0f, 0.0f, w
    };
}

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

PI_INLINE_C PI_MATRIX_4X4* pi_matrix_rotate_x(PI_MATRIX_4X4* dst, float rx)
{
    float sx = sinf(rx);
    float cx = cosf(rx);

    *dst = PI_INIT_STRUCT(PI_MATRIX_4X4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cx  , sx  , 0.0f,
        0.0f, -sx , cx  , 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return dst;
}

PI_INLINE_C PI_MATRIX_4X4* pi_matrix_rotate_y(PI_MATRIX_4X4* dst, float ry)
{
    float sy = sinf(ry);
    float cy = cosf(ry);

    *dst = PI_INIT_STRUCT(PI_MATRIX_4X4){
        cy  , 0.0f, -sy , 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sy  , 0.0f, cy  , 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return dst;
}

PI_INLINE_C PI_MATRIX_4X4* pi_matrix_rotate_z(PI_MATRIX_4X4* dst, float rz)
{
    float sz = -sinf(rz); // Clockwise rotation instead of standard counter-clockwise rotation. Usually would work better for computer graphics rotations, but might change it.
    float cz = cosf(rz);

    *dst = PI_INIT_STRUCT(PI_MATRIX_4X4){
        cz  , sz , 0.0f, 0.0f,
        -sz , cz  , 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return dst;
}


    /* Rotation with normalized `n_axis`, DON'T use it unless you're sure that n_axis is normalized. Otherwise, use `pi_matrix_rotate4` */
PI_INLINE_C PI_MATRIX_4X4* pi_matrix_rotate4n(PI_MATRIX_4X4* dst, float angle, PI_VECTOR3 n_axis)
{
    float c = cosf(angle);
    float s = sinf(angle);

    float c1 = 1.0f - c;
    float xc1 = n_axis.x*c1;
    float yc1 = n_axis.y*c1;
    float zc1 = n_axis.z*c1;
    float xs = n_axis.x*s;
    float ys = n_axis.y*s;
    float zs = n_axis.z*s;
    *dst = PI_INIT_STRUCT(PI_MATRIX_4X4){
        n_axis.x*xc1 + c, n_axis.x*yc1 + zs, n_axis.x*zc1 - ys, 0.0f,
        n_axis.y*xc1 - zs, n_axis.y*yc1 + c, n_axis.y*zc1 + xs, 0.0f,
        n_axis.z*xc1 + ys, n_axis.z*yc1 - xs, n_axis.z*zc1 + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return dst;
}

PI_INLINE_C PI_MATRIX_4X4* pi_matrix_rotate4(PI_MATRIX_4X4* dst, float angle, PI_VECTOR3 axis)
{
    PI_VECTOR4 n_axis = { axis.x, axis.y, axis.z };
    pi_vector_normalize4(&n_axis, n_axis);
    return pi_matrix_rotate4n(dst, angle, *(PI_VECTOR3*)&n_axis);
}


    /* View matrices */

PI_INLINE_C PI_MATRIX_4X4* pi_matrix_view(PI_MATRIX_4X4* dst, PI_VECTOR4 eye, PI_VECTOR4 xaxis, PI_VECTOR4 yaxis, PI_VECTOR4 zaxis)
{
    pi_vector_negative4(&eye, eye);
    pi_vector_negative4(&xaxis, xaxis);
    pi_vector_negative4(&yaxis, yaxis);

    PI_MATRIX_4X4 _dst = {
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,

        0.0f, 0.0f, 0.0f, 1.0f
    };
    eye.w = 1.0f;
    *(PI_VECTOR4*)&_dst.wx = pi_mulv_4x4(eye, &_dst);

    *dst = _dst;
    return dst;
}

PI_INLINE_C PI_MATRIX_4X4* pi_look_along(PI_MATRIX_4X4* dst, PI_VECTOR4 eye, PI_VECTOR4 look, PI_VECTOR4 up)
{
    PI_VECTOR4 right;
    pi_vector_cross4(&right, look, up);
    pi_vector_normalize4(&right, right);
    pi_vector_cross4(&up, look, right);
    return pi_matrix_view(dst, eye, right, up, look);
}

PI_INLINE_C PI_MATRIX_4X4* pi_look_at(PI_MATRIX_4X4* dst, PI_VECTOR4 eye, PI_VECTOR4 at, PI_VECTOR4 up)
{
    PI_VECTOR4 look = { at.x - eye.x, at.y - eye.y, at.z - eye.z };
    pi_vector_normalize4(&look, look);
    return pi_look_along(dst, eye, look, up);
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
