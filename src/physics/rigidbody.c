#include "physics_internal.h"

static PI_RIGIDBODY_DATA* append_rigidbody_data(PI_HSYSTEMSCENE hsysscene);

PI_HRIGIDBODY _physics_add_rigid_body(PI_HSYSTEMSCENE hsysscene)
{
    struct PI_RIGIDBODY_IMPL* rigidbody = PI_ALLOC(sizeof(struct PI_RIGIDBODY_IMPL), 0);
    if (!rigidbody) {
        return NULL;
    }

    PI_RIGIDBODY_DATA* rbdata = append_rigidbody_data(hsysscene);
    if (!rbdata) {
        PI_FREE(rigidbody);
        return NULL;
    }

    rigidbody->hscene = hsysscene;
    rigidbody->p_data = rbdata;
    rigidbody->pnext = hsysscene->rbhead;
    rigidbody->pprev = NULL;
    if (rigidbody->pnext) {
        PI_ASSERT(rigidbody->pnext->pprev == NULL);
        rigidbody->pnext->pprev = rigidbody;
    }
    hsysscene->rbhead = rigidbody;
    return rigidbody;
}

void pi_remove_rigid_body(PI_HRIGIDBODY hrigidbody)
{
    if (!hrigidbody)
        return;

    unsigned curindex = (unsigned)(hrigidbody->p_data - hrigidbody->hscene->rbdata);
    PI_ASSERT(curindex < hrigidbody->hscene->rbdata_count);
    /* We want to add `curindex` to the freelist. */
    unsigned pos = hrigidbody->hscene->rbfreelist_first - 1;
    if (pos >= hrigidbody->hscene->rbfreelist_last) {
        unsigned newcount = hrigidbody->hscene->rbfreelist_capacity;
        if (hrigidbody->hscene->rbfreelist_last == hrigidbody->hscene->rbfreelist_capacity) {
            newcount += 1 + hrigidbody->hscene->rbfreelist_capacity/2;
        }
        else if (hrigidbody->hscene->rbfreelist_last == 0) {  /* Freelist is empty. */
            PI_ASSERT(hrigidbody->hscene->rbfreelist_first == 0);
            pos = newcount - 1;
            goto UPDATE_FREELIST;
        }
        else {
            PI_ASSERT(hrigidbody->hscene->rbfreelist_last < hrigidbody->hscene->rbfreelist_capacity);
            newcount += hrigidbody->hscene->rbfreelist_capacity - hrigidbody->hscene->rbfreelist_last;
        }

        unsigned* newfreelist = PI_ALLOC(newcount * sizeof(unsigned), 0);
        unsigned oldcount = hrigidbody->hscene->rbfreelist_last - hrigidbody->hscene->rbfreelist_first;
        memcpy(newfreelist + newcount - oldcount,
               hrigidbody->hscene->rbfreelist + hrigidbody->hscene->rbfreelist_first,
               oldcount
            );
        PI_FREE(hrigidbody->hscene->rbfreelist);

        pos = newcount - oldcount - 1;
        PI_ASSERT(pos < newcount);
        hrigidbody->hscene->rbfreelist = newfreelist;
    UPDATE_FREELIST:
        hrigidbody->hscene->rbfreelist_capacity = newcount;
        hrigidbody->hscene->rbfreelist_last = newcount;
    }
    hrigidbody->hscene->rbfreelist[pos] = curindex;
    hrigidbody->hscene->rbfreelist_first = pos;
    
    if (hrigidbody->pnext) {
        hrigidbody->pnext->pprev = hrigidbody->pprev;
    }
    if (hrigidbody->pprev) {
        hrigidbody->pprev->pnext = hrigidbody->pnext;
    }
    else {
        PI_ASSERT(hrigidbody->hscene->rbhead == hrigidbody);
        hrigidbody->hscene->rbhead = hrigidbody->pnext;
    }

    PI_FREE(hrigidbody);
}

/**/

PI_RIGIDBODY_PROPERTIES* pi_get_rigid_body_properties(PI_HRIGIDBODY hrigidbody)
{
    if (!hrigidbody) {
        return NULL;
    }

    return &hrigidbody->p_data->rbproperties;
}

