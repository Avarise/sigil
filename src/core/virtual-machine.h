#pragma once
#include "system.h"
#include "utils.h"
#include "parser.h"

namespace sigil::virtual_machine {
    status_t initialize(int arg, const char *argv[]);
    status_t deinitialize();
    status_t run_command(sigil::parser::command_t &command);
    bool is_active();
    status_t flush();
    status_t reset();

    status_t add_platform_node(sigil::vmnode_descriptor_t node_info);
    status_t add_runtime_node(sigil::vmnode_descriptor_t node_info);
    status_t spawn_thread(void (*task)(void));
    status_t run_command(parser::command_t cmd);
    
    /**
    * Request shutdown of a VM
    * Returns VM_OK on success
    * or VM_NOT_FOUND if VM is not active
    */
    status_t request_shutdown();

    /**
    * Go into a loop on a current thread
    * Wait until shutdown is requrested
    * Returns VM_OK on success
    */
    status_t wait_for_shutdown();

    /**
    * Waits until VM and shutdown handler are started
    * Returns VM_OK on success
    * or VM_FAILED if timeout is reached
    */
    status_t wait_for_vm();

    status_t get_state();
    
    status_t set_debug_mode(bool debug);
    bool     get_debug_mode();
}