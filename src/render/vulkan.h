#pragma once
/*
    Configuration host for Vulkan.
    Can configure: physical and logical device, vk instance, pipeline caching etc.
*/

#include <vulkan/vulkan_core.h>
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdio>
#include "utils.h"

namespace sigil::vulkan {
    // VM Tree API
    sigil::status_t deinitialize();
    sigil::status_t initialize();

    // Vulkan specific controls
    sigil::status_t reset_vulkan();
    sigil::status_t probe_devices();
    sigil::status_t clean_pipeline_cache();
    sigil::status_t create_default_queues();
    sigil::status_t select_primary_device(int index);

    struct vk_qinfo {
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

    const char* result_to_cstring(VkResult res);
    void check_result(VkResult res);

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
}
