#include "virtual-machine.h"
#include "system.h"
#include "utils.h"
#include <cstdio>
#include <thread>
#include <vector>

static sigil::vmnode_t *platform = nullptr;
static sigil::vmnode_t *runtime = nullptr;
static sigil::vmnode_t *vmroot = nullptr;
static bool debug_mode = false;
static uint32_t uptime_seconds;
static std::string hostname;
static uint32_t commands_run;
static uint32_t num_workers;
static uint32_t hw_cores;

// Vector of all threads, must be joined and cleaned
static std::vector<std::thread*> vm_guest_threads;

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


sigil::status_t sigil::virtual_machine::is_active() {
    if (!vmroot) return VM_INVALID_ROOT;

    return VM_OK;
}

sigil::status_t sigil::virtual_machine::flush() {
    if (!vmroot) return VM_INVALID_ROOT;

    printf("sigil-vm: flushing not implemented yet\n");
    return VM_NOT_IMPLEMENTED;
}

sigil::status_t sigil::virtual_machine::add_platform_node(sigil::vmnode_descriptor_t node_info) {
    if (!platform) return VM_INVALID_ROOT;

    printf("sigil-vm: platform registered node %s\n", node_info.name.value.c_str());
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::add_runtime_node(sigil::vmnode_descriptor_t node_info) {
    if (!runtime) return VM_INVALID_ROOT;

    vmnode_t *new_node = runtime->spawn_subnode(node_info.name.value.c_str());

    printf("sigil-vm: runtime registered node %s\n", node_info.name.value.c_str());
    return VM_OK;
}

sigil::status_t sigil::virtual_machine::initialize(int arg, const char *argv[]) {
    vmroot = new sigil::vmnode_t(VM_NODE_VMROOT);
    if (!vmroot) return VM_INVALID_ROOT;

    runtime = vmroot->spawn_subnode(VM_NODE_RUNTIME);
    if (!runtime) return VM_FAILED_ALLOC;
    
    platform = vmroot->spawn_subnode(VM_NODE_PLATFORM);
    if (!platform) return VM_FAILED_ALLOC;

    return VM_OK;
}

sigil::status_t sigil::virtual_machine::deinitialize() {
    if (!vmroot) return VM_NOOP;

    for (int i = 0; i < vm_guest_threads.size(); i++) {
        vm_guest_threads[i]->join();
    }

    return VM_OK;
}

sigil::status_t sigil::virtual_machine::set_debug_mode(bool mode) {
    debug_mode = mode;
    return VM_OK;
}