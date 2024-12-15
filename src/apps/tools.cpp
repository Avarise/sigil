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
    --exec
        Run a SigilVM script snippet
    --fexec
        Run a SigilVM script snippet from a file
*/

#include "vulkan.h"
#include <cstring>
#include <iostream>
#include <vector>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <thread>
#include <argp.h>

#include "virtual-machine.h"
#include "graphics.h"
#include "imgui.h"
#include "utils.h"
#include "visor.h"
#include "log.h"

// Temporary define, for clangd to include debug code
// Probably should undef it and strip during build process
#define APP_USE_VULKAN_DEBUG_REPORT

#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Sigil Tools subcommands
static void             subcommand_flush_vm();
static void             subcommand_console();
static void             subcommand_exec();
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
static void             popup_save_project();
static void             popup_exit_project();
static void             popup_new_project();
static void             popup_import_file();
static void             popup_export_file();

// Procedure sent to visor to be threaded
static sigil::status_t  gui_main_loop();
static void             gui_prepare_frame(bool is_window_minimized);

// Clean exit with all subprocedures wrapped
static void             exit_sigil_tools();

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

// static struct {
//     std::thread *console;
//     std::thread *dwm;
//     std::thread *gui;
// } tools_threads {0};

static struct text_editor_t {
    char editor_content[1024 * 32] = {0};
} text_editor;

// SigilVM and auxiliary variable
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

sigil::graphics::window_t *main_window = nullptr;
const char program_name[] = "SigilVM Tools";
sigil::status_t app_status = sigil::VM_NOOP;
uint32_t frames_processed = 0;
sigil::memstat_t mem_usage;

void ensure_active_vm(int argc, const char **argv) {
    app_status = sigil::virtual_machine::is_active();
    if (app_status == sigil::VM_OK) return;

    app_status = sigil::virtual_machine::initialize(argc, argv);
    if (app_status == sigil::VM_OK) return;
    if (app_status == sigil::VM_ALREADY_EXISTS) return;

    printf("sigil-tools: failed to activate VM: %s\n", sigil::status_to_cstr(app_status));
    exit_sigil_tools();
}

// Main, main loop, init/deinit
int main(int argc, const char **argv) {
    sigil::argparser_t parser(argc, argv);

    if (parser.is_set("--debug")) {
        sigil::virtual_machine::set_debug_mode(true);
    }

    // First check for subcommands that don't spawn VM
    if (parser.is_set("--countdown")) {
        sigil::log::tt_end_of_year();
        app_status = sigil::VM_OK;
    }


    if (parser.is_set("--flushvm")) {
        subcommand_flush_vm();
    }

    if (parser.is_set("--exec")) {
        ensure_active_vm(argc, argv);
        subcommand_exec();
    }

    if (parser.is_set("--console")) {
        ensure_active_vm(argc, argv);
        subcommand_console();
    }

    if (parser.is_set("--dwm")) {
        ensure_active_vm(argc, argv);
        subcommand_dwm();
    }
    
    if (parser.is_set("--gui")) {
        ensure_active_vm(argc, argv);
        subcommand_gui();
    }


    exit_sigil_tools();
}

static void console_subprogram() {
    sigil::status_t status = sigil::VM_OK;

    while (status == sigil::VM_OK) {
        printf("sigil -> ");
        fflush(stdout);
        std::string payload;
        std::cin >> payload;

        if (strcmp(payload.c_str(), "exit") == 0) {
            exit_sigil_tools();
        }
        // TODO: ADd code execution
        //status = exe(payload);
        printf("\n");
    }

    printf("sigil-tools: shutting down console: %s\n", sigil::status_to_cstr(status));
}

void desktop_subprogram() {
    printf("SigilVM: Starting desktop session\n");
    system("startx /sigil/vm/src/scripts/dwm.sh");
    printf("SigilVM: Desktop session closed\n");
}


// Subcommands
void subcommand_flush_vm() {
    printf("sigil-tools: Flushing SigilVM\n");
    app_status = sigil::virtual_machine::flush();
    exit_sigil_tools();
}

