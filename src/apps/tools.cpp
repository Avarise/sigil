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

#include "virtual-machine.h"
#include "graphics.h"
#include "vulkan.h"
#include "imgui.h"
#include "utils.h"
#include "visor.h"
#include "log.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <stdio.h>
#include <regex.h>
#include <string>
#include <vector>

// SigilVM and auxiliary variable
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
sigil::graphics::window_t *main_window = nullptr;
const char program_name[] = "SigilVM Tools";
uint32_t frames_processed = 0;
sigil::memstat_t mem_usage;

std::vector<std::string> dummy_log = {};

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

static struct text_editor_t {
    char editor_content[1024 * 32] = {0};
} text_editor;

static bool vm_shutdown_on_dwm_shutdown = false;

// Sigil Tools subcommands
static sigil::status_t subcommand_flush_vm();
static sigil::status_t subcommand_console();
static sigil::status_t subcommand_exec();
static void            subcommand_help();
static sigil::status_t subcommand_roll();
static sigil::status_t subcommand_gui();
static sigil::status_t subcommand_dwm();

// Sigil Tools GUI subwindows
static void subwindow_style_manager();
static void subwindow_asset_manager();
static void subwindow_asset_editor();
static void subwindow_text_editor();
static void subwindow_overview();
static void subwindow_options();
static void subwindow_menubar();
static void subwindow_output();
static void subwindow_perf();
static void subwindow_demo();

// Popups
static void popup_open_project();
static void popup_save_project();
static void popup_exit_project();
static void popup_new_project();
static void popup_import_file();
static void popup_export_file();

// Subprograms aka funtions to be threaded
static void subprogram_console();
static void subprogram_desktop();
static void subprogram_gui();

// Wrapper to run sigil tools commands
sigil::status_t tools_cmd(const char* cmd);

// Main, main loop, init/deinit
int main(int argc, const char **argv) {
    if (argc == 1) {
        subcommand_help();
        exit(-1);
    }

    sigil::argparser_t parser(argc, argv);
    sigil::status_t status;

    if (parser.is_set("--help") || parser.is_set("-h")) {
        subcommand_help();
        exit(0);
    }

    if (parser.is_set("--debug") || parser.is_set("-d")) {
        sigil::virtual_machine::set_debug_mode(true);
    }

    if (parser.is_set("--countdown")) {
        sigil::log::tt_end_of_year();
    }

    if (parser.is_set("--flushvm") || parser.is_set("-f")) {
        subcommand_flush_vm();
        goto tools_wait_for_shutdown;
    }

    if (parser.is_set("--exec") || parser.is_set("-e")) {
        status = sigil::virtual_machine::initialize(argc, argv);
        if (!(status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS)) goto tools_wait_for_shutdown;
        subcommand_exec();
    }

    if (parser.is_set("--console")) {
        status = sigil::virtual_machine::initialize(argc, argv);
        if (!(status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS)) goto tools_wait_for_shutdown;
        subcommand_console();
    }

    if (parser.is_set("--dwm")) {
        if (argc < 3) vm_shutdown_on_dwm_shutdown = true;
        status = sigil::virtual_machine::initialize(argc, argv);
        if (!(status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS)) goto tools_wait_for_shutdown;
        subcommand_dwm();
    }
    
    if (parser.is_set("--gui")) {
        status = sigil::virtual_machine::initialize(argc, argv);
        if (!(status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS)) goto tools_wait_for_shutdown;
        subcommand_gui();
    }

    tools_wait_for_shutdown:
    status = sigil::virtual_machine::wait_for_shutdown();
    printf("Exitting (%s)\n", sigil::status_to_cstr(status));
    return status;
}

// Subcommands
static sigil::status_t subcommand_flush_vm() {
    printf("sigil-tools: flushing virtual machine\n");
    return sigil::virtual_machine::flush();
}

static sigil::status_t subcommand_console() {
    sigil::status_t status = sigil::virtual_machine::spawn_thread(subprogram_console);

    status != sigil::VM_OK ?
        printf("sigil-tools: failed to start console (%s)\n", sigil::status_to_cstr(status)):
        printf("sigil-tools: console started\n");
    
    return status;
}

static sigil::status_t subcommand_dwm() {
    sigil::status_t status = sigil::virtual_machine::spawn_thread(subprogram_desktop);

    status != sigil::VM_OK ?
        printf("sigil-tools: failed to start desktop environment (%s)\n", sigil::status_to_cstr(status)):
        printf("sigil-tools: desktop environment started\n");

    return status;
}

