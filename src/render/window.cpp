#include "window.h"
#include "../vm/core.h"

// Default constructor
// Set basics of a window, assuming Vulkan as default.
sigil::window_t::window_t() {
    memset((void*)this, 0, sizeof(*this));
    vk_present_mode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
    clear_enable = true;
    this->theme = DARKMODE;
}

sigil::window_t::~window_t() {

    //ImGui_ImplVulkanH_DestroyWindow(vulkan_data->vk_inst, app_data.vkhost->vk_dev, &app_data.imgui_wd, app_data.vkhost->vk_allocators);

    glfwDestroyWindow(this->glfw_window);
}

sigil::status_t sigil::window_t::refresh_properties() {
    int fb_width, fb_height;
    
    glfwGetFramebufferSize(this->glfw_window, &fb_width, &fb_height);

    bool fb_positive_size = fb_width > 0 && fb_height > 0;
    bool fb_size_changed = fb_width != this->width || fb_height != this->height;

    if (fb_positive_size && (this->swapchain_rebuild || fb_size_changed)) {
        //ImGui_ImplVulkan_SetMinImageCount(this->min_image_count);
        //ImGui_ImplVulkanH_CreateOrResizeWindow(app_data.vkhost_data->vk_inst, app_data.vkhost_data->main_gpu(), app_data.vkhost_data->vk_dev, &g_MainWindowData, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount);

        this->frame_index = 0;
        this->swapchain_rebuild = 0;
    }

    return sigil::VM_OK;
}

sigil::status_t sigil::window_t::set_content(sigil::status_t (*gui_routine)(void)) {
    this->gui_routine = gui_routine;
    return sigil::VM_OK;
}

std::thread* sigil::window_t::deploy() {
    return this->thread;
}

void sigil::window_t::wait_for_shutdown() {

}