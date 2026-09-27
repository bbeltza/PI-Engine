#define PI_INCLUDE_BACKEND_API 1
#include "render_internal.hpp"

const PI_INIT_SYSTEM _PI_INIT_RENDER = {
    [](size_t* allocsize) -> bool {
        *allocsize = sizeof(PI_SYSTEM_RENDER);
        return true;
    },
    [](void* hsystem, PI_ENGINEDATA* hengine) -> bool {
        auto renderer = new (hsystem) PI_SYSTEM_RENDER{ hengine };
        return false;
    },
    [](void* hsystem) -> void {
        auto renderer = static_cast<PI_SYSTEM_RENDER*>(hsystem);
        renderer->~PI_SYSTEM_RENDER();
    }
};

PI_SYSTEM_RENDER::PI_SYSTEM_RENDER(PI_ENGINEDATA* hengine)
{
}

PI_EXTERN_C PI_HMESH pi_create_mesh_indexed(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[], int num_indices, const void* indices, PI_INDEX index_type) {
    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->subsystems.render->create_mesh_indexed(
            num_vertices, vertices,
            num_indices, indices, index_type);
}

PI_EXTERN_C PI_HMESH pi_create_mesh(PI_HGAME hgame, int num_vertices, const PI_VERTEX vertices[]) {
    return pi_create_mesh_indexed(hgame, num_vertices, vertices, 0, NULL, PI_INDEX_UNKNOWN);
}

PI_EXTERN_C void pi_destroy_mesh(PI_HMESH hmesh) {
    if (!hmesh) {
        return;
    }
    hmesh->renderer->destroy_mesh(hmesh);
}

PI_EXTERN_C PI_HSCENE pi_create_scene(PI_HGAME hgame) {
    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->subsystems.render->create_scene();
}

PI_EXTERN_C void pi_release_scene(PI_HSCENE hscene) {
    hscene->renderer->release_scene(hscene);
}

#define HSCENE_ERROR reinterpret_cast<PI_HSCENE>(UINTPTR_MAX)

PI_EXTERN_C PI_HSCENE pi_set_current_scene(PI_HGAME hgame, PI_HSCENE hscene) {
    auto engine = hgame ? PI_ENGINE_FROM_HGAME(hgame) : NULL;
    if (!engine || (hscene && engine->subsystems.render != hscene->renderer)) { // `hscene` can beNULL, used to unbind the scene from the game
        return HSCENE_ERROR;
    }

    return engine->subsystems.render->set_current_scene(hscene);
}
PI_EXTERN_C PI_HSCENE pi_get_current_scene(PI_HGAME hgame) {
    if (!hgame) {
        return HSCENE_ERROR;
    }

    auto engine = PI_ENGINE_FROM_HGAME(hgame);
    return engine->subsystems.render->get_current_scene();
}

PI_EXTERN_C bool pi_set_scene_clear_color(PI_HSCENE hscene, float r, float g, float b, float a) {
    /* TODO: Add assertion??? I don't know just to check if `hscene` is non-NULL. I don't have any assert headers included right now aaaaaaahaaAAAHHH  */

    hscene->clear_color[0] = r;
    hscene->clear_color[1] = g;
    hscene->clear_color[2] = b;
    hscene->clear_color[3] = a;

    return true;
}
PI_EXTERN_C bool pi_set_scene_projection(PI_HSCENE hscene, const float mproj[16]) {
    /* Meh just make a memcpy :D */
    std::memcpy(hscene->projection, mproj, sizeof(hscene->projection));
    hscene->updates.viewproj = true; // Here we need to notify update state since the view/projection is to be updated inside a buffer (managed by the backend (D3D or GL....))
    return true;
}
PI_EXTERN_C bool pi_set_scene_view(PI_HSCENE hscene, const float mview[16]) {
    std::copy(mview, mview + 16, hscene->view); // std::memcpy vs std::copy
    hscene->updates.viewproj = true;
    return true;
}

PI_EXTERN_C PI_HINSTANCE pi_add_scene_mesh(PI_HSCENE hscene, PI_HMESH hmesh, const PI_INSTANCE* instance) {
    if (!hscene) {
        return NULL;
    }
    if (!hmesh) {
        return NULL;
    }
    if (hmesh->renderer != hscene->renderer) {
        return NULL;
    }

    return hscene->add_scene_mesh(hmesh, instance);
}
PI_EXTERN_C void pi_remove_scene_mesh(PI_HINSTANCE hmeshinst) {
    if (!hmeshinst) {
        return;
    }
    hmeshinst->hscene->remove_scene_mesh(hmeshinst);
}
PI_EXTERN_C bool pi_update_instance(PI_HINSTANCE hmeshinst, const PI_INSTANCE* instance) {
    return hmeshinst->update_instance(instance);
}
