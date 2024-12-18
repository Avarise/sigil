#pragma once
#include "system.h"

namespace sigil::virtual_machine_t {
    status_t initialize(int arg, char **argv);
    status_t deinitialize();
    status_t start_api_server();
    status_t start_api_vulkan();
    status_t start_api_visor();
    status_t start_api_sound();
    status_t start_api_ntt();
    vmnode_t* get_root_node();

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
    void console_subprogram();
}


// /* 
//     All in one loader, for use in tests and verbose aplications
// */
// sigil::status_t system::load_all_extensions() {
//     printf("SigilVM: loading all available extensions\n");
//     assert(vmroot != nullptr);

//     sigil::status_t ret;

//     ret = iocommon::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded iocommon\n");

//     ret = vulkan::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded vkhost\n");

//     ret = ntt::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded entity framework\n");

//     ret = station::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded station manager\n");

//     //     ret = vmwebhost::initialize(vmroot);
//     //     if (ext_load_failed(ret)) goto err;
//     //    printf("SigilVM: loaded vmwebhost\n");

//     return sigil::VM_OK;

//     err:
//     printf("SigilVM: Failed to load extensions (%s)\n", sigil::status_t_cstr(ret));
//     return ret;
// }