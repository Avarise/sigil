#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdint>
#include <vector>
#include "graphics.h"
#include "system.h"
#include "utils.h"
#include "virtual-machine.h"
#include "vulkan.h"
#include "visor.h"

// Vulkan data
sigil::vulkan::vk_qinfo vk_main_q, vk_graphics_q, vk_compute_q;
VkDebugReportCallbackEXT vk_dbg_callback_ext;
VkAllocationCallbacks* vk_allocators;
std::vector<const char*> vk_inst_ext;
VkDescriptorPool vk_descriptor_pool;
VkPipelineCache vk_pipeline_cache;
static bool compute_mode = false;
VkInstance vk_inst;
VkDevice vk_dev;

// Physical devices managed via Vulkan
uint32_t num_phy_devices = 0;
uint32_t num_registered_phy_devices = 0; // Register main device at 0th index
std::vector<VkPhysicalDevice> phy_dev_all;
std::vector<VkPhysicalDevice> phy_dev_registered;

// SigilVM specific data
static sigil::vmnode_t *vulkan_node = nullptr;

// Vulkan setup procedures
static sigil::status_t initialize_vulkan_physical_devices(); // GPUs
static sigil::status_t initialize_vulkan_descriptor_pool();
static sigil::status_t initialize_vulkan_extensions(); 
static sigil::status_t initialize_vulkan_instance();
static sigil::status_t initialize_vulkan_queues();



static bool has_gpu() {
    if (phy_dev_registered.size() > 0) return true;
    return false;
}

VkPhysicalDevice get_main_gpu() {
    if (has_gpu()) return phy_dev_registered.at(0);
    return nullptr;
}

bool is_vk_ext_available(const std::vector<VkExtensionProperties> &properties, const char* extension) {
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0) {
            if (sigil::virtual_machine::get_debug_mode())
                printf("vulkan: extenstion %s available\n", extension);
            return true;
        }
    return false;
}

// VM Tree integration
sigil::status_t sigil::vulkan::initialize() {
    sigil::exec_timer tmr;
    tmr.start();
    sigil::status_t status = virtual_machine::get_state();
    if (status != VM_OK) return status;

    sigil::vmnode_descriptor_t node_info;
    node_info.name.value = "vulkan";
    status = sigil::virtual_machine::add_platform_node(node_info);

    status = initialize_vulkan_instance();
    status = initialize_vulkan_physical_devices();
    //status = initialize_vulkan_queues();
    //status = setup_vulkan_descriptor_pool();


    
    tmr.stop();
    if (virtual_machine::get_debug_mode()) {
        printf("vulkan: initialized in %luns\n", tmr.ns());
    }

    return status;
}

sigil::status_t sigil::vulkan::deinitialize() {
    //VkResult res = vkDeviceWaitIdle(vulkan_data->vk_dev);
    // sigil::vulkan::check_result(res);
    // ImGui_ImplVulkan_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();
    // //ImGui_ImplVulkanH_DestroyWindow(app_data.vkhost->vk_inst, app_data.vkhost->vk_dev, &app_data.imgui_wd, app_data.vkhost->vk_allocators);
    // vkDestroyDescriptorPool(vulkan_data->vk_dev, vulkan_data->vk_descriptor_pool, vulkan_data->vk_allocators);
    // vkDestroyDevice(vulkan_data->vk_dev, vulkan_data->vk_allocators);
    // vkDestroyInstance(vulkan_data->vk_inst, vulkan_data->vk_allocators);

    glfwTerminate();
    return VM_OK;
}

