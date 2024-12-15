#include <cstdio>
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "virtual-machine.h"
#include "GLFW/glfw3.h"
#include "system.h"
#include "utils.h"
#include "visor.h"

std::vector<sigil::visor::render_channel_t> render_channels = {};
std::vector<sigil::graphics::window_t*> windows = {};
static sigil::vmnode_t *visor_node = nullptr;

sigil::status_t sigil::visor::prepare_for_new_frame(sigil::graphics::window_t *window) {
    // glfw events first
    glfwPollEvents();

    // resize swap chain and determine visibility
    // sigil::visor::check_for_swapchain_update(window);

    auto imgui_draw_data = ImGui::GetDrawData();
    const bool is_minimized = (imgui_draw_data->DisplaySize.x <= 0.0f || imgui_draw_data->DisplaySize.y <= 0.0f);

    return VM_OK;
}

sigil::status_t sigil::visor::finalize_new_frame(sigil::graphics::window_t *window) {
    
    // Convert ImGui elements into framedata for visor, end preparation
    // ImGui::Render();
    // if (!is_window_minimized)
    //     sigil::visor::prepare_imgui_drawdata(main_window);
    
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }

    // if (!is_window_minimized)
    //     sigil::visor::present_frame(main_window);

    // if (!is_window_minimized)
    //     frames_presented++;
    return VM_OK;
}

// sigil::status_t sigil::visor::prepare_imgui_drawdata(window_t *window) {
//     VkResult err;

//     VkSemaphore image_acquired_semaphore  = window->imgui_wd.FrameSemaphores[window->imgui_wd.SemaphoreIndex].ImageAcquiredSemaphore;
//     VkSemaphore render_complete_semaphore = window->imgui_wd.FrameSemaphores[window->imgui_wd.SemaphoreIndex].RenderCompleteSemaphore;
//     err = vkAcquireNextImageKHR(vkhost->vk_dev, imgui_wd.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &imgui_wd.FrameIndex);
//     if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
//         window->swapchain_rebuild = true;
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

// sigil::status_t sigil::visor::present_frame(window_t *window) {
//     if (window->swapchain_rebuild) return SWAPCHAIN_REBUILDING;
//     VkSemaphore render_complete_semaphore = window->imgui_wd.FrameSemaphores[window->imgui_wd.SemaphoreIndex].RenderCompleteSemaphore;
//     VkPresentInfoKHR info = {};
//     info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//     info.waitSemaphoreCount = 1;
//     info.pWaitSemaphores = &render_complete_semaphore;
//     info.swapchainCount = 1;
//     info.pSwapchains = &window->imgui_wd.Swapchain;
//     info.pImageIndices = &window->imgui_wd.FrameIndex;
//     VkResult err = vkQueuePresentKHR(vkhost->vk_main_q.queue, &info);
//     if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
//     {
//         window->swapchain_rebuild = true;
//         return;
//     }
//     sigil::vulkan::check_result(err);
//     window->imgui_wd.SemaphoreIndex = (window->imgui_wd.SemaphoreIndex + 1) % window->imgui_wd.SemaphoreCount; 
// }

// sigil::status_t sigil::visor::prepare_for_imgui(sigil::window_t *window) {
//     VkBool32 res;

//     int w, h;
//     glfwGetFramebufferSize(window->glfw_window, &w, &h);

//     window->imgui_wd.Surface = window->vk_surface;

//     vkGetPhysicalDeviceSurfaceSupportKHR(vkhost->main_gpu(), vkhost->vk_main_q.family, imgui_wd.Surface, &res);
//     if (res != VK_TRUE) {
//         fprintf(stderr, "Error no WSI support on physical device 0\n");
//         exit(-1);
//     }

//     // Select Surface Format
//     const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//     const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     window->imgui_wd.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(vkhost->main_gpu(), imgui_wd.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

//     // Create SwapChain, RenderPass, Framebuffer, etc.
//     IM_ASSERT(window->min_image_count >= 2);
//     ImGui_ImplVulkanH_CreateOrResizeWindow(vkhost->vk_inst, vkhost->main_gpu(), vkhost->vk_dev, &imgui_wd, vkhost->vk_main_q.family, vkhost->vk_allocators, w, h, window->min_image_count);

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
//     ImGui_ImplGlfw_InitForVulkan(window->glfw_window, true);
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
//     init_info.MinImageCount = window->min_image_count;
//     init_info.ImageCount = imgui_wd.ImageCount;
//     init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//     init_info.Allocator = vkhost->vk_allocators;
//     init_info.CheckVkResultFn = sigil::vulkan::check_result;
    
