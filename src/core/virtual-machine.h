#pragma once
#include "system.h"
#include "utils.h"
#include "parser.h"

namespace sigil::virtual_machine {
    status_t initialize(int arg, const char *argv[]);
    status_t deinitialize();
    status_t run_command(sigil::parser::command_t &command);
    status_t is_active();
    status_t flush();
    status_t reset();
    
    status_t add_platform_node(sigil::vmnode_descriptor_t node_info);
    status_t add_runtime_node(sigil::vmnode_descriptor_t node_info);
    status_t spawn_thread(void (*task)(void));

    // status_t load_ntt();
    // status_t load_visor();
    // status_t load_vulkan();
    // status_t load_station();
    // status_t load_iocommon();
    // void console_subprogram();
    // void desktop_subprogram();
    status_t set_debug_mode(bool debug);
    bool get_debug_mode();
}

namespace sigil {
    void exit(status_t status);
}
