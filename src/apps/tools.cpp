/*
    sigil-tools
    Config utility, and launcher for SigilVM suite.
    
    It uses a tree of sigil::node_t, and starts other apps via threading. 
    exit_sigil_tools() will await for all started threads, and delete VM
    gracefully

    Subcommands:
    --console:
        uses CLI to execute various commands, most useful for manual control
        to start multiple apps.
    --gui:
        start ImGui based visual interface
    --dwm:
        spawn x11 dwm
    --countdown
        Show countdown until end of the year
    --flush-vm
        Clear tempdata and cache of SigilVM
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
#include "../sigil.h"

#define APP_USE_VULKAN_DEBUG_REPORT
#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Sigil Tools subcommands
static void             subcommand_flush_vm();
static void             subcommand_console();
static void             subcommand_roll();
static void             subcommand_gui();
static void             subcommand_dwm();

// Sigil Tools GUI subwindows
static void             subwindow_style_manager();
static void             subwindow_asset_manager();
static void             subwindow_asset_editor();
static void             subwindow_text_editor();
static void             subwindow_overview();
static void             subwindow_options();
static void             subwindow_menubar();
static void             subwindow_output();
static void             subwindow_perf();
static void             subwindow_demo();

// Popups
static void             popup_open_project();
static void             pupup_save_project();
static void             pupup_exit_project();
static void             popup_new_project();
static void             popup_import_file();
static void             popup_export_file();

// Procedure sent to visor to be threaded
static sigil::status_t  gui_main_loop();
static void             gui_prepare_frame(bool is_window_minimized);
static void             import_fonts(const std::string& fontDir, float fontSize, ImGuiIO& io);

// Clean exit with all subprocedures wrapped
static void             exit_sigil_tools(sigil::status_t status);

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
} subwindows;

// Subwindows helper data
std::string command_input;      // command to be executed via terminal
std::string output_buffer;      // output combined for many source like terminal or debug

static struct {
    std::thread *dwm;
    std::thread *console;
    std::thread *gui;
} tools_threads {0};

static struct text_editor_t {
    char editor_content[1024 * 32] = {0};
} text_editor;


// SigilVM and auxiliary variable
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
sigil::virtual_machine_t *virtual_machine = nullptr;
const char program_name[] = "SigilVM Tools";
sigil::status_t app_status = sigil::VM_OK;
sigil::window_t *main_window = nullptr;
sigil::memstat_t mem_usage;
uint32_t frames_presented = 0;
uint32_t frames_processed = 0;
uint32_t frames_prepared = 0;

// Main, main loop, init/deinit
int main(int argc, const char **argv) {
    sigil::argparser_t parser(argc, argv);

    // First check for subcommands that don't spawn VM
    if (parser.is_set("--flushvm"))
        subcommand_flush_vm();

    if (parser.is_set("--countdown"))
        sigil::log::tt_end_of_year();

    if (parser.is_set("--dwm"))
        subcommand_dwm();

    if (parser.is_set("--gui"))
        subcommand_gui();

    if (parser.is_set("--console"))
        subcommand_console();

    exit_sigil_tools(app_status);
}

static void exit_sigil_tools(sigil::status_t st) {
    //sigil::events::event_t *shutdown = sigil::events::peek(sigil::runtime::SHUTDOWN);
    //sigil::events::trigger(shutdown);
    virtual_machine->deinitialize();
    delete virtual_machine;
    virtual_machine = nullptr;
    printf("sigil-tools: exiting -> %s\n", sigil::status_t_cstr(st));
    exit(0);
}

void subcommand_flush_vm() {
    printf("sigil-tools: Flushing SigilVM\n");
    app_status = sigil::system::invalidate_root();
    exit_sigil_tools(app_status);
}

void subcommand_console() {
    tools_threads.console = new std::thread(sigil::console_subprogram);

    // Not usable since new throws, but keeping it for future
    if (tools_threads.console == nullptr) 
        exit_sigil_tools(sigil::VM_FAILED_ALLOC);
}

void subcommand_dwm() {
    // TODO: Change from blocking model to threaded model
    // use tools_threads.dwm

    printf("Starting dwm\n");
    std::system("startx /opt/sigil/src/scripts/dwm.sh");
}

void subcommand_gui() {
    if (virtual_machine == nullptr) exit_sigil_tools(sigil::VM_INVALID_ROOT);

    // First start needed APIs
    if ((app_status = virtual_machine->start_api_visor()) != sigil::VM_OK) {
        printf("sigil-tools: failed to initialize Visor Renderer\n");
        exit_sigil_tools(app_status);
    }

    if ((app_status = virtual_machine->start_api_vulkan()) != sigil::VM_OK) {
        printf("sigil-tools: failed to initialize Vulkan\n");
        exit_sigil_tools(app_status);
    }

    // TODO: Prepare ImGui here, to minimize abstraction within modules
    // Load fonts

    main_window = sigil::visor::spawn_window(program_name);
    if (main_window == nullptr) {
        printf("sigil-tools: could not acquire window\n");
        app_status = sigil::VM_FAILED_ALLOC;
        exit_sigil_tools(app_status);
    }

    app_status = sigil::visor::prepare_for_imgui(main_window);
    if (app_status != sigil::VM_OK) {
        printf("sigil-tools: failed to prepare ImGui\n");
        exit_sigil_tools(app_status);
    }

    app_status = main_window->set_content(gui_main_loop);
    if (app_status != sigil::VM_OK) {
        printf("sigil-tools: failed to set window content\n");
        exit_sigil_tools(app_status);
    }

    // Spawn gui thread
    tools_threads.gui = main_window->deploy();
    if (tools_threads.gui == nullptr) {
        printf("sigil-tools: failed to prepare sigil-tools gui\n");
        exit_sigil_tools(sigil::VM_FAILED_ALLOC);
    }
}

sigil::status_t gui_main_loop() {
    ImGuiIO& io = ImGui::GetIO();
    sigil::utils::exec_timer gui_main_loop_timer;

    while (!glfwWindowShouldClose(main_window->glfw_window)) {
        gui_main_loop_timer.start();

        // glfw events first
        glfwPollEvents();

        // resize swap chain and determine visibility
        sigil::visor::check_for_swapchain_update(main_window);
        auto imgui_draw_data = ImGui::GetDrawData();
        const bool is_minimized = (imgui_draw_data->DisplaySize.x <= 0.0f || imgui_draw_data->DisplaySize.y <= 0.0f);

        gui_prepare_frame(is_minimized);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        if (!is_minimized) {
            frames_presented++;
            sigil::visor::present_frame(main_window);
        }

        frames_processed++;
        gui_main_loop_timer.stop();

        if (!(frames_processed % 515)) {
            output_buffer += sigil::log::get_current_time();
            sigil::utils::insert_into_string(output_buffer,
                ":spent %lu microseconds rendering last frame [ %u/%u/%u  Processed/Prepared/Presented] #%u\n",
                gui_main_loop_timer.us(), frames_processed, frames_prepared, frames_presented);
        }
    }

    return sigil::VM_OK;
}

// Using Visor, and ImGui to arrange a new frame for rendering
// Don't use Vulkan directly here
static void gui_prepare_frame(bool is_window_minimized) {
    frames_prepared++;
    main_window->imgui_wd.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    main_window->imgui_wd.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    main_window->imgui_wd.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    main_window->imgui_wd.ClearValue.color.float32[3] = clear_color.w;

    // Visor first
    app_status = sigil::visor::new_frame(main_window);
    if (app_status != sigil::VM_OK) exit_sigil_tools(app_status);

    // Imgui second
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();
    
    // Main window composition
    ImGui::DockSpaceOverViewport();
    
    // Always draw menubar
    subwindow_menubar();
    
    // Conditional subwindows
    if (subwindows.asset_manager) subwindow_asset_manager();
    if (subwindows.style_editor) subwindow_style_manager();
    if (subwindows.asset_editor) subwindow_asset_editor();
    if (subwindows.text_editor) subwindow_text_editor();
    if (subwindows.overview) subwindow_overview();
    if (subwindows.options) subwindow_options();
    if (subwindows.output) subwindow_output();
    if (subwindows.demo) subwindow_demo();
    if (subwindows.perf) subwindow_perf();

    // Convert ImGui elements into framedata for visor
    ImGui::Render();
}

// Popups
void popup_new_project() {

}

void popup_open_project() {

}

void popup_save_project() {

}

void popup_export_file() {

}

void popup_import_file() {

}

// Subwindows
void subwindow_menubar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New project")) popup_new_project();
            if (ImGui::MenuItem("Open project", "Ctrl+O")) popup_open_project();
            if (ImGui::MenuItem("Save project", "Ctrl+S")) popup_save_project();
            if (ImGui::MenuItem("Save project as..")) popup_save_project();
            ImGui::Separator();
            if (ImGui::MenuItem("Export file")) popup_export_file();
            if (ImGui::MenuItem("Import file")) popup_import_file();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) exit_sigil_tools(app_status);
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
            ImGui::MenuItem("Text Editor", "", &subwindows.text_editor);
            ImGui::Separator();
            ImGui::MenuItem("Asset Manager", "Shift+A", &subwindows.asset_manager);
            ImGui::MenuItem("Asset Editor", "Shift+A", &subwindows.asset_editor);
            ImGui::Separator();
            //ImGui::MenuItem("Spellslot Tracker", NULL, &windows.show_spellslot_tracker);
            //ImGui::MenuItem("Dice Roller", NULL, &windows.show_dice_roller);
            ImGui::MenuItem("ImGui Demo", NULL ,&subwindows.demo);
            ImGui::MenuItem("Performance", NULL, &subwindows.perf);
            ImGui::MenuItem("Output window", "Ctrl + O", &subwindows.output);
            ImGui::MenuItem("Overview", NULL, &subwindows.overview);
            ImGui::MenuItem("Style Editor", NULL, &subwindows.style_editor);
            if (ImGui::MenuItem("Options", NULL, &subwindows.options)) {

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

void subwindow_perf() {
    ImGuiIO &io = ImGui::GetIO();
    sigil::get_memstats(mem_usage);
    ImGui::Begin("Performance", &subwindows.perf);

    ImGui::Text("Frame time: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Frames drawn: %lu", (uint64_t)frames_processed);
    ImGui::Text("Program size (Pages/MB): %lu/%luMB", mem_usage.size, mem_usage.size >> 8 );
    ImGui::Text("Text size (Pages/MB): %lu/%luMB", mem_usage.text, mem_usage.text >> 8);
    ImGui::Text("Shared memory (Pages/MB): %lu/%luMB", mem_usage.share, mem_usage.share >> 8);
    ImGui::Text("Data memory usage (Pages/MB): %lu/%luMB", mem_usage.data, mem_usage.data >> 8);
    ImGui::Text("Resident memory usage (Pages/MB): %lu/%luMB", mem_usage.resident, mem_usage.resident >> 8);

    /*
        1 PAGE = 4KB

        (num pages * 4) / 1024  
    */

    ImGui::End();
}