//     ImGui_ImplVulkan_Init(&init_info);
//     io.IniFilename = "/sigil/vm/assets/configs/imgui.ini";

//     import_fonts("/sigil/vm/assets/fonts", 16.0f, io);

//     // ImFont* font = io.Fonts->AddFontFromFileTTF("/sigil/vm/assets/fonts/Inter-VariableFont_slnt,wght.ttf", 16.0f);
//     // if (font == nullptr) {
//     //     return sigil::VM_FAILED_ALLOC;
//     // }
//     printf("Sigil-Tools: GUI prepared\n");
//     return sigil::VM_OK;
// }

// void import_fonts(const std::string& fontDir, float fontSize, ImGuiIO& io) {
//     namespace fs = std::filesystem;
//     try {
//         std::vector<ImFont*> loadedFonts;

//         // Iterate through the directory
//         for (const auto& entry : fs::directory_iterator(fontDir)) {
//             if (entry.is_regular_file() && entry.path().extension() == ".ttf") {
//                 const std::string fontPath = entry.path().string();
                
//                 // Load the font
//                 ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);
//                 if (font) {
//                     loadedFonts.push_back(font);
//                     std::cout << "Loaded font: " << fontPath << std::endl;
//                 } else {
//                     std::cerr << "Failed to load font: " << fontPath << std::endl;
//                 }
//             }
//         }

//         if (loadedFonts.empty()) {
//             std::cerr << "No fonts loaded from directory: " << fontDir << std::endl;
//         }
//     } catch (const fs::filesystem_error& e) {
//         std::cerr << "Filesystem error: " << e.what() << std::endl;
//     }
// }



// // sigil::status_t sigil::visor::start_gui(sigil::window_t *window) {
// //    printf("iocommon: starting gui(%p) for %s\n",window->gui_routine, window->name.c_str());
// //     std::thread draw_thread(window->gui_routine);
// //     return sigil::VM_OK;
// // }

// // sigil::status_t iocommon::attach_io_to_window(sigil::window_t *window) {
// //     ImGuiIO& io = ImGui::GetIO();
// //     IMGUI_CHECKVERSION();
// //     IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
// //     //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

// //     // Setup backend capabilities flags
// //     ImGui_ImplGlfw_Data* bd = IM_NEW(ImGui_ImplGlfw_Data)();
// //     io.BackendPlatformUserData = (void*)bd;
// //     io.BackendPlatformName = "imgui_impl_glfw";
// //     io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
// //     io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
// // #ifndef __EMSCRIPTEN__
// //     io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
// // #endif
// // #if GLFW_HAS_MOUSE_PASSTHROUGH || GLFW_HAS_WINDOW_HOVERED
// //     io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
// // #endif

// //     bd->Window = window;
// //     bd->Time = 0.0;
// //     bd->WantUpdateMonitors = true;

// //     io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
// //     io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
// //     io.ClipboardUserData = bd->Window;
// // #ifdef __EMSCRIPTEN__
// //     io.PlatformOpenInShellFn = [](ImGuiContext*, const char* url) { ImGui_ImplGlfw_EmscriptenOpenURL(url); return true; };
// // #endif

// //     // Create mouse cursors
// //     // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
// //     // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
// //     // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
// //     GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
// //     bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
// // #if GLFW_HAS_NEW_CURSORS
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
// // #else
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// //     bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// // #endif
// //     glfwSetErrorCallback(prev_error_callback);
// // #if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
// //     (void)glfwGetError(nullptr);
// // #endif

// //     // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
// //     if (install_callbacks)
// //         ImGui_ImplGlfw_InstallCallbacks(window);
// //     // Register Emscripten Wheel callback to workaround issue in Emscripten GLFW Emulation (#6096)
// //     // We intentionally do not check 'if (install_callbacks)' here, as some users may set it to false and call GLFW callback themselves.
// //     // FIXME: May break chaining in case user registered their own Emscripten callback?
// // #ifdef __EMSCRIPTEN__
// //     emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, false, ImGui_ImplEmscripten_WheelCallback);
// // #endif

// //     // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
// //     ImGui_ImplGlfw_UpdateMonitors();
// //     glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);

// //     // Set platform dependent data in viewport
// //     ImGuiViewport* main_viewport = ImGui::GetMainViewport();
// //     main_viewport->PlatformHandle = (void*)bd->Window;
// // #ifdef _WIN32
// //     main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
// // #elif defined(__APPLE__)
// //     main_viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(bd->Window);
// // #else
// //     IM_UNUSED(main_viewport);
// // #endif
// //     if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
// //         ImGui_ImplGlfw_InitPlatformInterface();

