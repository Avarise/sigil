/*
    Dependency for Visor module.
    Might use some vulkan headers, but will not interact with sigil's vulkan module
*/
#pragma once
#include <cstring>
#include <cstdint>
#include <vector>
#include "utils.h"

namespace sigil::graphics {
    struct color_t {
        uint8_t red, green, blue, alpha;
        color_t() { memset((void*)this, 0, sizeof(*this)); }
    };

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

        struct window_frame_t {
        VkCommandPool       CommandPool;
        VkCommandBuffer     CommandBuffer;
        VkFence             Fence;
        VkImage             Backbuffer;
        VkImageView         BackbufferView;
        VkFramebuffer       Framebuffer;
    };

    struct window_frame_semaphores_t {
        VkSemaphore         ImageAcquiredSemaphore;
        VkSemaphore         RenderCompleteSemaphore;
    };

    struct window_t {
        enum theme_t {
            DARKMODE,
            LIGHTMODE,
            SYSTEM,
        } theme;

        enum presentation_t {
            PRESENTATION_VSYNC_NONE,
            PRESENTATION_VSYNC_DOUBLE_BUFFERING,
            PRESENTATION_VSYNC_TRIPLE_BUFFERING,
            PRESENTATION_GSYNC,
            PRESENTATION_SYSTEM,
        } presentation;

        enum borders_t {
            BORDERS_NORMAL,
            BORDERS_BORDERLESS,
            BORDERS_FULLSCREEN,
        } borders;

        // Basics
        window_t();
        ~window_t();

        // Sync changes to windows with this
        sigil::status_t refresh_properties();
        // Vertical sync, GSync, no sync, etc
        sigil::status_t set_presentation_mode(presentation_t mode);
        sigil::status_t set_theme(theme_t);
        // Content is a procedure that arranges ImGui and other renderer functions
        // into a frame data
        sigil::status_t set_content(status_t (*gui_routine)(void));
        // Start a thread for immidiate mode render, after deploy
        // windows is automatically update
        // At this stage windows should be all setup and ready for user I/O
        std::thread *deploy();
        // Issue windows shutdown, and thread teardown. Use wait_for_shutdown
        // to ensure you can teardown Vulkan
        sigil::status_t shutdown();
        void wait_for_shutdown();

        // Window properties
        std::string name;
        std::thread *thread;
        int width, height;
        GLFWwindow *glfw_window;
        VkSwapchainKHR vk_swapchain;
        bool swapchain_rebuild;
        bool clear_enable;
        bool use_dynamic_rendering;
        VkSurfaceKHR vk_surface;
        VkSurfaceFormatKHR vk_surface_format;
        VkPresentModeKHR vk_present_mode;
        VkRenderPass vk_render_pass;
        VkPipeline vk_pipeline;
        ImGui_ImplVulkanH_Window imgui_wd;
        VkClearValue clear_value;
        uint32_t min_image_count;
        uint32_t frame_index;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
        uint32_t image_count;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
        uint32_t semaphore_count;         // Number of simultaneous in-flight frames + 1, to be able to use it in vkAcquireNextImageKHR
        uint32_t semaphore_index;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
        window_frame_t* frames;
        window_frame_semaphores_t* frame_semaphores;
        status_t (*gui_routine)(void) = nullptr;

    };
} 


// Basic components for global ecs
namespace sigil {

}