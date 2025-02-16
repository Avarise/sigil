/*
    Vulkan based renderer.
    Contains array of target scenes, and can create view-channels to attach 
    output to a window/surface/viewport
*/

#pragma once
#include "graphics.h"
#include "ntt.h"
#include "utils.h"

namespace sigil::visor {
    struct render_channel_t {
        sigil::sync_data_t sync;
        void (*init_function)(void);
        void (*task)(void);
        //void (*init_function)(void);
    };

    /*
        Common SigilVM calls
    */
    status_t initialize();
    status_t deinitialize();

    /*
        Render channels connect a scene with window.
    */
    status_t create_render_channel(sigil::graphics::window_t *window, ntt::scene_t *scene);
    status_t destroy_render_channel(render_channel_t *ch);
    status_t start_render_channel(render_channel_t *ch);
    status_t stop_render_channel(render_channel_t *ch);
    
    /*
        Window operations:
        Creating/destroying
        prepare/finalize, in between, draw ImGui elements
        detect swapchain resize etc

    */
    sigil::graphics::window_t* spawn_window(const char *window_name); //Returns handle
    status_t destroy_window(sigil::graphics::window_t *window);
    status_t check_for_swapchain_update(sigil::graphics::window_t *window);
    status_t prepare_for_imgui(sigil::graphics::window_t *window);
    status_t prepare_for_new_frame(sigil::graphics::window_t *window);
    status_t finalize_new_frame(sigil::graphics::window_t *window);

    /*
        Global atlas
    */
}