void subwindow_output() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Begin("Output");
    // Make the text field read-only and scrollable
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(output_buffer.c_str());

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
    
    ImGui::EndChild();
    ImGui::End();
}

void subwindow_asset_manager() {
    ImGui::Begin("Asset Manager");

    if (ImGui::CollapsingHeader("Modules")) {
        if (ImGui::CollapsingHeader("Asset1")) {
            //nodetreeinfo(vmsr);
        }
    }

    if (ImGui::CollapsingHeader("Events")) {
    }

    if (ImGui::CollapsingHeader("Hardware")) {
        // TODO: Rework subnode search, to easily get references to needed apis
        //virtual_machine->get_subnode("vulkan", 4);
        // TODO: Get list of items from sigil::vulkan
        // Use virtual_machine-> 
        //VkPhysicalDeviceProperties properties;
        //vkGetPhysicalDeviceProperties(vkhost->main_gpu(), &properties);
        //ImGui::Text("Main GPU: %s\n", properties.deviceName);
    }

    if (ImGui::CollapsingHeader("Scenes")) {
    }

    ImGui::End();


}

static void subwindow_overview() {
    ImGui::Begin("Overview");
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(0, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();
}

static void subwindow_asset_editor() {
    ImGui::Begin("Asset Editor");
    ImGui::Text("Asset editor not yet available");
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    ImGui::Image(0, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();
}

static void subwindow_text_editor() {
    ImGui::Begin("Text Editor");
    //ImGui::InputTextMultiline("##EditorContent", editor_content, ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput);

    ImGui::End();
}

static void subwindow_style_manager() {
    ImGui::Begin("GUI Settings", &subwindows.style_editor);
    ImGui::ShowStyleEditor();
    ImGui::End();
}

static void subwindow_options() {

}

static void subwindow_demo() {
    ImGui::ShowDemoWindow(&subwindows.demo);
}


void import_fonts(const std::string& fontDir, float fontSize, ImGuiIO& io) {
    namespace fs = std::filesystem;
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



// sigil::status_t subwindow_prepare() {
//     VkBool32 res;

//     int w, h;
//     glfwGetFramebufferSize(main_window->glfw_window, &w, &h);

//     imgui_wd.Surface = main_window->vk_surface;

//     vkGetPhysicalDeviceSurfaceSupportKHR(vkhost->main_gpu(), vkhost->vk_main_q.family, imgui_wd.Surface, &res);
//     if (res != VK_TRUE) {
//         fprintf(stderr, "Error no WSI support on physical device 0\n");
//         exit(-1);
//     }

//     // Select Surface Format
//     const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//     const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     imgui_wd.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(vkhost->main_gpu(), imgui_wd.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

//     // Create SwapChain, RenderPass, Framebuffer, etc.
//     IM_ASSERT(main_window->min_image_count >= 2);
//     ImGui_ImplVulkanH_CreateOrResizeWindow(vkhost->vk_inst, vkhost->main_gpu(), vkhost->vk_dev, &imgui_wd, vkhost->vk_main_q.family, vkhost->vk_allocators, w, h, main_window->min_image_count);

//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO(); (void)io;
//     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//     io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
//     io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

//     ImGui::StyleColorsDark();
//     ImGuiStyle& style = ImGui::GetStyle();
//     if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
//         style.WindowRounding = 0.0f;
//         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
//     }

//     // Setup Platform/Renderer backends
//     ImGui_ImplGlfw_InitForVulkan(main_window->glfw_window, true);
//     ImGui_ImplVulkan_InitInfo init_info = {};
//     init_info.Instance = vkhost->vk_inst;
//     init_info.PhysicalDevice = vkhost->main_gpu();
//     init_info.Device = vkhost->vk_dev;
//     init_info.QueueFamily = vkhost->vk_main_q.family;
//     init_info.Queue = vkhost->vk_main_q.queue;
//     init_info.PipelineCache = vkhost->vk_pipeline_cache;
//     init_info.DescriptorPool = vkhost->vk_descriptor_pool;
//     init_info.RenderPass = imgui_wd.RenderPass;
//     init_info.Subpass = 0;
//     init_info.MinImageCount = main_window->min_image_count;
//     init_info.ImageCount = imgui_wd.ImageCount;
//     init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//     init_info.Allocator = vkhost->vk_allocators;
//     init_info.CheckVkResultFn = sigil::vulkan::check_result;
    
//     ImGui_ImplVulkan_Init(&init_info);
//     io.IniFilename = "/opt/sigil/assets/configs/imgui.ini";

//     import_fonts("/opt/sigil/assets/fonts", 16.0f, io);

//     // ImFont* font = io.Fonts->AddFontFromFileTTF("/opt/sigil/assets/fonts/Inter-VariableFont_slnt,wght.ttf", 16.0f);
//     // if (font == nullptr) {
//     //     return sigil::VM_FAILED_ALLOC;
//     // }
//     printf("Sigil-Tools: GUI prepared\n");
//     return sigil::VM_OK;
// }



// TODO: replace with prepare frame
// static void sigil_tools_frame_render() {
//     VkResult err;

//     VkSemaphore image_acquired_semaphore  = imgui_wd.FrameSemaphores[imgui_wd.SemaphoreIndex].ImageAcquiredSemaphore;
//     VkSemaphore render_complete_semaphore = imgui_wd.FrameSemaphores[imgui_wd.SemaphoreIndex].RenderCompleteSemaphore;
//     err = vkAcquireNextImageKHR(vkhost->vk_dev, imgui_wd.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &imgui_wd.FrameIndex);
//     if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
//         main_window->swapchain_rebuild = true;
//         return;
//     }
//     sigil::vulkan::check_result(err);

//     ImGui_ImplVulkanH_Frame* fd = &imgui_wd.Frames[imgui_wd.FrameIndex];
//     {
//         err = vkWaitForFences(vkhost->vk_dev, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
//         sigil::vulkan::check_result(err);

//         err = vkResetFences(vkhost->vk_dev, 1, &fd->Fence);
//         sigil::vulkan::check_result(err);
//     }
//     {
//         err = vkResetCommandPool(vkhost->vk_dev, fd->CommandPool, 0);
//         sigil::vulkan::check_result(err);
//         VkCommandBufferBeginInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//         err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
//         sigil::vulkan::check_result(err);
//     }
//     {
//         VkRenderPassBeginInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//         info.renderPass = imgui_wd.RenderPass;
//         info.framebuffer = fd->Framebuffer;
//         info.renderArea.extent.width = imgui_wd.Width;
//         info.renderArea.extent.height = imgui_wd.Height;
//         info.clearValueCount = 1;
//         info.pClearValues = &imgui_wd.ClearValue;
//         vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
//     }

//     // Record dear imgui primitives into command buffer
//     ImGui_ImplVulkan_RenderDrawData(imgui_draw_data, fd->CommandBuffer);

//     // Submit command buffer
//     vkCmdEndRenderPass(fd->CommandBuffer);
//     {
//         VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         VkSubmitInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         info.waitSemaphoreCount = 1;
//         info.pWaitSemaphores = &image_acquired_semaphore;
//         info.pWaitDstStageMask = &wait_stage;
//         info.commandBufferCount = 1;
//         info.pCommandBuffers = &fd->CommandBuffer;
//         info.signalSemaphoreCount = 1;
//         info.pSignalSemaphores = &render_complete_semaphore;

//         err = vkEndCommandBuffer(fd->CommandBuffer);
//         sigil::vulkan::check_result(err);
//         err = vkQueueSubmit(vkhost->vk_main_q.queue, 1, &info, fd->Fence);
//         sigil::vulkan::check_result(err);
//     }
// }
