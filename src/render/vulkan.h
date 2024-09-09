#pragma once
/*
    Configuration host for Vulkan.
    Can configure: physical and logical device, vk instance, pipeline caching etc.

    Vulkan based renderer.
    Contains array of target scenes, and can create view-channels to attach 
    output to a window/surface/viewport
*/

#include <vulkan/vulkan_core.h>
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <cstdio>
#include "../vm/core.h"
#include "window.h"
#include "../vm/node.h"

// TODO: Remove this dependency, vulkan should not interact with ECS
#include "../engine/ntt.h"

namespace sigil::vulkan {
    struct vk_queue {
        VkQueue queue;
        uint32_t family;
    };

    struct vk_framedata_t {
        VkCommandPool CommandPool;
        VkCommandBuffer CommandBuffer;
        VkFence Fence;
        VkImage Backbuffer;
        VkImageView BackbufferView;
        VkFramebuffer Framebuffer;
    };
    
    struct vk_frame_semaphores_t {
        VkSemaphore ImageAcquiredSemaphore;
        VkSemaphore RenderCompleteSemaphore;
    };



    struct host_data_t {
        bool compute_mode;
        // Number of physical devices found and registered
        uint32_t num_phys, num_registered;
        // Vector of all physical devices found
        std::vector<VkPhysicalDevice> phy_dev_all;
        // Use phy at 0 as active GPU, and other indeces as aux GPUs
        std::vector<VkPhysicalDevice> phy_dev_registered;
        // Vulkan instance
        VkInstance vk_inst;
        // Extensions loaded for vulkan
        std::vector<const char*> vk_inst_ext;
        // Logical device for Vulkan
        VkDevice vk_dev;
        // Vulkan command queues and other control structures
        vk_queue vk_main_q, vk_graphics_q, vk_compute_q;
        VkPipelineCache vk_pipeline_cache;
        VkAllocationCallbacks* vk_allocators;
        VkDebugReportCallbackEXT vk_dbg_callback_ext;
        VkDescriptorPool vk_descriptor_pool;


        bool has_gpu() {
            if (phy_dev_registered.size() > 0) return true;
            return false;
        }

        VkPhysicalDevice main_gpu() {
            if (has_gpu()) return phy_dev_registered.at(0);
            return nullptr;
        }

    };

    inline const char* result_to_cstring(VkResult err) {
        if (err == VK_SUCCESS)                      return "VK_SUCCESS";
        if (err == VK_NOT_READY)                    return "VK_NOT_READY";
        if (err == VK_TIMEOUT)                      return "VK_TIMEOUT";
        if (err == VK_EVENT_SET)                    return "VK_EVENT_SET";
        if (err == VK_EVENT_RESET)                  return "VK_EVENT_RESET";
        if (err == VK_INCOMPLETE)                   return "VK_INCOMPLETE";
        if (err == VK_ERROR_OUT_OF_HOST_MEMORY)     return "VK_ERROR_OUT_OF_HOST_MEMORY";
        if (err == VK_ERROR_OUT_OF_DEVICE_MEMORY)   return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        if (err == VK_ERROR_INITIALIZATION_FAILED)  return "VK_ERROR_INITIALIZATION_FAILED";
        if (err == VK_ERROR_DEVICE_LOST)            return "VK_ERROR_DEVICE_LOST";
        if (err == VK_ERROR_MEMORY_MAP_FAILED)      return "VK_ERROR_MEMORY_MAP_FAILED";
        if (err == VK_ERROR_LAYER_NOT_PRESENT)      return "VK_ERROR_LAYER_NOT_PRESENT";
        if (err == VK_ERROR_EXTENSION_NOT_PRESENT)  return "VK_ERROR_EXTENSION_NOT_PRESENT";
        if (err == VK_ERROR_FEATURE_NOT_PRESENT)    return "VK_ERROR_FEATURE_NOT_PRESENT";
        if (err == VK_ERROR_INCOMPATIBLE_DRIVER)    return "VK_ERROR_INCOMPATIBLE_DRIVER";
        if (err == VK_ERROR_TOO_MANY_OBJECTS)       return "VK_ERROR_TOO_MANY_OBJECTS";
        if (err == VK_ERROR_FORMAT_NOT_SUPPORTED)   return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        if (err == VK_ERROR_FRAGMENTED_POOL)        return "VK_ERROR_FRAGMENTED_POOL";
        if (err == VK_ERROR_UNKNOWN)                return "VK_ERROR_UNKNOWN";
        if (err == VK_ERROR_OUT_OF_POOL_MEMORY)     return "VK_ERROR_OUT_OF_POOL_MEMORY";
        if (err == VK_ERROR_INVALID_EXTERNAL_HANDLE)return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        return nullptr;
    }

    inline void check_result(VkResult result) {
        if (result == 0) return;
        
        printf("vulkan: result -> %s (code: %d)\n", result_to_cstring(result), result);
        if (result < 0) {
            printf("vulkan: aborting program execution...\n");
            abort();
        }
    }

    inline int vk_present_mode_to_img_count(VkPresentModeKHR present_mode) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return 3;
        if (present_mode == VK_PRESENT_MODE_FIFO_KHR || present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
            return 2;
        if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return 1;
        return 1;
    }

    // VM Tree API
    sigil::status_t initialize(sigil::vmnode_t *vmsr);
    sigil::vmnode_t probe(sigil::vmnode_t *vmsr);
    sigil::status_t deinitialize();
    host_data_t* api_handle(sigil::vmnode_t *vmsr);

    // Vulkan control calls
    sigil::status_t reset_vulkan();
    sigil::status_t attach_to_window(sigil::window_t *window);
    sigil::status_t release_window(sigil::window_t *window);
    sigil::status_t select_presentation_mode(sigil::window_t *window, sigil::window_t::presentation_t mode);
    sigil::status_t select_surface_format(sigil::window_t *window);
    


    // Deprecated
    int attach_window(sigil::window_t *window);
    int attach_scene(sigil::ntt::scene_t *scene);
    int setup_window_for_vk(sigil::window_t *window);
    int vk_set_presentation_mode(sigil::window_t *window, sigil::window_t::presentation_t mode);
    int vk_set_surface_format(sigil::window_t *window);
    int vk_force_create_window(sigil::window_t *window);
}