static sigil::status_t subcommand_gui() {
    sigil::status_t status = sigil::vulkan::initialize();
    if (status != sigil::VM_OK) {
        printf("sigil-tools: an error occured when initializing vulkan %s\n", sigil::status_to_cstr(status));
        return status;
    } 
        
    status = sigil::visor::initialize();
    if (status != sigil::VM_OK) {
        printf("sigil-tools: an error occured when initializing visor %s\n", sigil::status_to_cstr(status));
        return status;
    }

    status = sigil::virtual_machine::spawn_thread(subprogram_gui);
    return status;
}

sigil::status_t subcommand_exec() {
    printf("virtual-machine: Direct command execution not available yet\n");
    return sigil::VM_NOT_IMPLEMENTED;
}

static void subcommand_help() {
    printf("Usage: sigil-tools [OPTIONS] [SUBCOMMAND] [ARGS]\n\n");
    printf("Options and Subcommands:\n");
    printf("  --help               Show this help page.\n");
    printf("  --gui                Start the application in GUI mode.\n");
    printf("  --console            Start the application in interactive console mode.\n");
    printf("  --flushvm            Flush the current virtual machine state.\n");
    printf("  --dwm                Launch the built-in window manager.\n");
    printf("  --test-log           Run a test procedure for message logging.\n");
    printf("  --test-load          Run a test procedure for threaded load.\n");
    printf("  --test-results       Dump the results of the last test procedure.\n");
    printf("  --countdown          Display the percentage of the year passed and countdown to New Year's Eve.\n");
    printf("  --exec [COMMAND]     Execute an arbitrary command.\n");
    printf("  --fexec:/path/to/script.svm\n");
    printf("                      Execute a custom script from a file.\n\n");
    printf("Inbuilt Console Commands:\n");
    printf("  help                 Show this help page within the interactive console.\n");
    printf("  exit                 Exit the interactive console.\n");
    printf("  flushvm              Flush the current virtual machine state.\n");
    printf("  dwm                  Launch the built-in window manager.\n");
    printf("  test-log             Run a test procedure for message logging.\n");
    printf("  test-load            Run a test procedure for threaded load.\n");
    printf("  test-results         Dump the results of the last test procedure.\n");
    printf("  countdown            Display the percentage of the year passed and countdown to New Year's Eve.\n");
    printf("  exec [COMMAND]       Execute an arbitrary command within the console.\n");
    printf("  fexec /path/to/script.svm\n");
    printf("                      Execute a custom script from a file.\n\n");
    printf("For more details, refer to the documentation or use the --help option.\n");
}

// Subprograms
static void subprogram_console() {
    sigil::virtual_machine::wait_for_vm();
    std::string payload;
    sigil::status_t status;

    while (sigil::virtual_machine::is_active()) {
        // Simple propmt
        // TODO: Add variable for changeable prompt
        printf("sigil -> ");
        // Flush to get prompt out
        fflush(stdout);
        // Get the user input
        std::cin >> payload;
        //printf("\n");
        // Run command
        status = tools_cmd(payload.c_str());
        if (status == sigil::VM_SYSTEM_SHUTDOWN) goto console_exit;

        bool critical_err = !(status == sigil::VM_OK || status == sigil::VM_NOT_IMPLEMENTED);
        if (critical_err) {
            printf("sigil-tools: critical error occured within console thread (%s)\n", sigil::status_to_cstr(status));
            goto console_exit;
        }
    }

    console_exit:
    printf("\nsigil-tools: exiting console\n");
}

static void subprogram_desktop() {
    // Wait until VM is fully launched
    sigil::exec_timer desktop_timer;
    desktop_timer.start();
    sigil::virtual_machine::wait_for_vm();
    desktop_timer.stop();
    printf("\nsigil-tools: starting desktop session after wating %lums\n", desktop_timer.ms());
    
    desktop_timer.start();
    system("startx /sigil/vm/src/scripts/dwm.sh");
    
    desktop_timer.stop();
    printf("\nsigil-tools: desktop session closed after %lums\n", desktop_timer.ms());

    if (vm_shutdown_on_dwm_shutdown) sigil::virtual_machine::request_shutdown();
}

