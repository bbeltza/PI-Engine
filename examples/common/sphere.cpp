#include "sphere.h"

#include <piengine/pi_math.h>

#include <vector>

PI_EXTERN_C PI_VERTEX* common::generate_sphere_mesh(float radius, int h_vertices, int v_vertices, PI_VERTEX* vertices, int* indices, bool flatshaded)
{
    static thread_local std::vector<PI_VERTEX> point_verts;
    static thread_local std::vector<float> h_anglesines;
    h_anglesines.resize(h_vertices*2);
    point_verts.reserve(calc_num_sphere_vertices(h_vertices, v_vertices, false));
    point_verts.clear();

    float inc = static_cast<float>(360_deg/h_vertices);
    float vinc = static_cast<float>(2.0/(v_vertices-1));

    {
        float angle = 0.0f;
        for (int i = 0; i < h_vertices; i++, angle += inc) {
            h_anglesines.at(i*2+0) = cosf(angle); 
            h_anglesines.at(i*2+1) = sinf(angle); 
        }
    }

    float y = -1.0f;
    for (int j = 0; j < v_vertices; j++, y += vinc) {
        float angle = 0.0f;
        float cy = static_cast<float>(cos(y*(PI_M_PI/2)));
        cy = powf(cy, 0.5f);
        for (int i = 0; i < h_vertices; i++, angle += inc) {
            float cx = h_anglesines.at(i*2+0);
            float sx = h_anglesines.at(i*2+1);
            PI_VERTEX vt{
                { cy*cx*radius, y*radius, cy*sx*radius },
                { cy*cx, y, cy*sx }
            };

            point_verts.push_back(vt);
        }
    }

    int num_indices = (h_vertices*(v_vertices-1)) * 6;
    if (flatshaded) {
        size_t vi = 0;
        for (size_t i = 0; i < point_verts.size() - h_vertices; i++) {
            int a = 1;
            a = i%h_vertices == h_vertices-1 ? 1-h_vertices : a;
            const PI_VERTEX* verts[] = {
                &point_verts.at(i+0),
                &point_verts.at(i+a),
                &point_verts.at(i+h_vertices+0),
                &point_verts.at(i+h_vertices+a)
            };
            
            PI_VECTOR3 normal{
                (verts[0]->normal.x + verts[1]->normal.x + verts[2]->normal.x + verts[3]->normal.x)*0.25f,
                (verts[0]->normal.y + verts[1]->normal.y + verts[2]->normal.y + verts[3]->normal.y)*0.25f,
                (verts[0]->normal.z + verts[1]->normal.z + verts[2]->normal.z + verts[3]->normal.z)*0.25f
            };
            PI_VERTEX vt;
            vt.normal = normal;
            for (const auto v : verts) {
                vt.pos = v->pos;
                vertices[vi] = vt;
                vi++;
            }
        }

        for (int i = 0, n = 0; i < num_indices; i+= 6, n += 4) {
            indices[i+0] = n+2;
            indices[i+1] = n+3;
            indices[i+2] = n+1;
            indices[i+3] = n+1;
            indices[i+4] = n+0;
            indices[i+5] = n+2;
        }
        return vertices;
    }
    else
    {
        for (int i = 0, n = 0; i < num_indices; i += 6, n++) {
            int face[4] = {
                n+0, n+h_vertices+0,
                n+1, n+h_vertices+1
            };

            indices[i+0] = face[0];
            indices[i+1] = face[1];
            indices[i+2] = face[2];
            indices[i+3] = face[2];
            indices[i+4] = face[1];
            indices[i+5] = face[3];
        }
        
        memcpy(vertices, point_verts.data(), sizeof(PI_VERTEX)*point_verts.size());
    }


    return vertices;
}
