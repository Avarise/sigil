#pragma once
#include "system.h"
#include "utils.h"
#include "parser.h"

namespace sigil::virtual_machine {
    status_t initialize(int arg, const char *argv[]);
    status_t deinitialize();
    vmnode_t* get_root_node();
    vmnode_t* get_runtime_node();
    vmnode_t* get_platform_node();
    status_t run_command(sigil::parser::command_t &command);
    status_t add_node(vmnode_t *node /*start, stop*/ );
    status_t remove_node(vmnode_t *node);
    status_t is_active();
    status_t flush();
    status_t reset();

    status_t load_ntt();
    status_t load_visor();
    status_t load_vulkan();
    status_t load_station();
    status_t load_iocommon();
    void console_subprogram();
}

namespace sigil {
    void exit(status_t status);
}
