#include <vulkan.h>
#include "graphics.h"
#include "utils.h"

static bool glfw_initialized = false;


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


sigil::graphics::dynamic_mesh_t::dynamic_mesh_t() {
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

sigil::graphics::static_mesh_t::static_mesh_t() {
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    verteces = {};
}

void sigil::graphics::static_mesh_t::randomize() {
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

// Default constructor
// Set basics of a window, assuming Vulkan as default.
sigil::graphics::window_t::window_t() {
    memset((void*)this, 0, sizeof(*this));
    vk_present_mode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
    clear_enable = true;
    this->theme = DARKMODE;
}

sigil::graphics::window_t::~window_t() {

    //ImGui_ImplVulkanH_DestroyWindow(vulkan_data->vk_inst, app_data.vkhost->vk_dev, &app_data.imgui_wd, app_data.vkhost->vk_allocators);

    glfwDestroyWindow(this->glfw_window);
}

sigil::status_t sigil::graphics::window_t::refresh_properties() {
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

sigil::status_t sigil::graphics::window_t::set_content(sigil::status_t (*gui_routine)(void)) {
    this->gui_routine = gui_routine;
    return sigil::VM_OK;
}

std::thread* sigil::graphics::window_t::deploy() {
    return this->thread;
}

void sigil::graphics::window_t::wait_for_shutdown() {

}

sigil::status_t sigil::graphics::initialize_glfw() {
    if (glfw_initialized) return sigil::VM_ALREADY_EXISTS;
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        printf("iocommon: failed glfw init\n");
        glfw_initialized = false;
        return sigil::VM_FAILED;
    }

    glfw_initialized = true;
    return sigil::VM_OK;
}