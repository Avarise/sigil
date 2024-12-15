#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <random>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../render/vulkan.h"
#include "../net/station.h"
#include "../engine/ntt.h"
#include "../utils/log.h"
#include "iocommon.h"
#include "memory.h"
#include "system.h"
#include "core.h"
#include "node.h"


using namespace sigil;
static sigil::vmnode_t *vmroot = nullptr;
static sigil::vmnode_t *platform = nullptr;
static sigil::vmnode_t *runtime = nullptr;
static system::vmroot_data_t *vmroot_data = nullptr;
static system::platform_data_t *platform_data = nullptr;
static system::runtime_data_t *runtime_data = nullptr;
static bool using_fakeroot = false;

std::thread *console_thread = nullptr;

void cleanup_vmroot_data() {
    if (!vmroot) return;
    if (!vmroot->node_data.data) return;
    delete((system::vmroot_data_t*)vmroot->node_data.data);
    vmroot->node_data.data = nullptr;
    vmroot_data = nullptr;
}

void cleanup_runtime_data() {
    if (!runtime) return;
    if (!runtime->node_data.data) return;
    delete((system::runtime_data_t*)runtime->node_data.data);
    runtime->node_data.data = nullptr;
    runtime_data = nullptr;
}

void cleanup_platform_data() {
    if (!platform) return;
    if (!platform->node_data.data) return;
    delete((system::platform_data_t*)platform->node_data.data);
    platform->node_data.data = nullptr;
    platform_data = nullptr;
}

/*
    Valid command line parameters follow this structure:
    -tag=myval
    which will be mapped as
    init_params[tag] -> "myval"
    free standing arguments will be parsed as a launch command, which can be
    interpolated with other tags
    sigil-tools -debug=full -o=/path/to/open 
*/
void parse_init_params(int argc, const char *argv[]) {
    assert(vmroot != nullptr);
    assert(vmroot_data != nullptr);
    if (argc <= 1) return;
    
    for (int i = 1; i < argc; i++) {
       printf("arg%d:%s\n", i, argv[i]);
    }
}

std::map<std::string, std::string>* system::get_init_params() {
    assert(vmroot != nullptr);
    assert(vmroot_data != nullptr);
    return &vmroot_data->init_params;
}

bool ext_load_failed(sigil::status_t status) {
    if (status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS) return false;
    return true;
}
/* 
    All in one loader, for use in tests and verbose aplications
*/
sigil::status_t system::load_all_extensions() {
    printf("SigilVM: loading all available extensions\n");
    assert(vmroot != nullptr);

    sigil::status_t ret;

    ret = iocommon::initialize(vmroot);
    if (ext_load_failed(ret)) goto err;
    printf("SigilVM: loaded iocommon\n");

    ret = vulkan::initialize(vmroot);
    if (ext_load_failed(ret)) goto err;
    printf("SigilVM: loaded vkhost\n");

    ret = ntt::initialize(vmroot);
    if (ext_load_failed(ret)) goto err;
    printf("SigilVM: loaded entity framework\n");

    ret = station::initialize(vmroot);
    if (ext_load_failed(ret)) goto err;
    printf("SigilVM: loaded station manager\n");

    //     ret = vmwebhost::initialize(vmroot);
    //     if (ext_load_failed(ret)) goto err;
    //    printf("SigilVM: loaded vmwebhost\n");

    return sigil::VM_OK;

    err:
    printf("SigilVM: Failed to load extensions (%s)\n", sigil::status_t_cstr(ret));
    return ret;
}

sigil::status_t system::validate_root(sigil::vmnode_t *node) {
    //printf("Validating node %p\n", node);
    if (node == nullptr) return sigil::VM_ARG_NULL;
    if (node->depth_at_tree != 0) return sigil::VM_ARG_INVALID;
    if (node->name.value.compare(VM_NODE_VMROOT) != 0) return sigil::VM_ARG_INVALID;
    return sigil::VM_OK;
}

sigil::status_t system::shutdown() {
    if (log::use_console) {
        wait_for_console();
    }
    
    sigil::status_t s = vmroot->deinit();
    delete(vmroot);
    return s;
}

sigil::vmnode_t* system::spawn_root(int argc, const char *argv[]) {
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {     
       printf("Failed to created shared memory\n");
        return nullptr;
    }

    if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1) {
       printf("Failed to set shared memory size\n");
        return nullptr;
    }

    void* ptr = mmap(nullptr, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
       printf("Failed to map shared memory\n");
        return nullptr;
    }

    vmroot = static_cast<sigil::vmnode_t*>(ptr);
    //return shm_fd;

    vmroot->name.value = VM_NODE_VMROOT;
    //vmroot = new sigil::vmnode_t(VM_NODE_VMROOT);
    vmroot_data = new system::vmroot_data_t;
    vmroot->set_data(vmroot_data, cleanup_vmroot_data);
    
    runtime = vmroot->spawn_subnode(VM_NODE_RUNTIME);
    runtime_data = new system::runtime_data_t;
    runtime->set_data(runtime_data, cleanup_runtime_data);

    platform = vmroot->spawn_subnode(VM_NODE_PLATFORM);
    platform_data = new system::platform_data_t;
    platform->set_data(platform_data, cleanup_platform_data);
     
    return vmroot;
}

