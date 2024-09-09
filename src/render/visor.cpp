#include <thread>
#include "visor.h"
#include "../vm/system.h"

static sigil::vmnode_t *visor_node = nullptr;
static sigil::visor::visor_data_t *visor_data = nullptr;

static void cleanup_visor_data() {
    if (!visor_node) return;
    if (!visor_data) return;
    delete visor_data;
    visor_data = nullptr;
    visor_node->node_data.data = nullptr;
}

sigil::visor::visor_data_t::visor_data_t() {

}

sigil::visor::visor_data_t::~visor_data_t() {
    
}


sigil::status_t sigil::visor::start_gui(sigil::window_t *window) {
   printf("iocommon: starting gui(%p) for %s\n",window->gui_routine, window->name.c_str());
    std::thread draw_thread(window->gui_routine);
    return sigil::VM_OK;
}

// sigil::status_t iocommon::attach_io_to_window(sigil::window_t *window) {
//     ImGuiIO& io = ImGui::GetIO();
//     IMGUI_CHECKVERSION();
//     IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
//     //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

//     // Setup backend capabilities flags
//     ImGui_ImplGlfw_Data* bd = IM_NEW(ImGui_ImplGlfw_Data)();
//     io.BackendPlatformUserData = (void*)bd;
//     io.BackendPlatformName = "imgui_impl_glfw";
//     io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
//     io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
// #ifndef __EMSCRIPTEN__
//     io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
// #endif
// #if GLFW_HAS_MOUSE_PASSTHROUGH || GLFW_HAS_WINDOW_HOVERED
//     io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)
// #endif

//     bd->Window = window;
//     bd->Time = 0.0;
//     bd->WantUpdateMonitors = true;

//     io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
//     io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
//     io.ClipboardUserData = bd->Window;
// #ifdef __EMSCRIPTEN__
//     io.PlatformOpenInShellFn = [](ImGuiContext*, const char* url) { ImGui_ImplGlfw_EmscriptenOpenURL(url); return true; };
// #endif

//     // Create mouse cursors
//     // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
//     // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
//     // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
//     GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
//     bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
// #if GLFW_HAS_NEW_CURSORS
//     bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
// #else
//     bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// #endif
//     glfwSetErrorCallback(prev_error_callback);
// #if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // Eat errors (see #5908)
//     (void)glfwGetError(nullptr);
// #endif

//     // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
//     if (install_callbacks)
//         ImGui_ImplGlfw_InstallCallbacks(window);
//     // Register Emscripten Wheel callback to workaround issue in Emscripten GLFW Emulation (#6096)
//     // We intentionally do not check 'if (install_callbacks)' here, as some users may set it to false and call GLFW callback themselves.
//     // FIXME: May break chaining in case user registered their own Emscripten callback?
// #ifdef __EMSCRIPTEN__
//     emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, false, ImGui_ImplEmscripten_WheelCallback);
// #endif

//     // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
//     ImGui_ImplGlfw_UpdateMonitors();
//     glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);

//     // Set platform dependent data in viewport
//     ImGuiViewport* main_viewport = ImGui::GetMainViewport();
//     main_viewport->PlatformHandle = (void*)bd->Window;
// #ifdef _WIN32
//     main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
// #elif defined(__APPLE__)
//     main_viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(bd->Window);
// #else
//     IM_UNUSED(main_viewport);
// #endif
//     if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//         ImGui_ImplGlfw_InitPlatformInterface();

//     // Windows: register a WndProc hook so we can intercept some messages.
// #ifdef _WIN32
//     bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC);
//     IM_ASSERT(bd->PrevWndProc != nullptr);
//     ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)ImGui_ImplGlfw_WndProc);
// #endif

//     bd->ClientApi = client_api;
//     return sigil::VM_OK;
// }

// sigil::status_t iocommon::attach_vk_to_window(sigil::window_t *window) {

//     return sigil::VM_OK;
// }

// std::vector<sigil::window_t*>* get_window_table() {
//     if (!visor_node) return nullptr;
//     if (!visor_data) return nullptr;
//     return &visor_data->windows;
// }

// sigil::status_t iocommon::attach_io_to_window(sigil::window_t *window) {
//     return sigil::VM_OK;
// }


void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

sigil::status_t sigil::visor::soft_init_glfw() {
    assert(visor_data != nullptr);
    if (visor_data->glfw_initialized) return sigil::VM_ALREADY_EXISTS;

    if (!glfwInit()) {
       printf("iocommon: failed glfw init\n");
        visor_data->glfw_initialized = false;
        return sigil::VM_FAILED;
    }

    visor_data->glfw_initialized = true;
    return sigil::VM_OK;
}

sigil::window_t *sigil::visor::spawn_window(const char *window_name) {
    if (!visor_node) return nullptr;
    if (!visor_data) return nullptr;

    // TODO: User returned status from soft init and log the outcome
    soft_init_glfw();

    if (!glfwVulkanSupported()) {
        printf("iocommon: glfw-vulkan not available\n");
        return nullptr;
    }

    sigil::window_t *new_window = new sigil::window_t();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    new_window->glfw_window = glfwCreateWindow(1280, 720, window_name, nullptr, nullptr);

    if (new_window->glfw_window == nullptr) {
        printf("iocommon: failed to create glfw_window\n");
        return nullptr;
    }

    new_window->name = window_name;
    new_window->min_image_count = 2;

    visor_data->windows.push_back(new_window);
    return new_window;
}

sigil::window_t *sigil::visor::get_window(int index) {
    if (!visor_node) return nullptr;
    if (index < 0 && index > visor_data->windows.size()) return nullptr;
    return visor_data->windows[index];
}

    // glfwSetErrorCallback(glfw_error_callback);

sigil::status_t sigil::visor::initialize(sigil::vmnode_t *vmsr) {
    if (!vmsr) return sigil::VM_ARG_NULL;
    if (sigil::system::validate_root(vmsr) != 0) return sigil::VM_INVALID_ROOT;

    sigil::vmnode_t *runtime = vmsr->peek_subnode("runtime", 1);
    if (!runtime) return sigil::VM_INVALID_ROOT;

    if (visor_node != nullptr) {
        visor_data = (visor_data_t*)visor_node->node_data.data;
        return sigil::VM_ALREADY_EXISTS;
    }
    
    visor_node = runtime->spawn_subnode("vmwebhost");
    if (!visor_node) return sigil::VM_FAILED_ALLOC;

    visor_data = new visor_data_t;
    visor_node->set_data(visor_data, cleanup_visor_data);
    return sigil::VM_OK;
}

sigil::status_t sigil::visor::deinitialize() {

    return VM_OK;
}