sigil::status_t initialize_vulkan_instance() {
    // assert(vulkan_node != nullptr);
    // assert(vulkan_data != nullptr);
    // assert(vulkan_data->vk_inst == nullptr);
    // assert(vulkan_data->vk_allocators == nullptr);

    sigil::exec_timer tmr;
    tmr.start();

    // Extension properties enabled by default, we use those to determine features available
    std::vector<VkExtensionProperties> properties_implicit;
    uint32_t properties_implicit_count;
    VkResult err;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Probe for implicit extensions
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_implicit_count, nullptr);
    properties_implicit.resize(properties_implicit_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_implicit_count, properties_implicit.data());
    sigil::vulkan::check_result(err);

    // Load GUI extensions if not in compute mode
    if (!compute_mode) {
        sigil::graphics::initialize_glfw();
        if (!glfwVulkanSupported()) {
           printf("vulkan: Vulkan GLFW Not Supported\n");
            return sigil::VM_NOT_SUPPORTED;
        }
        
        uint32_t glfw_extensions_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
        for (uint32_t i = 0; i < glfw_extensions_count; i++) {
            vk_inst_ext.push_back(glfw_extensions[i]);
        }
        
        if (is_vk_ext_available(properties_implicit, 
                                            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            vk_inst_ext.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (is_vk_ext_available(properties_implicit, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            vk_inst_ext.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif
    }

    create_info.enabledExtensionCount = (uint32_t)vk_inst_ext.size();
    create_info.ppEnabledExtensionNames = vk_inst_ext.data();
    //printf("vulkan: enabled Vulkan extensions: %u\n", create_info.enabledExtensionCount);
    err = vkCreateInstance(&create_info, vk_allocators, &vk_inst);
    sigil::vulkan::check_result(err);
    tmr.stop();
    printf("vulkan: instance created in %lums\n", tmr.ms());
    return sigil::VM_OK;
}

sigil::status_t initialize_vulkan_physical_devices() {
    sigil::status_t status;

    // First get number of devices found
    VkResult err = vkEnumeratePhysicalDevices(vk_inst, &num_phy_devices, nullptr);
    sigil::vulkan::check_result(err);

    if (num_phy_devices == 0) {
        // TODO: Swap for logger
        printf("vulkan: no GPUs found...\n");
        return sigil::VM_NOT_FOUND;
    } else {
        printf("vulkan: found %u GPUs\n", num_phy_devices);
    }

    // Resize physical device list and populate it
    phy_dev_all.resize(num_phy_devices);
    err = vkEnumeratePhysicalDevices(vk_inst, &num_phy_devices, 
                                     phy_dev_all.data());
    sigil::vulkan::check_result(err);

    // Seek and register entries with VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    for (uint32_t i = 0; i < num_phy_devices; i++) {
        VkPhysicalDevice device = phy_dev_all.at(i);
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        printf("vulkan: checking %s...\n", properties.deviceName);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            phy_dev_registered.push_back(device);
            printf("vulkan: %s registered\n", properties.deviceName);
        }
    }

    if (phy_dev_registered.size() > 0) return sigil::VM_OK;

    phy_dev_registered.push_back(phy_dev_all.at(0));
    // TODO: Change to log
    printf("vulkan: no discrete GPU entry found, using default\n");
    return sigil::VM_OK;
}

// sigil::status_t setup_vulkan_descriptor_pool() {
//     VkResult err;

//     VkDescriptorPoolSize pool_sizes[] = {
//         { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
//     };
    
//     VkDescriptorPoolCreateInfo pool_info = {};
//     pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//     pool_info.maxSets = 1;
//     pool_info.poolSizeCount = (uint32_t)sizeof(pool_info);
//     pool_info.pPoolSizes = pool_sizes;
//     err = vkCreateDescriptorPool(vk_dev, &pool_info, 
//                                  vk_allocators, 
//                                  &vk_descriptor_pool);
//     sigil::vulkan::check_result(err);
//     return sigil::VM_OK;
// }



// static void cleanup_vulkan_data() {
//     if (!vulkan_node) return;
//     if (!vulkan_node->node_data.data) return;
//     if (vulkan_node->node_data.data != vulkan_data) {
//         printf("Error: Allocated Vulkan data does not match the cache\n");
//         return;
//     }

//     sigil::vulkan::deinitialize();

//     delete(vulkan_data);
//     vulkan_node->node_data.data = nullptr;
//     vulkan_data = nullptr;
// }


// // vulkan::host_data_t* vulkan::api_handle(sigil::vmnode_t *node) {
// //     return vulkan_data;
// // }


// sigil::status_t initialize_vulkan_queues() {
//     assert(vulkan_node != nullptr);
//     assert(vulkan_data != nullptr);
//     assert(vulkan_data->main_gpu() != nullptr);
//     VkResult err;

//     std::vector<VkExtensionProperties> properties;
//     uint32_t properties_count;
//     uint32_t count;

//     vkGetPhysicalDeviceQueueFamilyProperties(vulkan_data->main_gpu(), &count, nullptr);
//     VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
//     vkGetPhysicalDeviceQueueFamilyProperties(vulkan_data->main_gpu(), &count, queues);

//     for (uint32_t i = 0; i < count; i++) {
//         if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//             vulkan_data->vk_main_q.family = i;
//             break;
//         }
//     }

//     free(queues);
//     assert(vulkan_data->vk_main_q.family != (uint32_t)-1);

//     std::vector<const char*> vk_dev_ext;
//     // Create Logical Device (with 1 queue)
//     vk_dev_ext.push_back("VK_KHR_swapchain");
//     properties_count = 0;
//     properties = {};

//     // Enumerate physical device extension
//     vkEnumerateDeviceExtensionProperties(vulkan_data->main_gpu(), nullptr, &properties_count, nullptr);
//     properties.resize(properties_count);
//     vkEnumerateDeviceExtensionProperties(vulkan_data->main_gpu(), nullptr, &properties_count, properties.data());
// #   ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
//     if (sigil::vulkan::is_vk_ext_available(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
//         vk_dev_ext.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
// #   endif

//     const float queue_priority[] = { 1.0f };
//     VkDeviceQueueCreateInfo queue_info[1] = {};
//     queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//     queue_info[0].queueFamilyIndex = vulkan_data->vk_main_q.family;
//     queue_info[0].queueCount = 1;
//     queue_info[0].pQueuePriorities = queue_priority;
//     VkDeviceCreateInfo create_info = {};
//     create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
//     create_info.pQueueCreateInfos = queue_info;
//     create_info.enabledExtensionCount = (uint32_t)vk_dev_ext.size();
//     create_info.ppEnabledExtensionNames = vk_dev_ext.data();

//     //printf("vkDevice: %p\n", vulkan_data->vk_dev);

//     err = vkCreateDevice(vulkan_data->main_gpu(), &create_info, vulkan_data->vk_allocators, &vulkan_data->vk_dev);
//     sigil::vulkan::check_result(err);
//     //printf("vkDevice: %p\n", vulkan_data->vk_dev);
    
//     vkGetDeviceQueue(vulkan_data->vk_dev, vulkan_data->vk_main_q.family, 0, &vulkan_data->vk_main_q.queue);
//     return sigil::VM_OK;
// }

// sigil::status_t initialize_vulkan_extensions() {
//     sigil::status_t err;

//     err = initialize_vulkan_queues();
    
//     return sigil::VM_OK;
//     return err;
// }


// sigil::status_t enumerate_required_extensions() {
//     assert(vulkan_node != nullptr);
//     assert(vulkan_data != nullptr);
    
//     sigil::status_t err;


//     return sigil::VM_OK;
//     return err;
// }




// sigil::status_t vulkan::attach_to_window(sigil::window_t *window) {
//     return sigil::VM_OK;
// }

// // Copy of ImGui backend implementation
// // TODO: Remake into native functions
// void destroy_frame(VkDevice device, sigil::window_frame_t* fd, const VkAllocationCallbacks* allocator) {
//     vkDestroyFence(device, fd->Fence, allocator);
//     vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
//     vkDestroyCommandPool(device, fd->CommandPool, allocator);
//     fd->Fence = VK_NULL_HANDLE;
//     fd->CommandBuffer = VK_NULL_HANDLE;
//     fd->CommandPool = VK_NULL_HANDLE;

//     vkDestroyImageView(device, fd->BackbufferView, allocator);
//     vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
// }

// void destroy_frame_semaphores(VkDevice device, sigil::window_frame_semaphores_t* fsd, const VkAllocationCallbacks* allocator)
// {
//     vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
//     vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
//     fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
// }

// void create_window_command_buffer(VkPhysicalDevice physical_device, VkDevice device, sigil::window_t* wd, uint32_t queue_family, const VkAllocationCallbacks* allocator)
// {
//     assert(physical_device != VK_NULL_HANDLE && device != VK_NULL_HANDLE);
//     assert(physical_device);

//     // Create Command Buffers
//     VkResult err;
//     for (uint32_t i = 0; i < wd->image_count; i++)
//     {
//         sigil::window_frame_t* fd = &wd->frames[i];
//         {
//             VkCommandPoolCreateInfo info = {};
//             info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//             info.flags = 0;
//             info.queueFamilyIndex = queue_family;
//             err = vkCreateCommandPool(device, &info, allocator, &fd->CommandPool);
//             vulkan::check_result(err);
//         }
//         {
//             VkCommandBufferAllocateInfo info = {};
//             info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//             info.commandPool = fd->CommandPool;
//             info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//             info.commandBufferCount = 1;
//             err = vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
//             vulkan::check_result(err);
//         }
//         {
//             VkFenceCreateInfo info = {};
//             info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//             info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//             err = vkCreateFence(device, &info, allocator, &fd->Fence);
//             vulkan::check_result(err);
//         }
//     }

//     for (uint32_t i = 0; i < wd->semaphore_count; i++)
//     {
//         sigil::window_frame_semaphores_t* fsd = &wd->frame_semaphores[i];
//         {
//             VkSemaphoreCreateInfo info = {};
//             info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//             err = vkCreateSemaphore(device, &info, allocator, &fsd->ImageAcquiredSemaphore);
//             vulkan::check_result(err);
//             err = vkCreateSemaphore(device, &info, allocator, &fsd->RenderCompleteSemaphore);
//             vulkan::check_result(err);
//         }
//     }

//    printf("vulkan: recreated window command buffer\n");
// }



    

// int sigil::vulkan::vk_set_surface_format(sigil::window_t *window) {
//     const VkFormat request_formats[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//     const VkColorSpaceKHR request_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    
//     uint32_t avail_count;


//     vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_data->main_gpu(), window->vk_surface, &avail_count, nullptr);
//     std::vector<VkSurfaceFormatKHR> avail_format;
//     avail_format.resize((int)avail_count);
//     vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_data->main_gpu(), window->vk_surface, &avail_count, avail_format.data());

//     // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
//     if (avail_count == 1) {
//         if (avail_format[0].format == VK_FORMAT_UNDEFINED) {
//             VkSurfaceFormatKHR ret;
//             ret.format = request_formats[0];
//             ret.colorSpace = request_color_space;
//             window->vk_surface_format = ret;
//         } else {
//             // No point in searching another format
//             window->vk_surface_format = avail_format[0];
//         }
//     } else {
//         // Request several formats, the first found will be used
//         for (int request_i = 0; request_i < sizeof(request_formats) / sizeof(VkFormat); request_i++)
//             for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
//                 if (avail_format[avail_i].format == request_formats[request_i] && avail_format[avail_i].colorSpace == request_color_space) {
//                     window->vk_surface_format = avail_format[avail_i];
//                     break;
//                 }

//         // If none of the requested image formats could be found, use the first available
//         window->vk_surface_format = avail_format[0];
//     }
//     return 0;
// }

// int sigil::vulkan::vk_set_presentation_mode(sigil::window_t *window, sigil::window_t::presentation_t mode) {
//     assert(vulkan_node != nullptr);
//     assert(vulkan_data != nullptr);
//     assert(vulkan_data->main_gpu() != VK_NULL_HANDLE);
//     assert(window->vk_surface != NULL);

//     VkPhysicalDevice main_dev = vulkan_data->main_gpu();

//     std::vector<VkPresentModeKHR> present_modes = {};

//     if (mode == sigil::window_t::PRESENTATION_VSYNC_TRIPLE_BUFFERING) {
//         present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
//     } else {
//         present_modes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
//         present_modes.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);
//         present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
//     }

//     uint32_t avail_count = 0;
//     vkGetPhysicalDeviceSurfacePresentModesKHR(main_dev,
//                                              window->vk_surface,
//                                              &avail_count, nullptr);


//     present_modes.resize((int)avail_count);
//     vkGetPhysicalDeviceSurfacePresentModesKHR(main_dev, window->vk_surface, &avail_count, present_modes.data());
//     //for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
//     //   printf("[vulkan] avail_modes[%d] = %d\n", avail_i, avail_modes[avail_i]);

//     for (int request_i = 0; request_i < avail_count; request_i++)
//         for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
//             if (present_modes[request_i] == present_modes[avail_i])
//                 window->vk_present_mode = present_modes[request_i];

//     //return VK_PRESENT_MODE_FIFO_KHR; // Always available

//    printf("vulkan: vk_present_mode set to %d\n",window->vk_present_mode);
//     return 0;
// }



// int sigil::vulkan::setup_window_for_vk(sigil::window_t *window) {
//     if (!window) return -ENOMEM;
//     assert(vulkan_data != nullptr);
//     assert(vulkan_data->vk_inst != nullptr);
//     assert(window->glfw_window != nullptr);
//     //assert(vulkan_data->vk_allocators != nullptr);

//    printf("vulkan: Creating window surface\n");
//     VkResult err = glfwCreateWindowSurface(vulkan_data->vk_inst, 
//                                         window->glfw_window,   
//                                         vulkan_data->vk_allocators, 
//                                         &window->vk_surface);
//     sigil::vulkan::check_result(err);

//    printf("vulkan: checking frame buffer size\n");
//     glfwGetFramebufferSize(window->glfw_window, &window->width, &window->height);

//     // Check for WSI support
//     VkBool32 res;
//     vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_data->phy_dev_registered.at(0),
//                                         vulkan_data->vk_main_q.family, window->vk_surface, &res);
//     if (res != VK_TRUE) {
//         fprintf(stderr, "vulkan: Error no WSI support main physical device\n");
//         std::exit(-1);
//     }

//     // int w, h;
//     // glfwGetFramebufferSize(app_data.main_window->glfw_window, &w, &h);

//     // app_data.imgui_wd.Surface = app_data.main_window->vk_surface;

//     // vkGetPhysicalDeviceSurfaceSupportKHR(app_data.vkhost->main_gpu(), app_data.vkhost->vk_main_q.family, app_data.imgui_wd.Surface, &res);
//     // if (res != VK_TRUE) {
//     //     fprintf(stderr, "Error no WSI support on physical device 0\n");
//     //     exit(-1);
//     // }

//     // // Select Surface Format
//     // const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//     // const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     // app_data.imgui_wd.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(app_data.vkhost->main_gpu(), app_data.imgui_wd.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);


//     vk_set_surface_format(window);
//     vk_set_presentation_mode(window, sigil::window_t::PRESENTATION_VSYNC_TRIPLE_BUFFERING);

//     return 0;
// }

// int sigil::vulkan::vk_force_create_window(sigil::window_t *window) {
//     VkResult err;
//     VkSwapchainKHR old_swapchain = window->vk_swapchain;
//     window->vk_swapchain = nullptr;

//     err = vkDeviceWaitIdle(vulkan_data->vk_dev);
//     sigil::vulkan::check_result(err);

//     for (uint32_t i = 0; i < window->image_count; i++)
//         destroy_frame(vulkan_data->vk_dev, &window->frames[i], vulkan_data->vk_allocators);
    
//     for (uint32_t i = 0; i < window->semaphore_count; i++)
//         destroy_frame_semaphores(vulkan_data->vk_dev, &window->frame_semaphores[i], vulkan_data->vk_allocators);


//     free (window->frames);
//     free (window->frame_semaphores);
//     window->frames = nullptr;
//     window->frame_semaphores =  nullptr;
//     window->image_count = 0;

//     if (window->vk_render_pass)
//         vkDestroyRenderPass(vulkan_data->vk_dev, window->vk_render_pass, vulkan_data->vk_allocators);

//     if (window->vk_pipeline)
//         vkDestroyPipeline(vulkan_data->vk_dev, window->vk_pipeline, vulkan_data->vk_allocators);

//     // If min image count was not specified, request different count of images dependent on selected present mode
//     if (window->min_image_count == 0) {}
//         //window->min_image_count = ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(window->vk_present_mode);

//     // Create Swapchain
//     {
//         VkSwapchainCreateInfoKHR info = {};
//         info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//         info.surface = window->vk_surface;
//         info.minImageCount = window->min_image_count;
//         info.imageFormat = window->vk_surface_format.format;
//         info.imageColorSpace = window->vk_surface_format.colorSpace;
//         info.imageArrayLayers = 1;
//         info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//         info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;           // Assume that graphics family == present family
//         info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
//         info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//         info.presentMode = window->vk_present_mode;
//         info.clipped = VK_TRUE;
//         info.oldSwapchain = old_swapchain;
//         VkSurfaceCapabilitiesKHR cap;
//         err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_data->main_gpu(), window->vk_surface, &cap);
//         vulkan::check_result(err);

//         if (info.minImageCount < cap.minImageCount) info.minImageCount = cap.minImageCount;

//         else if (cap.maxImageCount != 0 && info.minImageCount > cap.maxImageCount) {
//             info.minImageCount = cap.maxImageCount;
//         }

//         if (cap.currentExtent.width == 0xffffffff) {
//             info.imageExtent.width = window->width ; // = w;
//             info.imageExtent.height = window->height;// = h;
//         } else {
//             info.imageExtent.width = window->width = cap.currentExtent.width;
//             info.imageExtent.height = window->height = cap.currentExtent.height;
//         }

//         err = vkCreateSwapchainKHR(vulkan_data->vk_dev, &info, vulkan_data->vk_allocators, &window->vk_swapchain);
//         vulkan::check_result(err);
//         err = vkGetSwapchainImagesKHR(vulkan_data->vk_dev, window->vk_swapchain, &window->image_count, nullptr);
//         vulkan::check_result(err);
//         VkImage backbuffers[16] = {};

//         assert(window->image_count >= window->min_image_count);
//         //assert(window->image_count < IM_ARRAYSIZE(backbuffers));
//         err = vkGetSwapchainImagesKHR(vulkan_data->vk_dev, window->vk_swapchain, &window->image_count, backbuffers);
//         vulkan::check_result(err);

//         assert(window->frames == nullptr && window->frame_semaphores == nullptr);
        
//         window->semaphore_count = window->image_count + 1;
//         window->frames = (sigil::window_frame_t*)malloc(sizeof(sigil::window_frame_t) * window->image_count);
//         window->frame_semaphores = (sigil::window_frame_semaphores_t*)malloc(sizeof(sigil::window_frame_semaphores_t) * window->semaphore_count);
//         memset(window->frames, 0, sizeof(window->frames[0]) * window->image_count);
//         memset(window->frame_semaphores, 0, sizeof(window->frame_semaphores[0]) * window->semaphore_count);
//         for (uint32_t i = 0; i < window->image_count; i++)
//             window->frames[i].Backbuffer = backbuffers[i];
//     }
//     if (old_swapchain)
//         vkDestroySwapchainKHR(vulkan_data->vk_dev, old_swapchain, vulkan_data->vk_allocators);

//     // Create the Render Pass
//     if (window->use_dynamic_rendering == false)
//     {
//         VkAttachmentDescription attachment = {};
//         attachment.format = window->vk_surface_format.format;
//         attachment.samples = VK_SAMPLE_COUNT_1_BIT;
//         attachment.loadOp = window->clear_enable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//         attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//         attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//         attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//         attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//         VkAttachmentReference color_attachment = {};
//         color_attachment.attachment = 0;
//         color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         VkSubpassDescription subpass = {};
//         subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//         subpass.colorAttachmentCount = 1;
//         subpass.pColorAttachments = &color_attachment;
//         VkSubpassDependency dependency = {};
//         dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//         dependency.dstSubpass = 0;
//         dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         dependency.srcAccessMask = 0;
//         dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//         VkRenderPassCreateInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//         info.attachmentCount = 1;
//         info.pAttachments = &attachment;
//         info.subpassCount = 1;
//         info.pSubpasses = &subpass;
//         info.dependencyCount = 1;
//         info.pDependencies = &dependency;
//         err = vkCreateRenderPass(vulkan_data->vk_dev, &info, vulkan_data->vk_allocators, &window->vk_render_pass);
//         vulkan::check_result(err);

//         // We do not create a pipeline by default as this is also used by examples' main.cpp,
//         // but secondary viewport in multi-viewport mode may want to create one with:
//         //ImGui_ImplVulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE, wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, v->Subpass);
//     }

//     // Create The Image Views
//     {
//         VkImageViewCreateInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//         info.format = window->vk_surface_format.format;
//         info.components.r = VK_COMPONENT_SWIZZLE_R;
//         info.components.g = VK_COMPONENT_SWIZZLE_G;
//         info.components.b = VK_COMPONENT_SWIZZLE_B;
//         info.components.a = VK_COMPONENT_SWIZZLE_A;
//         VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
//         info.subresourceRange = image_range;
//         for (uint32_t i = 0; i < window->image_count; i++)
//         {
//             sigil::window_frame_t* fd = &window->frames[i];
//             info.image = fd->Backbuffer;
//             err = vkCreateImageView(vulkan_data->vk_dev, &info, vulkan_data->vk_allocators, &fd->BackbufferView);
//             vulkan::check_result(err);
//         }
//     }

//     // Create Framebuffer
//     if (window->use_dynamic_rendering == false)
//     {
//         VkImageView attachment[1];
//         VkFramebufferCreateInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//         info.renderPass = window->vk_render_pass;
//         info.attachmentCount = 1;
//         info.pAttachments = attachment;
//         info.width = window->width;
//         info.height = window->height;
//         info.layers = 1;
//         for (uint32_t i = 0; i < window->image_count; i++)
//         {
//             sigil::window_frame_t* fd = &window->frames[i];
//             attachment[0] = fd->BackbufferView;
//             err = vkCreateFramebuffer(vulkan_data->vk_dev, &info, vulkan_data->vk_allocators, &fd->Framebuffer);
//             vulkan::check_result(err);
//         }
//     }
//     //ImGui_ImplVulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE, wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, g_VulkanInitInfo.Subpass);
//     create_window_command_buffer(vulkan_data->main_gpu(), vulkan_data->vk_dev, window, vulkan_data->vk_main_q.family, vulkan_data->vk_allocators);
//     return 0;
// }

// // sigil::status_t vulkan::create_render_channel(sigil::window_t *window, sigil::scene_t *scene) {
// //     return sigil::VM_OK;
// // }