sigil::vmnode_t* system::spawn_fakeroot(int argc, const char *argv[]) {
    using_fakeroot = true;

    vmroot = new sigil::vmnode_t(VM_NODE_VMROOT);
    vmroot_data = new system::vmroot_data_t;
    vmroot->set_data(vmroot_data, cleanup_vmroot_data);
    
    runtime = vmroot->spawn_subnode(VM_NODE_RUNTIME);
    runtime_data = new system::runtime_data_t;
    runtime->set_data(runtime_data, cleanup_runtime_data);

    platform = vmroot->spawn_subnode(VM_NODE_PLATFORM);
    platform_data = new system::platform_data_t;
    platform->set_data(platform_data, cleanup_platform_data);
     
    return vmroot;
}

sigil::vmnode_t* system::probe_root() {
//     void *shared_map = nullptr;
//     int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);

//     if (shm_fd == -1) {
//        printf("No shared memory found\n");
//         return nullptr;
//     }

//     shared_map = mmap(nullptr, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (shared_map == MAP_FAILED) {
//        printf("SigilVM: Failed to map shared memory\n");
        
//         return nullptr;
//     }

//     vmroot = (sigil::vmnode_t*)shared_map;
//    printf("Checking vmroot %p -> %s\n", shared_map, sigil::status_t_cstr(validate_root(vmroot)));
    return vmroot;
}


sigil::status_t system::invalidate_root() {
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);

    if (shm_fd == -1) {
        std::cout << "Shared memory object " << SHARED_MEMORY_NAME << " does not exist or cannot be opened." << std::endl;
    } else {
        // Close the file descriptor
        close(shm_fd);

        // Unlink (delete) the shared memory object
        if (shm_unlink(SHARED_MEMORY_NAME) == 0) {
            std::cout << "Shared memory object " << SHARED_MEMORY_NAME << " successfully deleted." << std::endl;
        } else {
            std::cerr << "Failed to delete shared memory object " << SHARED_MEMORY_NAME << "." << std::endl;
        }
    }

    return sigil::VM_OK;
}

sigil::status_t sigil::system::initialize(int argc, const char *argv[]) {
    sigil::argparser_t parser(argc, argv);

    // if (parser.is_set("--fakeroot")) {
    //     vmroot = system::spawn_fakeroot(argc, argv);
    // } else if ((vmroot = system::probe_root()) == nullptr) {
    //     vmroot = system::spawn_root(argc, argv);
    // }

    vmroot = system::spawn_fakeroot(argc, argv);

    if (vmroot == nullptr) sigil::exit(sigil::VM_INVALID_ROOT);

    if (parser.is_set("--debug")) {
        vmroot_data->global_debug = true;
    }


    parser.is_set("--console") ?
        sigil::log::enable_console():
        sigil::log::disable_console();


    // TODO: Check if start was successful
    if (sigil::log::use_console) sigil::system::start_console();
    return VM_OK;
}

sigil::status_t system::exec(std::string &payload) {
    //printf("Got command: %s\n", payload.c_str());
    
    if (payload.compare("exit") == 0) {
        return sigil::VM_SYSTEM_SHUTDOWN;
    }

    if (payload.compare("disable root") == 0) {
        system::invalidate_root();
        return sigil::VM_SYSTEM_SHUTDOWN;
    }

    if (payload.compare("treeinfo") == 0) {
        vmroot->print_nodeinfo();
        return sigil::VM_OK;    
    }

    // if (payload.compare("memstats") == 0) {
    //     memory::print_mem_report();
    //     return sigil::VM_OK;    
    // }
    
    if (payload.compare("cookie") == 0) {
       printf("Previous cookie: %x\n", vmroot_data->cookie);
        std::random_device rd;

        // Create a Mersenne Twister generator seeded with the random device
        std::mt19937 gen(rd());

        // Define a uniform distribution in the range of uint32_t
        std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

        // Generate a random uint32_t value
        uint32_t random_value = dis(gen);
        vmroot_data->cookie = random_value;
       printf("New cookie: %x\n", vmroot_data->cookie);
        return sigil::VM_OK;    
    }

    return sigil::VM_OK;
}



// sigil::status_t system::start_console() {
//     console_thread = new std::thread(console_loop);
//     return sigil::VM_OK;
// }

// sigil::status_t system::wait_for_console() {
//     if (console_thread == nullptr) return VM_NOT_FOUND;
//     console_thread->join();
//     printf("Console stopped\n");
//     return sigil::VM_OK;
// }

