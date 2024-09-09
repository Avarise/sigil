/*
    Multitool for SigilVM.
    Feature list:
    - (✓)  Initialize fakeroot, load all extensions
    - (☓)  Initialize deamon, probe for root
    - (☓)  Create archived file
    - (☓)  Use GUI mode

    Program flowchart:
        1. Check for special keywords: flushvm, legacy, dwm
        2. Initialize sigilvm
            - Additionally, load iocommon and memory modules
        3. Check for server startup
            - load net extensions if needed
        4. Check for gui startup
            - load render extensions if needed
            - use iocommon to spawn a window
            - use window methods to set properties
            - start gui thread
        5. Check for console startup
            - start console thread
        6. Join console, server, gui threads
        7. Deinit sigilvm
        8. exit and report the status.
*/
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <filesystem>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <thread>
#include <argp.h>
#include <vector>
#include "../extern/imgui/backends/imgui_impl_vulkan.h"
#include "../extern/imgui/backends/imgui_impl_glfw.h"
#include "../extern/imgui/imgui.h"
#include "../render/window.h"
#include "../render/vulkan.h"
#include "../render/visor.h"
#include "../utils/generic.h"
#include "../utils/text.h"
#include "../utils/log.h"
#include "../vm/iocommon.h"
#include "../vm/system.h"
#include "../vm/core.h"

#define APP_USE_VULKAN_DEBUG_REPORT
#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Global Variables
const char program_name[] = "SigilVM Tools";
sigil::vmnode_t *virtual_machine = nullptr;
sigil::window_t *main_window = nullptr;


// Visibility of various subwindows
static struct {
    bool asset_manager = true;
    bool style_editor = false;
    bool asset_editor = false;
    bool text_editor = true;
    bool overview = true;
    bool options = false;
    bool output = true;
    bool perf = false;
    bool demo = false;
} windows;

// Various
static void flush_vm() {
    printf("sigil-tools: Flushing SigilVM\n");
    sigil::system::invalidate_root();
}

static struct app_data_t {
    // SigilVM variables
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    sigil::vulkan::host_data_t *vkhost = nullptr;
    sigil::window_t *main_window = nullptr;
    sigil::memstat_t mem_usage;
    ImGui_ImplVulkanH_Window imgui_wd;
    sigil::vmnode_t *vmsr = nullptr;
    std::thread *gui = nullptr;
    uint32_t frames_drawn = 0;
    ImDrawData* imgui_draw_data = nullptr;


    // Basic tools variables
    std::string command_input;      // command to be executed via terminal
    std::string output_buffer;      // output combined for many source like terminal or debug
    char editor_content[1024 * 32];

    // GUI Subwindows default visibility
    struct {
        bool asset_manager = true;
        bool style_editor = false;
        bool asset_editor = false;
        bool text_editor = true;
        bool overview = true;
        bool options = false;
        bool output = true;
        bool perf = false;
        bool demo = false;
    } windows;
} app_data;

// GUI Elements rendering
static void sigil_tools_gui_style_manager();
static void sigil_tools_gui_asset_manager();
static void sigil_tools_gui_asset_editor();
static void sigil_tools_gui_text_editor();
static void sigil_tools_gui_overview();
static void sigil_tools_gui_options();
static void sigil_tools_gui_menubar();
static void sigil_tools_gui_output();
static void sigil_tools_gui_perf();
static void sigil_tools_gui_demo();
static void sigil_tools_main_menu();
static void sigil_tools_close_app();

// Setup for ImGui, and visor renderer
static sigil::status_t sigil_tools_gui_prepare();
// Use this as window content
static sigil::status_t sigil_tools_gui_main_loop();


void import_fonts(const std::string& fontDir, float fontSize, ImGuiIO& io);
static void sigil_tools_frame_present();
static void sigil_tools_frame_render();