bool pi_set_collision(PI_HRIGIDBODY hrigidbody, PI_COLLISION_TYPE type)
{
    if (!hrigidbody) {
        return false;
    }

    switch (type) {
        case PI_COLLISION_AABB:
            hrigidbody->p_data->clproperties.aabb.min = PI_INIT_STRUCT(PI_VECTOR3X){
                -0.5f,
                -0.5f,
                -0.5f
            };
            hrigidbody->p_data->clproperties.aabb.max = PI_INIT_STRUCT(PI_VECTOR3X){
                0.5f,
                0.5f,
                0.5f
            };
            break;
        case PI_COLLISION_OBB:
            hrigidbody->p_data->clproperties.obb.half_size = PI_INIT_STRUCT(PI_VECTOR3X){
                0.5f, 0.5f, 0.5f
            };
            hrigidbody->p_data->clproperties.obb.right = PI_INIT_STRUCT(PI_VECTOR3X){
                1.0f, 0.0f, 0.0f
            };
            hrigidbody->p_data->clproperties.obb.up = PI_INIT_STRUCT(PI_VECTOR3X){
                0.0f, 1.0f, 0.0f
            };
            break;
        case PI_COLLISION_NONE:
            break;
        default:
            return false;
    }

    hrigidbody->p_data->clproperties.type = type;
    return true;
}

PI_COLLISION_PROPERTIES* pi_get_collision(PI_HRIGIDBODY hrigidbody) {
    if (!hrigidbody) {
        return NULL;
    }
    if (hrigidbody->p_data->clproperties.type == PI_COLLISION_NONE) {
        return NULL;
    }

    return &hrigidbody->p_data->clproperties;
}

/**/

static PI_RIGIDBODY_DATA* append_rigidbody_data(PI_HSYSTEMSCENE hsysscene)
{
    if (hsysscene->rbfreelist_last) {
        PI_ASSERT(hsysscene->rbdata != NULL);
        PI_ASSERT(hsysscene->rbfreelist != NULL);

        unsigned rbindex = hsysscene->rbfreelist[hsysscene->rbfreelist_first];
        hsysscene->rbfreelist_first++;
        if (hsysscene->rbfreelist_first >= hsysscene->rbfreelist_last) {
            /* Freelist is empty, reset the indices */
            hsysscene->rbfreelist_last = 0;
            hsysscene->rbfreelist_first = 0;
        }
        return &hsysscene->rbdata[rbindex];
    }

    if (hsysscene->rbdata_count >= hsysscene->rbdata_capacity) {
        unsigned new_cap = 1 + (hsysscene->rbdata_capacity*3)/2;
        PI_RIGIDBODY_DATA* rbdata = PI_ALLOC(new_cap * sizeof(PI_RIGIDBODY_DATA), 0);
        if (!rbdata) {
            return NULL;
        }

        struct PI_RIGIDBODY_IMPL* cur = hsysscene->rbhead;
        unsigned newcount = 0;
        while (cur) {
            rbdata[newcount] = *cur->p_data;
            cur->p_data = &rbdata[newcount];
        
            cur = cur->pnext;
            newcount++;
        }

        hsysscene->rbfreelist_last = 0;
        hsysscene->rbfreelist_first = 0;

        PI_FREE(hsysscene->rbdata);
        hsysscene->rbdata_count = newcount;
        hsysscene->rbdata_capacity = new_cap;
        hsysscene->rbdata = rbdata;
    }

    PI_RIGIDBODY_DATA* rbdata = &hsysscene->rbdata[hsysscene->rbdata_count];
    hsysscene->rbdata_count++;

    /* Default values for the properties */
    rbdata->rbproperties.mass = 0.0f;
    
    rbdata->rbproperties.direction = PI_INIT_STRUCT(PI_VECTOR4){ 1.0f, 0.0f, 0.0f, 1.0f };
    rbdata->rbproperties.scale = PI_INIT_STRUCT(PI_VECTOR4){ 1.0f, 1.0f, 1.0f, 1.0f };
    rbdata->rbproperties.position = PI_INIT_STRUCT(PI_VECTOR3X){ 0.0f, 0.0f, 0.0f };

    rbdata->rbproperties.velocity = PI_INIT_STRUCT(PI_VECTOR3X){ 0.0f, 0.0f, 0.0f };
    rbdata->rbproperties.force = PI_INIT_STRUCT(PI_VECTOR3X){ 0.0f, 0.0f, 0.0f };
    rbdata->clproperties.type = PI_COLLISION_NONE;
    rbdata->p_transform = NULL;
    rbdata->p_dirtyflag = NULL;
    return rbdata;
}

/**/

bool pi_attach_transform(PI_HRIGIDBODY hrigidbody, PI_MATRIX_4X4* ptransform, void* p_dirtyflag)
{
    if (!hrigidbody) {
        return false;
    }

    if (p_dirtyflag && (!hrigidbody->p_data->p_dirtyflag || *hrigidbody->p_data->p_dirtyflag)) {
        /* `p_dirtyflag` gets set whenever there wasn't a flag previously, or if the previous flag was also set */
        *(uint8_t*)p_dirtyflag |= 1;
    }

    hrigidbody->p_data->p_transform = ptransform;
    hrigidbody->p_data->p_dirtyflag = (uint8_t*)p_dirtyflag;
    return true;
}
