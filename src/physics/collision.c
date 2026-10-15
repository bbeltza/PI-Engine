#include "physics_internal.h"
#include <piengine/pi_math.h>

static void update_aabb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar);
static void update_obb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar);

void _physics_update_collision(PI_HSYSTEMSCENE hscene, PI_HRIGIDBODY rigidbody)
{
    PI_RIGIDBODY_DATA* data = rigidbody->p_data;
    if (data->clproperties.type == PI_COLLISION_NONE) {
        return;
    }

    if (hscene->rbdata_count == 1) {
        return;
    }

    PI_RIGIDBODY_DATA* scenedata = hscene->rbdata;
    for (unsigned i = 0; i < hscene->rbdata_count; i++, scenedata++)
    {
        if (data == scenedata)
            continue;
        if (data->clproperties.type == PI_COLLISION_NONE)
            continue;

        switch (data->clproperties.type) {
            case PI_COLLISION_AABB:
                update_aabb(data, scenedata);
                break;
            case PI_COLLISION_OBB:
                update_obb(data, scenedata);
                break;
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

static bool aabb_intersect(const AABB* aabb1, AABB* aabb2, PI_VECTOR3X* p_push, PI_VECTOR3X* p_mask)
{
    PI_VECTOR3X d1 = pi_sub_4(aabb1->max, aabb2->min);
    if (is_negative3x(d1))
        return false;

    PI_VECTOR3X d2 = pi_sub_4(aabb2->max, aabb1->min);
    if (is_negative3x(d2))
        return false;

    {
        PI_VECTOR3X abspush;
        PI_VECTOR3X push = {
            select_lowest(d1.x, d2.x, &abspush.x),
            select_lowest(d1.y, d2.y, &abspush.y),
            select_lowest(d1.z, d2.z, &abspush.z)
        };

        float min = fminf(abspush.x, abspush.y);
        min = fminf(min, abspush.z);
        #if PI_MATH_USE_SSE
            register __m128 minmm = _mm_set_ps1(min);
            register __m128 pmm = _mm_cmpeq_ps(abspush.__xmm, minmm);
            *p_push = PI_INIT_STRUCT(PI_VECTOR3X){ _mm_and_ps(push.__xmm, pmm) };
        #else
            *p_push = PI_INIT_STRUCT(PI_VECTOR3X){
                push.x * (abspush.x == min),
                push.y * (abspush.y == min),
                push.z * (abspush.z == min)
            };
        #endif
            *p_mask = PI_INIT_STRUCT(PI_VECTOR3X){
                !p_push->x,
                !p_push->y,
                !p_push->z
            };
    }

    return true;
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
        pi_add_4(datal->rbproperties.position, 
                pi_mul_4(datal->rbproperties.scale, datal->clproperties.aabb.min)),
        pi_add_4(datal->rbproperties.position, 
                pi_mul_4(datal->rbproperties.scale, datal->clproperties.aabb.max))
    };
    struct PI_COLLISIONPROP_AABB worldspacer = {
        pi_add_4(datar->rbproperties.position, 
                pi_mul_4(datar->rbproperties.scale, datar->clproperties.aabb.min)),
        pi_add_4(datar->rbproperties.position, 
                pi_mul_4(datar->rbproperties.scale, datar->clproperties.aabb.max))
    };

    PI_VECTOR3X push;
    PI_VECTOR3X mask;
    if (aabb_intersect(&worldspacel, &worldspacer, &push, &mask)) {
        if (datal->p_dirtyflag) {
            *datal->p_dirtyflag |= 1;
        }
        if (!datar->rbproperties.mass) {
            datal->rbproperties.position = pi_add_4(datal->rbproperties.position, push);
            datal->rbproperties.force = pi_mul_4(datal->rbproperties.force, mask);
        }
        else {
            float ratio;
            float r;
            float l;
            if (datal->rbproperties.mass > datar->rbproperties.mass) {
                ratio = datar->rbproperties.mass / datal->rbproperties.mass;
                l = ratio*0.5f;
                r = (2.0f - ratio)*0.5f;
            }
            else {
                ratio = datal->rbproperties.mass / datar->rbproperties.mass;
                r = ratio*0.5f;
                l = (2.0f - ratio)*0.5f;
            }

            PI_VECTOR3X lpush = pi_mul_4s(push, l);
            PI_VECTOR3X rpush = pi_mul_4s(push, r);
            datal->rbproperties.position = pi_add_4(datal->rbproperties.position, lpush);
            datar->rbproperties.position = pi_sub_4(datar->rbproperties.position, rpush);
        }
    }
}

/**/

static void update_aabb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{
    switch (datar->clproperties.type) {
        case PI_COLLISION_AABB:
            aabb_vs_aabb(datal, datar);
            break;
        default:
            PI_ASSUME(0);
    }
}

static void update_obb(PI_RIGIDBODY_DATA* datal, PI_RIGIDBODY_DATA* datar)
{

}