// //     // Windows: register a WndProc hook so we can intercept some messages.
// // #ifdef _WIN32
// //     bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC);
// //     IM_ASSERT(bd->PrevWndProc != nullptr);
// //     ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)ImGui_ImplGlfw_WndProc);
// // #endif

// //     bd->ClientApi = client_api;
// //     return sigil::VM_OK;
// // }

// // sigil::status_t iocommon::attach_vk_to_window(sigil::window_t *window) {

// //     return sigil::VM_OK;
// // }

// // std::vector<sigil::window_t*>* get_window_table() {
// //     if (!visor_node) return nullptr;
// //     if (!visor_data) return nullptr;
// //     return &visor_data->windows;
// // }

// // sigil::status_t iocommon::attach_io_to_window(sigil::window_t *window) {
// //     return sigil::VM_OK;
// // }


// void glfw_error_callback(int error, const char* description) {
//     fprintf(stderr, "GLFW Error %d: %s\n", error, description);
// }

// sigil::status_t sigil::visor::soft_init_glfw() {
//     assert(visor_data != nullptr);
//     if (visor_data->glfw_initialized) return sigil::VM_ALREADY_EXISTS;

//     if (!glfwInit()) {
//        printf("iocommon: failed glfw init\n");
//         visor_data->glfw_initialized = false;
//         return sigil::VM_FAILED;
//     }

//     visor_data->glfw_initialized = true;
//     return sigil::VM_OK;
// }

sigil::graphics::window_t *sigil::visor::spawn_window(const char *window_name) {
    if (!visor_node) return nullptr;
    //if (!visor_data) return nullptr;

    // TODO: User returned status from soft init and log the outcome
    //soft_init_glfw();

    if (!glfwVulkanSupported()) {
        printf("iocommon: glfw-vulkan not available\n");
        return nullptr;
    }

    sigil::graphics::window_t *new_window = new sigil::graphics::window_t();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    new_window->glfw_window = glfwCreateWindow(1280, 720, window_name, nullptr, nullptr);

    if (new_window->glfw_window == nullptr) {
        printf("iocommon: failed to create glfw_window\n");
        return nullptr;
    }

    new_window->name = window_name;
    new_window->min_image_count = 2;

    windows.push_back(new_window);
    return new_window;
}

// sigil::window_t *sigil::visor::get_window(int index) {
//     if (!visor_node) return nullptr;
//     if (index < 0 && index > visor_data->windows.size()) return nullptr;
//     return visor_data->windows[index];
// }

//     // glfwSetErrorCallback(glfw_error_callback);


// sigil::status_t sigil::visor::check_for_swapchain_update(sigil::window_t *window) {
//     // int fb_width, fb_height;
//     // glfwGetFramebufferSize(window->glfw_window, &fb_width, &fb_height);
//     // if (fb_width > 0 && fb_height > 0 && (window->swapchain_rebuild || imgui_wd.Width != fb_width || imgui_wd.Height != fb_height))
//     // {
//     //     ImGui_ImplVulkan_SetMinImageCount(window->min_image_count);
//     //     ImGui_ImplVulkanH_CreateOrResizeWindow(vkhost->vk_inst, vkhost->main_gpu(), vkhost->vk_dev, &imgui_wd, vkhost->vk_main_q.family, vkhost->vk_allocators, fb_width, fb_height, window->min_image_count);
//     //     imgui_wd.FrameIndex = 0;
//     //     window->swapchain_rebuild = false;
//     // }

//     return VM_NOT_IMPLEMENTED;
// }

// sigil::status_t sigil::visor::new_frame(window_t *window) {
//     ImGui_ImplGlfw_NewFrame();
//     ImGui_ImplVulkan_NewFrame();
//     return sigil::VM_OK;
// }
sigil::status_t sigil::visor::initialize() {
    status_t status = sigil::virtual_machine::is_active();
    if (status != VM_OK) return status;

    sigil::exec_timer tmr;
    tmr.start();

    sigil::vmnode_descriptor_t node_info;
    node_info.name.value = "visor";
    
    sigil::virtual_machine::add_runtime_node(node_info);
    
    tmr.stop();
    printf("visor: initialized in %luns\n", tmr.ns());
    return sigil::VM_OK;
}

sigil::status_t sigil::visor::deinitialize() {

    return VM_OK;
}