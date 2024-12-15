/*
    Dependency for Visor module.
    Might use some vulkan headers, but will not interact with sigil's vulkan module
*/
#pragma once
#include <cstring>
#include <cstdint>
#include <thread>
#include <vector>
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "utils.h"

namespace sigil::graphics {
    enum theme_t {
        DARKMODE,
        LIGHTMODE,
        SYSTEM,
    };

    enum presentation_t {
        PRESENTATION_VSYNC_NONE,
        PRESENTATION_VSYNC_DOUBLE_BUFFERING,
        PRESENTATION_VSYNC_TRIPLE_BUFFERING,
        PRESENTATION_GSYNC,
        PRESENTATION_SYSTEM,
    };

    enum wd_borders_t {
        BORDERS_NORMAL,
        BORDERS_BORDERLESS,
        BORDERS_FULLSCREEN,
    };


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
        static_mesh_t();
        ~static_mesh_t();
        void randomize();
        std::vector<sigil::v3_t> verteces;
        sigil::v3_t position;
    };

    struct material_t {
    };

    struct dynamic_mesh_t {
        dynamic_mesh_t();
        ~dynamic_mesh_t();

        sigil::v3_t position;
        sigil::transform3d transform;
        std::vector<sigil::v3_t> verteces;
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
        theme_t theme;
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

    sigil::status_t initialize_glfw();
}