void subcommand_console() {
    app_status = sigil::virtual_machine::spawn_thread(console_subprogram);

    app_status != sigil::VM_OK ?
        printf("Failed to start console: %s\n", sigil::status_to_cstr(app_status)):
        printf("Console Started\n");
}

void subcommand_dwm() {
    sigil::virtual_machine::spawn_thread(desktop_subprogram);
}

void subcommand_gui() {
    app_status = sigil::virtual_machine::is_active();
    if (app_status != sigil::VM_OK) {
        printf("sigil-tools: could not find active virtual machine, exiting\n");
        exit_sigil_tools();
    }

    app_status = sigil::vulkan::initialize();
    if (app_status != sigil::VM_OK) exit_sigil_tools();

    app_status = sigil::visor::initialize();
    if (app_status != sigil::VM_OK) exit_sigil_tools();


    // TODO: Prepare ImGui here, to minimize abstraction within modules
    // Load fonts
    printf("sigil-tools: creating main app window\n");
    main_window = sigil::visor::spawn_window(program_name);


    // if (main_window == nullptr) {
    //     printf("sigil-tools: could not acquire window\n");
    //     app_status = sigil::VM_FAILED_ALLOC;
    //     exit_sigil_tools(app_status);
    // }

    // app_status = sigil::visor::prepare_for_imgui(main_window);
    // if (app_status != sigil::VM_OK) {
    //     printf("sigil-tools: failed to prepare ImGui\n");
    //     exit_sigil_tools(app_status);
    // }

    // app_status = main_window->set_content(gui_main_loop);
    // if (app_status != sigil::VM_OK) {
    //     printf("sigil-tools: failed to set window content\n");
    //     exit_sigil_tools(app_status);
    // }

    // // Spawn gui thread
    // tools_threads.gui = main_window->deploy();
    // if (tools_threads.gui == nullptr) {
    //     printf("sigil-tools: failed to prepare sigil-tools gui\n");
    //     exit_sigil_tools(sigil::VM_FAILED_ALLOC);
    // }
}

void subcommand_exec() {
    printf("SigilVM: Direct command execution not available yet\n");
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
            if (ImGui::MenuItem("Exit", "Alt+F4")) exit_sigil_tools();
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
    ImGui::Begin("Text Editor");
    ImGui::End();
}

static void subwindow_demo() {
    ImGui::ShowDemoWindow(&subwindows.demo);
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

sigil::status_t gui_main_loop() {
    ImGuiIO& io = ImGui::GetIO();
    sigil::exec_timer gui_main_loop_timer;

    while (!glfwWindowShouldClose(main_window->glfw_window)) {
        // Start measuring frame thread time
        gui_main_loop_timer.start();

        // Use visor to prepare new frame
        app_status = sigil::visor::prepare_for_new_frame(main_window);
        if (app_status != sigil::VM_OK) exit_sigil_tools();

        // Prepare new frame, but from ImGui perspective only
        ImGui::NewFrame();
        ImGuiIO &io = ImGui::GetIO();
        main_window->imgui_wd.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        main_window->imgui_wd.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        main_window->imgui_wd.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        main_window->imgui_wd.ClearValue.color.float32[3] = clear_color.w;
        
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

        app_status = sigil::visor::finalize_new_frame(main_window);
        if (app_status != sigil::VM_OK) exit_sigil_tools();

        frames_processed++;
        gui_main_loop_timer.stop();

        


        if (!(frames_processed % 515)) {
            output_buffer += sigil::log::get_current_time();
            sigil::insert_into_string(output_buffer,":spent %lu microseconds rendering last frame (#%u)\n",
                                     gui_main_loop_timer.us(), frames_processed);
        }
    }

    return sigil::VM_OK;
}

void print_help() {
    
}

static void exit_sigil_tools() {
    if (app_status == sigil::VM_NOOP) {
        printf("Nothing to do...\n");
        goto egress;
    }

    app_status = sigil::virtual_machine::deinitialize();

    egress:
    if (sigil::virtual_machine::get_debug_mode())
        printf("sigil-tools: exiting -> %s\n", sigil::status_to_cstr(app_status));

    exit(0);
}