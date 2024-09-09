#pragma once
#include "../engine/runtime.h"
#include "../utils/math.h"
#include "../vm/core.h"
#include "../vm/node.h"
#include "window.h"
#include "std.h"

// Visor vmnode
namespace sigil::visor {
    struct render_channel_t {
        sigil::sync_data_t sync;
    };

    struct visor_data_t {
        std::vector<render_channel_t> render_channels;
        std::vector<sigil::window_t*> windows;
        bool glfw_initialized = false;
        visor_data_t();
        ~visor_data_t();
        status_t soft_init_glfw();
    };
    

    // Virtual machine API
    status_t initialize(sigil::vmnode_t *vmsr);
    vmnode_t probe(sigil::vmnode_t *vmsr);
    status_t deinitialize();

    status_t prepare_for_imgui(sigil::window_t *window);
    status_t check_for_swapchain_update(sigil::window_t *window);
    status_t new_frame(window_t *window);
    status_t render_frame(window_t *window);
    status_t present_frame(window_t *window);
    status_t prepare_imgui_drawdata(window_t *window);
    status_t load_font(const std::string& font_path, float font_size);


    // Renderer API
    status_t create_render_channel(window_t *window, ntt::scene_t *scene);
    status_t destroy_render_channel(render_channel_t *ch);
    // Stop/start render channel. Channel state is separate from scene;
    // Stopping scene stops physics etc, stopping channel freezes channel on the last frame
    status_t start_render_channel(render_channel_t *ch);
    status_t stop_render_channel(render_channel_t *ch);

    // Create and destroy windows via active render backend
    // Currently, it is limited to Vulkan only.
    window_t* spawn_window(const char *window_name); //Returns handle
    window_t* get_window(int index); //Returns handle
    status_t destroy_window(window_t *window);
    status_t present_window(window_t *window);
}

// Basic components for global ecs
namespace sigil {
    struct texture_component_t {
        //sigil::asset_t *data;
        texture_component_t() {
            //data = nullptr;
        }
    };

    struct sprite_t {

    };

    struct light_component_t {
        v3_t position;
        uint8_t brightness; // 0-255 0% - 100%
        uint32_t field_of_view; // in degrees
    };

    struct static_mesh_t {
        std::vector<sigil::v3_t> verteces;
        sigil::v3_t position;

        static_mesh_t() {
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            verteces = {};
        }

        void randomize() {
            verteces = {};
            int num_new_verts = random_i32_scoped(5, 124);
            for (int i = 0; i < num_new_verts; i++) {
                float vert_x = random_i32_scoped(0, 255) * 0.01f;
                float vert_y = random_i32_scoped(0, 255) * 0.01f;
                float vert_z = random_i32_scoped(0, 255) * 0.01f;
                sigil::v3_t new_ver(vert_x, vert_y, vert_z);
                verteces.push_back(new_ver);
            }
        }
    };

    struct material_t {
    };

    struct dynamic_mesh_t {
        sigil::v3_t position;
        sigil::transform3d transform;
        std::vector<sigil::v3_t> verteces;

        dynamic_mesh_t() {
            //asset_data = NULL;
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            transform.rotation.r = 0.0f;
            transform.rotation.i = 0.0f;
            transform.rotation.j = 0.0f;
            transform.rotation.k = 0.0f;
            transform.translation.x = 0.0f;
            transform.translation.y = 0.0f;
            transform.translation.z = 0.0f;
            transform.scale.x = 1.0f;
            transform.scale.y = 1.0f;
            transform.scale.z = 1.0f;
        }
    };

    // Basic camera uses 3d calculations, however
    // default view will be 2d, to simulate plane
    // mechanics, like particle colision
    struct camera_t {
        sigil::v3_t position;
        sigil::transform3d transform;
        uint8_t detail_level; // 0-255 0% - 100%
    };

}