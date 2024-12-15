#include "virtual-machine.h"
#include "system.h"
#include "utils.h"
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>


static std::vector<sigil::vmnode_t*> vmnode_lut;
static sigil::vmnode_t *platform = nullptr;
static sigil::vmnode_t *runtime = nullptr;
static sigil::vmnode_t *vmroot = nullptr;
static bool debug_mode = false;
static uint32_t uptime_seconds;
static std::string hostname;
static uint32_t commands_run = 0;
static uint32_t num_workers = 0;
static uint32_t hw_cores = 0;
static uint32_t num_nodes = 0;

// Vector of all threads, must be joined and cleaned
static std::vector<std::thread*> vm_guest_threads;

// Global state of VM, by default no vm active, so vm not found
static sigil::status_t vm_state = sigil::VM_NOT_FOUND;

//
static bool has_shutdown_handler = false;
static bool shutdown_requested = false;
static bool vm_initialized = false;


sigil::status_t sigil::virtual_machine::spawn_thread(void (*task)(void)) {
    if (!task) return VM_ARG_NULL;

    std::thread *temp_thread_ptr = new std::thread(task);
    if (!temp_thread_ptr) return VM_FAILED_ALLOC;
    vm_guest_threads.push_back(temp_thread_ptr);
    return VM_OK;
}

bool sigil::virtual_machine::get_debug_mode() {
    return debug_mode;
}


bool sigil::virtual_machine::is_active() {
    if (!vmroot) return false;
    if (!vm_initialized) return false;
    if (!has_shutdown_handler) return false;
    return true;
}

sigil::status_t sigil::virtual_machine::flush() {
    if (!vmroot) return VM_INVALID_ROOT;

    printf("virtual-machine: flushing not implemented yet\n");
    return VM_NOT_IMPLEMENTED;
}

sigil::status_t sigil::virtual_machine::add_platform_node(sigil::vmnode_descriptor_t node_info) {
    if (!platform) return VM_INVALID_ROOT;

    vmnode_t *new_node = platform->spawn_subnode(node_info.name.value.c_str());


    printf("virtual-machine: platform registered node %s\n", node_info.name.value.c_str());
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::add_runtime_node(sigil::vmnode_descriptor_t node_info) {
    if (!runtime) return VM_INVALID_ROOT;

    vmnode_t *new_node = runtime->spawn_subnode(node_info.name.value.c_str());

    printf("virtual-machine: runtime registered node %s\n", node_info.name.value.c_str());
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::initialize(int argc, const char *argv[]) {
    if (vmroot) return VM_ALREADY_EXISTS;

    if (argc > 0 && argv == nullptr) return sigil::VM_ARG_NULL;

    sigil::exec_timer tmr;
    
    tmr.start();
    vmroot = new sigil::vmnode_t(VM_NODE_VMROOT);
    if (!vmroot) return VM_INVALID_ROOT;

    runtime = vmroot->spawn_subnode(VM_NODE_RUNTIME);
    if (!runtime) return VM_FAILED_ALLOC;
    
    platform = vmroot->spawn_subnode(VM_NODE_PLATFORM);
    if (!platform) return VM_FAILED_ALLOC;

    vm_initialized = true;

    vm_state = VM_OK;

    tmr.stop();
    if (debug_mode) {
        printf("virtual-machine: initialized in %luus\n", tmr.us());
    }
    return vm_state;
}

sigil::status_t sigil::virtual_machine::deinitialize() {
    if (!vmroot) return VM_NOT_FOUND;

    vm_state = sigil::VM_SYSTEM_SHUTDOWN | vm_state;
    vm_initialized = false;
    
    for (auto &t : vm_guest_threads) {
        if (t->joinable()) {
            if (debug_mode) printf("virtual-machine: Joined a thread %p\n", t);
            t->join();
        }
    }
    
    //vmroot->deinit_subnodes();
    free(vmroot);
    vmroot = nullptr;
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::set_debug_mode(bool mode) {
    debug_mode = mode;
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::wait_for_shutdown() {
    if (!vmroot || !vm_initialized) {
        printf("virtual-machine: Error; waiting for shutdown while VM is not running\n");
        return VM_NOT_FOUND;
    }

    if (has_shutdown_handler) {
        printf("virtual-machine: warning, multiple waits for shutdown, might end in deadlock\n");
        return sigil::VM_LOCKED;
    }
    
    printf("virtual-machine: shutdown handler started\n");
    has_shutdown_handler = true;
    
    sigil::status_t status;

    while (is_active()) {
        sigil::sleep_ms(50);

        if (shutdown_requested) {
            status = sigil::virtual_machine::deinitialize();
            if (status != VM_OK) {
                printf("virtual-machine: Error occured when processing shutdown request\n");
            }
        }
    }

    return VM_OK;
}

sigil::status_t sigil::virtual_machine::wait_for_vm() {
    if (!vm_initialized) return VM_NOT_FOUND;

    while (!is_active()) {
        sleep_ms(10);
    }

    return VM_OK;
}

sigil::status_t sigil::virtual_machine::request_shutdown() {
    if (!is_active()) {
        printf("virtual-machine: Request shutdown, but the VM is not running\n");
        return VM_FAILED;
    }

    shutdown_requested = true;
    printf("virtual-machine: shutdown requested\n");
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::get_state() {
    return vm_state;
}

sigil::status_t sigil::virtual_machine::vminfo() {
    if (!vmroot) return VM_NOT_FOUND;

    vmroot->print_nodeinfo();

    return VM_OK;
}