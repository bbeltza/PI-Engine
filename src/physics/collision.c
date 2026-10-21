#include "physics_internal.h"
#include <piengine/pi_math.h>

#include <limits.h>

static void update_aabb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar);
static void update_sphere(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar);
//static void update_obb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar);

void _physics_update_collision(PI_HSYSTEMSCENE hscene, PI_HRIGIDBODY rigidbody)
{
    PI_RIGIDBODY_DATA* data = rigidbody->p_data;
    if (data->clproperties.type == PI_COLLISION_NONE) {
        return;
    }

    if (hscene->rbdata_count == 1) {
        return;
    }
    
    PI_ASSERT((hscene->rbdata_count + CHAR_BIT-1)/CHAR_BIT <= data->clmask_capacity);

    size_t _datai = data - hscene->rbdata;
    PI_ASSERT(_datai < hscene->rbdata_count);
    unsigned datai = (unsigned)_datai;
    unsigned bitl = datai % CHAR_BIT;
    unsigned maskil = datai / CHAR_BIT;
    unsigned cmpmaskl = 0x1 << bitl;

    PI_RIGIDBODY_DATA* scenedata = hscene->rbdata;
    for (unsigned i = 0; i < hscene->rbdata_count; i++, scenedata++)
    {
        if (data == scenedata)
            continue;
        if (scenedata->clproperties.type == PI_COLLISION_NONE)
            continue;

        unsigned bitr = i % CHAR_BIT;
        unsigned maskir = i / CHAR_BIT;
        unsigned cmpmaskr = 0x1 << bitr;
        PI_ASSERT((hscene->rbdata_count + CHAR_BIT-1)/CHAR_BIT <= scenedata->clmask_capacity);
        if (scenedata->clmask[maskil] & cmpmaskl) {
            PI_ASSERT(data->clmask[maskir] & cmpmaskr);
            continue;
        }

        scenedata->clmask[maskil] |= cmpmaskl;
        data->clmask[maskir] |= cmpmaskr;

        switch (data->clproperties.type) {
            case PI_COLLISION_AABB:
                update_aabb(data, scenedata);
                break;
            case PI_COLLISION_SPHERE:
                update_sphere(data, scenedata);
                break;
            //case PI_COLLISION_OBB:
            //    update_obb(data, scenedata);
            //    break;
            default:
                PI_ASSUME(0);
        }
    }
}

/**/

#if PI_MATH_USE_SSE4_1 // This will use SSE 4.1
static bool is_negative3x(PI_VECTOR3X v) {
    register __m128i xmask = _mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    register __m128 zeromm = _mm_setzero_ps();
    __m128 m = _mm_cmplt_ps(v.__xmm, zeromm);
    return _mm_test_all_zeros(*(__m128i*)&m, xmask) == 0;
}
#else
static bool is_negative3x(PI_VECTOR3X v) {
    return v.x < 0.0f || v.y < 0.0f || v.z < 0.0f;
}
#endif

static float select_lowest(float v1, float v2, float* retabs) {
    if (v1 < v2) {
        *retabs = v1;
        return -v1;
    }
    else {
        *retabs = v2;
        return v2;
    }
};

/**/

typedef struct PI_COLLISIONPROP_AABB AABB;

static bool aabb_intersect(const AABB* aabb1, AABB* aabb2, float* p_push, PI_VECTOR3X* p_normal)
{
    PI_VECTOR3X d1 = pi_vec4_sub(aabb1->max, aabb2->min);
    if (is_negative3x(d1))
        return false;

    PI_VECTOR3X d2 = pi_vec4_sub(aabb2->max, aabb1->min);
    if (is_negative3x(d2))
        return false;

    {
        PI_VECTOR3X abspush;
        PI_VECTOR3X push = {
            select_lowest(d1.x, d2.x, &abspush.x),
            select_lowest(d1.y, d2.y, &abspush.y),
            select_lowest(d1.z, d2.z, &abspush.z)
        };
        PI_VECTOR3X signvec = {
                (signbit(push.x) ? -1.0f : 1.0f),
                (signbit(push.y) ? -1.0f : 1.0f),
                (signbit(push.z) ? -1.0f : 1.0f)
        };
        float min = PI_MIN(abspush.x, abspush.y);
        min = PI_MIN(min, abspush.z);
        *p_push = min;

        #if PI_MATH_USE_SSE
            register __m128 minmm = _mm_set_ps1(min);
            register __m128 pmm = _mm_cmpeq_ps(abspush.__xmm, minmm);
            *p_normal = PI_INIT_STRUCT(PI_VECTOR3X){ _mm_and_ps(
                    signvec.__xmm,
                    pmm
                ) };
        #else
            *p_push = PI_INIT_STRUCT(PI_VECTOR3X){
                push.x * (abspush.x == min),
                push.y * (abspush.y == min),
                push.z * (abspush.z == min)
            };
            *p_normal = PI_INIT_STRUCT(PI_VECTOR3X){
                signvec.x * (abspush.x == min),
                signvec.y * (abspush.y == min),
                signvec.z * (abspush.z == min)
            };
        #endif
    }

    return true;
}

    /* Get weight distribution between two masses */
