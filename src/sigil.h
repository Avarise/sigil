#pragma once
#include <iostream>
#include "vm/system.h"
#include "vm/core.h"
#include "vm/node.h"

namespace sigil {
    struct virtual_machine_t {
        status_t initialize(int arg, char **argv);
        status_t deinitialize();
        status_t start_api_server();
        status_t start_api_vulkan();
        status_t start_api_visor();
        status_t start_api_sound();
        status_t start_api_ntt();

        // Virtual machine data
        vmnode_t *root;
    };

    void console_subprogram();
}