// Main, main loop, init/deinit
int main(int argc, const char **argv) {
    sigil::argparser_t parser(argc, argv);
    sigil::status_t status = sigil::VM_OK;
    bool use_legacy_gui = false;
    VkResult res;

    // Quick actions, no need to spawn VM
    if (parser.is_set("--flushvm")) flush_vm();
    if (parser.is_set("--eoy")) sigil::log::tt_end_of_year();
    if (parser.is_set("--dwm")) {
        if ((status = sigil::iocommon::start_dwm()) != sigil::VM_OK) {
            printf("sigil-tools: failed to initialize dwm\n");
            sigil::exit(status);
        }
    }

    // Spawn VM, some additional parsing happens there, like --debug
    // Other argument parsing is left for program
    if ((status = sigil::system::initialize(argc, argv)) != sigil::VM_OK) {
        printf("sigil-tools: failed to initialize SigilVM\n");
        sigil::exit(status);
    }

    // Store pointer to the VM
    virtual_machine = sigil::system::probe_root();

    if (status != sigil::VM_OK) {
        printf("sigil-tools: failed to start CLI\n");
        sigil::exit(status);
    }

    if (parser.is_set("--gui")) {
        status = sigil::visor::initialize(virtual_machine);
        if (status != sigil::VM_OK) {
            printf("sigil-tools: failed to initialize Visor Renderer\n");
            sigil::exit(status);
        }

        status = sigil::vulkan::initialize(virtual_machine);
        if (status != sigil::VM_OK) {
            printf("sigil-tools: failed to initialize Vulkan\n");
            sigil::exit(status);
        }

        main_window = sigil::visor::spawn_window(program_name);
        if (main_window == nullptr) {
            printf("sigil-tools: could not acquire window\n");
            status = sigil::VM_FAILED_ALLOC;
            sigil::exit(status);
        }

        status = sigil_tools_gui_prepare();
        if (status != sigil::VM_OK) {
            printf("sigil-tools: failed to prepare sigil-tools gui\n");
            sigil::exit(status);
        }
        
        status = main_window->set_content(sigil_tools_gui_main_loop);
        if (status != sigil::VM_OK) {
            printf("sigil-tools: failed to set window content\n");
            sigil::exit(status);
        }

        status = main_window->deploy();
        if (status != sigil::VM_OK) {
            printf("sigil-tools: failed to deploy main window\n");
            sigil::exit(status);
        }
    }

    if (main_window) main_window->wait_for_shutdown();
    sigil::system::shutdown();
    sigil::exit(status);
}


namespace fs = std::filesystem;