static void get_weight_distro(float lmass, float rmass, float* lratio, float* rratio)
{
    PI_ASSERT(lmass || rmass);

    if (!lmass) {
        *lratio = 0.0f;
        *rratio = 1.0f;
        return;
    }
    if (!rmass) {
        *lratio = 1.0f;
        *rratio = 0.0f;
        return;
    }


    float ratio;
    if (lmass > rmass) {
        ratio = rmass / lmass;
        *lratio = ratio*0.5f;
        *rratio = (2.0f - ratio)*0.5f;
    }
    else {
        ratio = lmass / rmass;
        *rratio = ratio*0.5f;
        *lratio = (2.0f - ratio)*0.5f;
    }
}

/**/

static void push_back_bodies(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar, float pushdist, PI_VECTOR3X lrnormal, PI_VECTOR3X rlnormal)
{
    float l, r;
    get_weight_distro(datal->rbproperties.mass, datar->rbproperties.mass, &l, &r);

    PI_VECTOR3X lfac = pi_vec4_muls(lrnormal, l);
    PI_VECTOR3X rfac = pi_vec4_muls(rlnormal, r);
    PI_VECTOR3X lpush = pi_vec4_muls(lfac, pushdist);
    PI_VECTOR3X rpush = pi_vec4_muls(rfac, pushdist);
    
    datal->rbproperties.position = pi_vec4_add(datal->rbproperties.position, lpush);
    datar->rbproperties.position = pi_vec4_add(datar->rbproperties.position, rpush);

    float lforcemag = pi_vec4_length(datal->rbproperties.force);
    float rforcemag = pi_vec4_length(datar->rbproperties.force);
    
    if (lforcemag) {
        float normalproj;
        PI_VECTOR3X newforce = pi_vec4_reflect(datal->rbproperties.force, lrnormal, &normalproj);
        newforce = pi_vec4_muls(newforce, 1/lforcemag);

        /*
         * -1: Cancel all of the force
         *  0: Keep all of the force
         *  1: Give part of the force
         *  */
        float newforcemag = lforcemag + (lforcemag*PI_CLAMP(normalproj, -1.0f, 0.0f));
        newforce = pi_vec4_muls(newforce, newforcemag);

        datal->rbproperties.force = newforce;
    }
    if (rforcemag) {
        float normalproj;
        PI_VECTOR3X newforce = pi_vec4_reflect(datar->rbproperties.force, rlnormal, &normalproj);
        newforce = pi_vec4_muls(newforce, 1/rforcemag);
        float newforcemag = rforcemag + (rforcemag*PI_CLAMP(normalproj, -1.0f, 0.0f));
        newforce = pi_vec4_muls(newforce, newforcemag);

        datar->rbproperties.force = newforce;
    }

}


/**/

static void aabb_vs_aabb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    PI_ASSERT(datal->clproperties.type == PI_COLLISION_AABB);
    PI_ASSERT(datar->clproperties.type == PI_COLLISION_AABB);

    // TODO: This is a set of 4 4D vectors, aka a 4x4 matrix!
        // Incorporate matrix addition (finally) and use it here instead of
        // adding the vectors individually, allowing better raw optimizations.
    struct PI_COLLISIONPROP_AABB worldspacel = {
        pi_vec4_add(datal->rbproperties.position, 
                pi_vec4_mul(datal->rbproperties.scale, datal->clproperties.aabb.min)),
        pi_vec4_add(datal->rbproperties.position, 
                pi_vec4_mul(datal->rbproperties.scale, datal->clproperties.aabb.max))
    };
    struct PI_COLLISIONPROP_AABB worldspacer = {
        pi_vec4_add(datar->rbproperties.position, 
                pi_vec4_mul(datar->rbproperties.scale, datar->clproperties.aabb.min)),
        pi_vec4_add(datar->rbproperties.position, 
                pi_vec4_mul(datar->rbproperties.scale, datar->clproperties.aabb.max))
    };

    float push;
    PI_VECTOR3X normal;
    if (aabb_intersect(&worldspacel, &worldspacer, &push, &normal)) {
        if (datal->p_dirtyflag) {
            *datal->p_dirtyflag |= 1;
        }

        push_back_bodies(datal, datar, push, normal, pi_vec4_unary(normal));
    }
}

