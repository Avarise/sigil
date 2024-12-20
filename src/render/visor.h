#pragma once
#include "../core/system.h"
#include "window.h"

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
    //status_t create_render_channel(window_t *window, ntt::scene_t *scene);
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