// Wrap this into a thread after we ensured that VM is running
static void subprogram_gui() {
    sigil::exec_timer gui_subpr_timer;
    gui_subpr_timer.start();
    sigil::status_t status = sigil::virtual_machine::wait_for_vm();
    gui_subpr_timer.stop();
    if (sigil::virtual_machine::get_debug_mode()) {
        printf("sigil-tools: GUI waited %lums for VM\n", gui_subpr_timer.ms());
    }

    gui_subpr_timer.start();
    status = sigil::graphics::initialize_glfw();
    if (!(status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS)) {
        printf("sigil-tools: error failed to init glfw %s\n", sigil::status_to_cstr(status));
        return;
    }

    gui_subpr_timer.stop();
    if (sigil::virtual_machine::get_debug_mode()) {
        printf("sigil-tools: glfw registered in %luus\n", gui_subpr_timer.us());
    }
    
    gui_subpr_timer.start();
    // TODO: Add proper initialization
    ImGuiIO& io = ImGui::GetIO();


    // Measure init stop
    gui_subpr_timer.stop();
    if (sigil::virtual_machine::get_debug_mode()) {
        printf("sigil-tools: GUI components initialized in %luus\n", gui_subpr_timer.us());
    }

    while (!glfwWindowShouldClose(main_window->glfw_window)) {
        // Start measuring frame thread time
        gui_subpr_timer.start();

        // Use visor to prepare new frame
        status = sigil::visor::prepare_for_new_frame(main_window);
        if (status != sigil::VM_OK) return;

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

        status = sigil::visor::finalize_new_frame(main_window);
        if (status != sigil::VM_OK) return;

        frames_processed++;
        gui_subpr_timer.stop();

        // TODO: Add decay based last X frame stats

        if (!(frames_processed % 515)) {
            output_buffer += sigil::log::get_current_time();
            sigil::insert_into_string(output_buffer,":spent %lu microseconds rendering last frame (#%u)\n",
                gui_subpr_timer.us(), frames_processed);
        }
    }
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
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                sigil::status_t st = sigil::virtual_machine::request_shutdown();
                if (st != sigil::VM_OK) {
                    printf("sigil-tools: errors occured when requesting shutdown %s\n", sigil::status_to_cstr(st));
                }
            }
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
    // ImGui::Text("Program size (Pages/MB): %lu/%luMB", mem_usage.size, mem_usage.size >> 8 );
    // ImGui::Text("Text size (Pages/MB): %lu/%luMB", mem_usage.text, mem_usage.text >> 8);
    // ImGui::Text("Shared memory (Pages/MB): %lu/%luMB", mem_usage.share, mem_usage.share >> 8);
    // ImGui::Text("Data memory usage (Pages/MB): %lu/%luMB", mem_usage.data, mem_usage.data >> 8);
    // ImGui::Text("Resident memory usage (Pages/MB): %lu/%luMB", mem_usage.resident, mem_usage.resident >> 8);

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

void dummy_load() {
    int i = 0;

    while (sigil::virtual_machine::is_active()) {
        sigil::sleep_ms(15000);
        std::string temp = "Test entry: " + std::to_string(i);
        dummy_log.push_back(temp);
        i++;
        //printf("Hello from some thread\n");
    }
}

sigil::status_t tools_cmd(const char* cmd) {
    if (!cmd) return sigil::VM_ARG_NULL;

    sigil::status_t status;

    if (strcmp(cmd, "exit") == 0) {
        sigil::virtual_machine::request_shutdown();
        return sigil::VM_SYSTEM_SHUTDOWN;
    }

    else if (strcmp(cmd, "desktop") == 0) {
        subprogram_desktop();
        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "hwprobe") == 0) {
        status = sigil::vulkan::initialize();
        if (status != sigil::VM_OK) return status;

        //status = sigil::vulkan::probe_devices();
        return status;
    }

    else if (strcmp(cmd, "gui") == 0) {
        subcommand_gui();
        return sigil::VM_OK;
    }


    else if (strcmp(cmd, "dummy_load") == 0) {
        sigil::virtual_machine::spawn_thread(dummy_load);
        return sigil::VM_OK;
    }


    else if (strcmp(cmd, "treeinfo") == 0) {
        sigil::virtual_machine::vminfo();
        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "memstats") == 0) {
        sigil::get_memstats(mem_usage);
        sigil::print_memstats(mem_usage);
        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "cookie") == 0) {
        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "cookie") == 0) {
        subcommand_help();
        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "test") == 0) {
        uint32_t num_iters = sigil::random_u32_scoped(255, 1024);
        
        printf("Adding %u entries\n", num_iters);
        
        for (uint32_t i = 0; i < num_iters; i++) {
            std::string temp = "Test entry: " + std::to_string(i);
            dummy_log.push_back(temp);
        }

        return sigil::VM_OK;
    }

    else if (strcmp(cmd, "test-results") == 0) {
        uint32_t num_iters = dummy_log.size();

        for (uint32_t i = 0; i < num_iters; i++) {
            printf("Log: %s\n", dummy_log.at(i).c_str());
        }

        return sigil::VM_OK;
    }

    else {
        printf("sigil-tools: unknown command %s\n", cmd);
        return sigil::VM_NOT_IMPLEMENTED;
    }
}