static void aabb_vs_sphere(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    PI_ASSERT(datal->clproperties.type == PI_COLLISION_AABB);
    PI_ASSERT(datar->clproperties.type == PI_COLLISION_SPHERE);

    PI_VECTOR3X min = pi_vec4_mul(datal->clproperties.aabb.min, datal->rbproperties.scale);
    PI_VECTOR3X max = pi_vec4_mul(datal->clproperties.aabb.max, datal->rbproperties.scale);

    PI_VECTOR3X minl = pi_vec4_add(datal->rbproperties.position, min);
    PI_VECTOR3X maxl = pi_vec4_add(datal->rbproperties.position, max);
    PI_VECTOR3X posr = pi_vec4_add(datar->rbproperties.position, datar->clproperties.sphere.origin);
    
    PI_VECTOR3X distmin = pi_vec4_sub(posr, minl);
    PI_VECTOR3X distmax = pi_vec4_sub(posr, maxl);
    PI_VECTOR3X distminsq = pi_vec4_mul(distmin, distmin);
    PI_VECTOR3X distmaxsq = pi_vec4_mul(distmax, distmax);

    float radius = datar->clproperties.sphere.radius;
    float dmin = radius*radius;
    for (int i = 0; i < 3; i++) {
        if (posr.arr[i] < minl.arr[i])
            dmin -=  (distminsq.arr[i]);
        else if (posr.arr[i] > maxl.arr[i])
            dmin -= distmaxsq.arr[i];
    }
    if (dmin > 0.0f)
    {
        PI_VECTOR3X halfl = pi_vec4_muls(pi_vec4_sub(maxl, minl), 0.5f);
        PI_VECTOR3X posl = pi_vec4_add(minl, halfl);
        PI_VECTOR3X dist = pi_vec4_sub(posr, posl);
        dist.w = 0.0f;

        PI_VECTOR3X dist_normalized = pi_vec4_normalize(dist);
        PI_VECTOR3X dirmask = dist_normalized;
        PI_VECTOR3X dirabs = {
            fabsf(dist_normalized.x/halfl.x),
            fabsf(dist_normalized.y/halfl.y),
            fabsf(dist_normalized.z/halfl.z),
            fabsf(dist_normalized.w),
        };
        float max = PI_MAX(dirabs.x, dirabs.y);
        max = PI_MAX(max, dirabs.z);
        if (max != dirabs.x) {
            dirmask.x = 0.0f;
        }
        if (max != dirabs.y) {
            dirmask.y = 0.0f;
        }
        if (max != dirabs.z) {
            dirmask.z = 0.0f;
        }

        push_back_bodies(datal, datar, dmin, pi_vec4_unary(dist_normalized), dirmask);
    }
}


static void sphere_vs_sphere(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    PI_ASSERT(datal->clproperties.type == PI_COLLISION_SPHERE);
    PI_ASSERT(datar->clproperties.type == PI_COLLISION_SPHERE);

    PI_VECTOR3X originl = pi_vec4_add(datal->rbproperties.position, datal->clproperties.sphere.origin);
    PI_VECTOR3X originr = pi_vec4_add(datar->rbproperties.position, datar->clproperties.sphere.origin);

    PI_VECTOR3X dist = pi_vec4_sub(originl, originr);
    dist.w = 0.0f;
    float radius_sums = datal->clproperties.sphere.radius + datar->clproperties.sphere.radius;
    float len = pi_vec4_length(dist);
    float lendif = radius_sums - len;
    if (lendif > 0) {
        PI_VECTOR3X dist_normalized = pi_vec4_normalize(dist);
        push_back_bodies(datal, datar, lendif, dist_normalized, pi_vec4_unary(dist_normalized));
    }
}

/**/

static void update_aabb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    switch (datar->clproperties.type) {
        case PI_COLLISION_AABB:
            aabb_vs_aabb(datal, datar);
            break;
        case PI_COLLISION_SPHERE:
            aabb_vs_sphere(datal, datar);
            break;
        default:
            PI_ASSUME(0);
    }
}

static void update_sphere(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    switch (datar->clproperties.type) {
        case PI_COLLISION_AABB:
            aabb_vs_sphere(datar, datal);
            break;
        case PI_COLLISION_SPHERE:
            sphere_vs_sphere(datal, datar);
            break;
        default:
            PI_ASSUME(0);
    }
}
