#pragma once
#include <thread>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include "../vm/core.h"
#include "../extern/imgui/backends/imgui_impl_vulkan.h"
#include "../extern/imgui/backends/imgui_impl_glfw.h"

namespace sigil {
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