#pragma once
/*
    calls to spawn vmroot node and two t1 nodes: runtime and platform
    runtime: spawns nodes for worker processes and similar
    platform: devices config, device info pci/usb etc? 
*/

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include "core.h"
#include "node.h"

namespace sigil::system {
    struct platform_data_t {
        uint32_t hw_cores;
        std::string hostname;
        uint8_t debug_mode;
        status_t refresh();
    };

    struct runtime_data_t {
        uint32_t num_workers;
        uint8_t debug_mode;
    };

    struct vmroot_data_t {
        std::map<std::string, std::string> init_params;
        uint32_t cookie;
        bool fakeroot = false;
        bool global_debug = false;
    };

    // IPC / Lifetime API
    sigil::status_t initialize(int argc, const char *argv[]);
    sigil::status_t shutdown();
    // Remove shared mem
    sigil::status_t flushvm();
    // Returns handle to active root if found any, nullptr otherwise
    sigil::vmnode_t* probe_root();
    // Create new VM root, pass argc/argv here
    sigil::vmnode_t* spawn_root(int argc, const char *argv[]);
    // Creates new root but keeps lifetime non-ipc, so might be used for mocking/assert testing
    sigil::vmnode_t* spawn_fakeroot(int argc, const char *argv[]);
    // Explains why it could not shutdown in return value
    sigil::status_t invalidate_root();     

    // Other calls
    // Validation: VM_OK if referenced node is valid root
    sigil::status_t validate_root(sigil::vmnode_t *node);
    // Execute generic command
    sigil::status_t exec(std::string &payload);
    // Cli handler is a simple looped text prompt 
    sigil::status_t start_cli_handler(sigil::vmnode_t *vmsr);
    std::map<std::string, std::string>* get_init_params();
    sigil::status_t shutdown_node(sigil::vmnode_t *target);
 
    // Extension loaders
    sigil::status_t load_all_extensions();
    sigil::status_t load_http_server_extensions();
    sigil::status_t load_game_engine_extensions();
    sigil::status_t load_minimal_extensions();
    sigil::status_t start_console();
    sigil::status_t wait_for_console();
}