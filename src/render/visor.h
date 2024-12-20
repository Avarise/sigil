#pragma once
#include "graphics.h"
#include "system.h"
#include "ntt.h"

// Visor vmnode
namespace sigil::visor {
    struct render_channel_t {
        sigil::sync_data_t sync;
    };

    // VM Tree controls
    status_t initialize();
    status_t deinitialize();

    // Renderer
    status_t create_render_channel(sigil::graphics::window_t *window, ntt::scene_t *scene);
    status_t start_render_channel(render_channel_t *ch);
    status_t stop_render_channel(render_channel_t *ch);
    status_t destroy_render_channel(render_channel_t *ch);
    
    // Window controls
    status_t check_for_swapchain_update(sigil::graphics::window_t *window);
    status_t prepare_imgui_drawdata(sigil::graphics::window_t *window);
    status_t prepare_for_imgui(sigil::graphics::window_t *window);
    // Deprecated
    sigil::graphics::window_t* spawn_window(const char *window_name); //Returns handle
    status_t present_window(sigil::graphics::window_t *window);
    status_t destroy_window(sigil::graphics::window_t *window);
    sigil::graphics::window_t* get_window(int index); //Returns handle

    // Utils
    status_t load_font(const std::string& font_path, float font_size);
    status_t new_frame(sigil::graphics::window_t *window);
    status_t render_frame(sigil::graphics::window_t *window);
    status_t present_frame(sigil::graphics::window_t *window);
}