void import_fonts(const std::string& fontDir, float fontSize, ImGuiIO& io) {
    try {
        std::vector<ImFont*> loadedFonts;

        // Iterate through the directory
        for (const auto& entry : fs::directory_iterator(fontDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ttf") {
                const std::string fontPath = entry.path().string();
                
                // Load the font
                ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);
                if (font) {
                    loadedFonts.push_back(font);
                    std::cout << "Loaded font: " << fontPath << std::endl;
                } else {
                    std::cerr << "Failed to load font: " << fontPath << std::endl;
                }
            }
        }

        if (loadedFonts.empty()) {
            std::cerr << "No fonts loaded from directory: " << fontDir << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

sigil::status_t sigil_tools_gui_main_loop() {
    ImGuiIO& io = ImGui::GetIO();

    while (!glfwWindowShouldClose(app_data.main_window->glfw_window)) {
        glfwPollEvents();

        
        if (!(app_data.frames_drawn % 250)) {
            app_data.output_buffer += sigil::log::get_current_time();
            sigil::utils::insert_into_string(app_data.output_buffer, ": frame %u\n", app_data.frames_drawn);
        }

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(app_data.main_window->glfw_window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (app_data.main_window->swapchain_rebuild || app_data.imgui_wd.Width != fb_width || app_data.imgui_wd.Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(app_data.main_window->min_image_count);
            ImGui_ImplVulkanH_CreateOrResizeWindow(app_data.vkhost->vk_inst, app_data.vkhost->main_gpu(), app_data.vkhost->vk_dev, &app_data.imgui_wd, app_data.vkhost->vk_main_q.family, app_data.vkhost->vk_allocators, fb_width, fb_height, app_data.main_window->min_image_count);
            app_data.imgui_wd.FrameIndex = 0;
            app_data.main_window->swapchain_rebuild = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        sigil_tools_main_menu();

        app_data.imgui_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (app_data.imgui_draw_data->DisplaySize.x <= 0.0f || app_data.imgui_draw_data->DisplaySize.y <= 0.0f);
        app_data.imgui_wd.ClearValue.color.float32[0] = app_data.clear_color.x * app_data.clear_color.w;
        app_data.imgui_wd.ClearValue.color.float32[1] = app_data.clear_color.y * app_data.clear_color.w;
        app_data.imgui_wd.ClearValue.color.float32[2] = app_data.clear_color.z * app_data.clear_color.w;
        app_data.imgui_wd.ClearValue.color.float32[3] = app_data.clear_color.w;
        
        if (!main_is_minimized) sigil_tools_frame_render();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        if (!main_is_minimized) sigil_tools_frame_present();

        app_data.frames_drawn++;
    }

    return sigil::VM_OK;
}

sigil::status_t sigil_tools_gui_prepare() {
    VkBool32 res;

    int w, h;
    glfwGetFramebufferSize(app_data.main_window->glfw_window, &w, &h);

    app_data.imgui_wd.Surface = app_data.main_window->vk_surface;

    vkGetPhysicalDeviceSurfaceSupportKHR(app_data.vkhost->main_gpu(), app_data.vkhost->vk_main_q.family, app_data.imgui_wd.Surface, &res);
    if (res != VK_TRUE) {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    app_data.imgui_wd.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(app_data.vkhost->main_gpu(), app_data.imgui_wd.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(app_data.main_window->min_image_count >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(app_data.vkhost->vk_inst, app_data.vkhost->main_gpu(), app_data.vkhost->vk_dev, &app_data.imgui_wd, app_data.vkhost->vk_main_q.family, app_data.vkhost->vk_allocators, w, h, app_data.main_window->min_image_count);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(app_data.main_window->glfw_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = app_data.vkhost->vk_inst;
    init_info.PhysicalDevice = app_data.vkhost->main_gpu();
    init_info.Device = app_data.vkhost->vk_dev;
    init_info.QueueFamily = app_data.vkhost->vk_main_q.family;
    init_info.Queue = app_data.vkhost->vk_main_q.queue;
    init_info.PipelineCache = app_data.vkhost->vk_pipeline_cache;
    init_info.DescriptorPool = app_data.vkhost->vk_descriptor_pool;
    init_info.RenderPass = app_data.imgui_wd.RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = app_data.main_window->min_image_count;
    init_info.ImageCount = app_data.imgui_wd.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = app_data.vkhost->vk_allocators;
    init_info.CheckVkResultFn = sigil::vulkan::check_result;
    
    ImGui_ImplVulkan_Init(&init_info);
    io.IniFilename = "/opt/sigil/assets/configs/imgui.ini";

    import_fonts("/opt/sigil/assets/fonts", 16.0f, io);

    // ImFont* font = io.Fonts->AddFontFromFileTTF("/opt/sigil/assets/fonts/Inter-VariableFont_slnt,wght.ttf", 16.0f);
    // if (font == nullptr) {
    //     return sigil::VM_FAILED_ALLOC;
    // }
    printf("Sigil-Tools: GUI prepared\n");
    return sigil::VM_OK;
}

static void sigil_tools_close_app() {
    //sigil::events::event_t *shutdown = sigil::events::peek(sigil::runtime::SHUTDOWN);
    //sigil::events::trigger(shutdown);
    exit(0);
}

// By frame updates
static void sigil_tools_frame_render() {
    VkResult err;

    VkSemaphore image_acquired_semaphore  = app_data.imgui_wd.FrameSemaphores[app_data.imgui_wd.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = app_data.imgui_wd.FrameSemaphores[app_data.imgui_wd.SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(app_data.vkhost->vk_dev, app_data.imgui_wd.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &app_data.imgui_wd.FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        app_data.main_window->swapchain_rebuild = true;
        return;
    }
    sigil::vulkan::check_result(err);

    ImGui_ImplVulkanH_Frame* fd = &app_data.imgui_wd.Frames[app_data.imgui_wd.FrameIndex];
    {
        err = vkWaitForFences(app_data.vkhost->vk_dev, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        sigil::vulkan::check_result(err);

        err = vkResetFences(app_data.vkhost->vk_dev, 1, &fd->Fence);
        sigil::vulkan::check_result(err);
    }
    {
        err = vkResetCommandPool(app_data.vkhost->vk_dev, fd->CommandPool, 0);
        sigil::vulkan::check_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        sigil::vulkan::check_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = app_data.imgui_wd.RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = app_data.imgui_wd.Width;
        info.renderArea.extent.height = app_data.imgui_wd.Height;
        info.clearValueCount = 1;
        info.pClearValues = &app_data.imgui_wd.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(app_data.imgui_draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        sigil::vulkan::check_result(err);
        err = vkQueueSubmit(app_data.vkhost->vk_main_q.queue, 1, &info, fd->Fence);
        sigil::vulkan::check_result(err);
    }
}

static void sigil_tools_frame_present() {
    if (app_data.main_window->swapchain_rebuild)
        return;
    VkSemaphore render_complete_semaphore = app_data.imgui_wd.FrameSemaphores[app_data.imgui_wd.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &app_data.imgui_wd.Swapchain;
    info.pImageIndices = &app_data.imgui_wd.FrameIndex;
    VkResult err = vkQueuePresentKHR(app_data.vkhost->vk_main_q.queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        app_data.main_window->swapchain_rebuild = true;
        return;
    }
    sigil::vulkan::check_result(err);
    app_data.imgui_wd.SemaphoreIndex = (app_data.imgui_wd.SemaphoreIndex + 1) % app_data.imgui_wd.SemaphoreCount; 
}

// Button procedures
static void sigil_button_new_project() {

}

static void open_project_popup() {

}

static void save_project_popup() {

}

static void save_project_as_popup() {

}

static void export_file() {

}

static void import_file() {
    
}

// GUI Elements
static void sigil_tools_main_menu() {
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::DockSpaceOverViewport();
    sigil_tools_gui_menubar();
    if (app_data.windows.asset_manager) sigil_tools_gui_asset_manager();
    if (app_data.windows.style_editor) sigil_tools_gui_style_manager();
    if (app_data.windows.asset_editor) sigil_tools_gui_asset_editor();
    if (app_data.windows.text_editor) sigil_tools_gui_text_editor();
    if (app_data.windows.overview) sigil_tools_gui_overview();
    if (app_data.windows.options) sigil_tools_gui_options();
    if (app_data.windows.output) sigil_tools_gui_output();
    if (app_data.windows.demo) sigil_tools_gui_demo();
    if (app_data.windows.perf) sigil_tools_gui_perf();
    ImGui::Render();
}

static void sigil_tools_gui_menubar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New project")) sigil_button_new_project();
            if (ImGui::MenuItem("Open project", "Ctrl+O")) open_project_popup();
            if (ImGui::MenuItem("Save project", "Ctrl+S")) save_project_popup();
            if (ImGui::MenuItem("Save project as..")) save_project_as_popup();
            ImGui::Separator();
            if (ImGui::MenuItem("Export file")) export_file();
            if (ImGui::MenuItem("Import file")) import_file();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) sigil_tools_close_app();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo")) {}
            if (ImGui::MenuItem("Find", "Ctrl+F")) {}
            if (ImGui::MenuItem("Find and replace", "Ctrl+F")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Text Editor", "", &app_data.windows.text_editor);
            ImGui::Separator();
            ImGui::MenuItem("Asset Manager", "Shift+A", &app_data.windows.asset_manager);
            ImGui::MenuItem("Asset Editor", "Shift+A", &app_data.windows.asset_editor);
            ImGui::Separator();
            //ImGui::MenuItem("Spellslot Tracker", NULL, &app_data.windows.show_spellslot_tracker);
            //ImGui::MenuItem("Dice Roller", NULL, &app_data.windows.show_dice_roller);
            ImGui::MenuItem("ImGui Demo", NULL ,&app_data.windows.demo);
            ImGui::MenuItem("Performance", NULL, &app_data.windows.perf);
            ImGui::MenuItem("Output window", "Ctrl + O", &app_data.windows.output);
            ImGui::MenuItem("Overview", NULL, &app_data.windows.overview);
            ImGui::MenuItem("Style Editor", NULL, &app_data.windows.style_editor);
            if (ImGui::MenuItem("Options", NULL, &app_data.windows.options)) {

            }          
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About Sigil")) {}
            if (ImGui::MenuItem("Shortcuts")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

static void sigil_tools_gui_perf() {
    ImGuiIO &io = ImGui::GetIO();
    sigil::get_memstats(app_data.mem_usage);
    ImGui::Begin("Performance", &app_data.windows.perf);

    ImGui::Text("Frame time: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Frames drawn: %lu", (uint64_t)app_data.frames_drawn);
    ImGui::Text("Program size (Pages/MB): %lu/%luMB", app_data.mem_usage.size, app_data.mem_usage.size >> 8 );
    ImGui::Text("Text size (Pages/MB): %lu/%luMB", app_data.mem_usage.text, app_data.mem_usage.text >> 8);
    ImGui::Text("Shared memory (Pages/MB): %lu/%luMB", app_data.mem_usage.share, app_data.mem_usage.share >> 8);
    ImGui::Text("Data memory usage (Pages/MB): %lu/%luMB", app_data.mem_usage.data, app_data.mem_usage.data >> 8);
    ImGui::Text("Resident memory usage (Pages/MB): %lu/%luMB", app_data.mem_usage.resident, app_data.mem_usage.resident >> 8);

    /*
        1 PAGE = 4KB

        (num pages * 4) / 1024  
    */

    ImGui::End();
}

static void sigil_tools_gui_output() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Output");
    // Make the text field read-only and scrollable
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(app_data.output_buffer.c_str());

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
    
    ImGui::EndChild();
    ImGui::End();
}

static void sigil_tools_gui_asset_manager() {
    ImGui::Begin("Asset Manager");

    if (ImGui::CollapsingHeader("Modules")) {
        if (ImGui::CollapsingHeader("Asset1")) {
            //nodetreeinfo(app_data.vmsr);
        }
    }

    if (ImGui::CollapsingHeader("Events")) {
    }

    if (ImGui::CollapsingHeader("Hardware")) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(app_data.vkhost->main_gpu(), &properties);
        ImGui::Text("Main GPU: %s\n", properties.deviceName);
    }

    if (ImGui::CollapsingHeader("Scenes")) {
    }

    ImGui::End();


}

static void sigil_tools_gui_overview() {
    ImGui::Begin("Overview");
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(0, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();
}

static void sigil_tools_gui_asset_editor() {
    ImGui::Begin("Asset Editor");
    ImGui::Text("Asset editor not yet available");
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(0, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();
}

static void sigil_tools_gui_text_editor() {
    ImGui::Begin("Text Editor");
    //ImGui::InputTextMultiline("##EditorContent", app_data.editor_content, ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput);

    ImGui::End();
}

static void sigil_tools_gui_style_manager() {
    ImGui::Begin("GUI Settings", &app_data.windows.style_editor);
    ImGui::ShowStyleEditor();
    ImGui::End();
}

static void sigil_tools_gui_options() {

}

static void sigil_tools_gui_demo() {
    ImGui::ShowDemoWindow(&app_data.windows.demo);
}