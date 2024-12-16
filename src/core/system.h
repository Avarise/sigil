#pragma once
/*
    calls to spawn vmroot node and two t1 nodes: runtime and platform
    runtime: spawns nodes for worker processes and similar
    platform: devices config, device info pci/usb etc? 
*/

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <mutex>
#include <ctime>
#include <map>

#define VM_NODE_VMROOT "vmroot"
#define VM_NODE_RUNTIME "runtime"
#define VM_NODE_PLATFORM "platform"
#define VM_NODE_INV_NAME "INVALID_NAME_DO_NOT_USE"
#define VM_NODE_LOOKUP_MAX_DEPTH 32

/* Memory helpers */
#define HASH_MASK 0xDEADBEEF
#define MSIZE_4K 4096
#define MMASK_4K 0xFFFFF000
#define MSIZE_PAGE 512
#define MMASK_PAGE 0xFFFFFE00
#define MSIZE_STRING 64
#define MMASK_STRING 0xFFFFFFC0
#define MSIZE_CHUNK 16
#define MMASK_CHUNK 0xFFFFFFF0
#define MAX_THREADS 128
#ifndef BIT
#define BIT(x) ((uint32_t)1 << x)
#endif /* BIT */

namespace sigil {
    enum status_t {
        VM_OK,
        VM_ALREADY_EXISTS,
        VM_BUSY,
        VM_LOCKED,
        VM_FAILED,
        VM_NOT_FOUND,
        VM_FAILED_ALLOC,
        VM_ARG_NULL,
        VM_ARG_INVALID,
        VM_NOT_SUPPORTED,
        VM_INVALID_ROOT,
        VM_SYSTEM_SHUTDOWN,
        VM_NOT_IMPLEMENTED,
        SWAPCHAIN_REBUILDING,
    };

    enum reference_type_t {
        REF_UNKNOWN,
        REF_VMNODE,
        REF_DEVICE,
        REF_EVENT,
        REF_FILE,
        REF_SERVICE,
        REF_GENERIC_MODULE,
        REF_WORKTHREAD,
        REF_WORKQUEUE,
        REF_ENGINE,
    };

    struct reference_t {
        reference_type_t type;
        uint64_t id;
        uint32_t refcount;
        void print_reference_info();
        std::string get_reference_info_string();
    };

    

    struct memstat_t {
        unsigned long size, resident, share, text, lib, data, dt;
    };

    // Usage info
    void get_memstats(memstat_t &result);
    void print_memstats(const memstat_t &result);
    void print_mem_report();
    
    // Byte viewer, chuck = 16 bytes
    void memview_chunk(const void *mem_start);
    void memview(const void *mem_start, uint32_t num_bytes, bool align);

    inline void u32swap(uint32_t a, uint32_t b){
#       ifndef USE_NO_COPY_SWAP
        uint32_t temp = a;
        a = b;
        b = temp;
#       else /* USE COPY SWAP*/
        a = a ^ b;
        b = a ^ b;
        a = a ^ b;
#       endif
    }


    inline void boolflip(bool &val) {
        val ^= 1;
    }

    inline void bitflip(uint32_t &bitarray, int bitidx) {
        bitarray ^= 1 << bitidx;
    }

    struct argparser_t {
        std::vector<std::string> arguments;

        argparser_t(int argc, const char* argv[]) {
            for (int i = 1; i < argc; ++i) {
                arguments.push_back(argv[i]);
            }
        }

        bool is_set(const std::string& option) const {
            for (const auto& arg : arguments) {
                if (arg == option) return true;
            }
            return false;
        }
    };

    struct name_t {
        std::string value;
    };

    inline const char* status_t_cstr(status_t status);
    inline const char* reference_type_t_cstr(reference_type_t reftype);
    
    inline void sleep_ms(uint32_t ms) {
        if (ms == 0) return;

#       ifdef __linux__
        timespec ts;
        ts.tv_sec = (ms / 1000);
        ts.tv_nsec = (ms % 1000) * 10e6;

        int res = 0;
        do {
            res = nanosleep(&ts, &ts);
        } while (res && errno == EINTR);

#       else
        return;
#       endif
    }
}

inline const char* sigil::status_t_cstr(sigil::status_t status) {
    switch (status) {
        case VM_OK: return "VM_OK";
        case VM_BUSY: return "VM_BUSY";
        case VM_LOCKED: return "VM_LOCKED";
        case VM_FAILED: return "VM_FAILED";
        case VM_ARG_NULL: return "VM_ARG_NULL";
        case VM_NOT_FOUND: return "VM_NOT_FOUND";
        case VM_FAILED_ALLOC: return "VM_FAILED_ALLOC";
        case VM_INVALID_ROOT: return "VM_INVALID_ROOT";
        case VM_NOT_SUPPORTED: return "VM_NOT_SUPPORTED";
        case VM_ALREADY_EXISTS: return "VM_ALREADY_EXISTS";
        case VM_SYSTEM_SHUTDOWN: return "VM_SYSTEM_SHUTDOWN";
        default: return "VM_UNKNOWN";
    }
}

inline const char* sigil::reference_type_t_cstr(sigil::reference_type_t type) {
    if (type == REF_VMNODE) return "REF_VMNODE";
    if (type == REF_DEVICE) return "REF_DEVICE";
    if (type == REF_EVENT) return "REF_EVENT";
    if (type == REF_FILE) return "REF_FILE";
    if (type == REF_SERVICE) return "REF_SERVICE";
    if (type == REF_GENERIC_MODULE) return "REF_GENERIC_MODULE";
    if (type == REF_WORKTHREAD) return "REF_WORKTHREAD";
    if (type == REF_WORKQUEUE) return "REF_WORKQUEUE";
    if (type == REF_ENGINE) return "REF_ENGINE";
    return "REF_UNKNOWN";
}

inline void sigil::reference_t::print_reference_info() {
    printf("sigil: Reference ID: %lu, reference count: %u, type: %s, location: %p\n",
                    this->id, this->refcount, sigil::reference_type_t_cstr(this->type), this);
}

inline std::string sigil::reference_t::get_reference_info_string() {
    return "";
}

namespace sigil {
    inline void exit(sigil::status_t status) {
        printf("VM Status: %s (%d), exiting\n", sigil::status_t_cstr(status), status);
        std::exit(0);
    }

        typedef status_t(*node_deinit_ft)(void); 


    struct vmnode_t : reference_t {
        sigil::name_t name;
        vmnode_t *master_node;
        std::vector<vmnode_t*> subnodes;
        uint8_t depth_at_tree;
        std::mutex node_mutex;

        struct node_data_t {
            void *data;
            //node_deinit_ft data_cleanup;
            void (*data_cleanup)(void);
        } node_data;

        sigil::status_t set_data(void *data, void (*data_cleanup)(void)) {
            if (!data || !data_cleanup) return sigil::VM_ARG_NULL;
            this->node_data.data = data;
            this->node_data.data_cleanup = data_cleanup;
            return sigil::VM_OK;
        }

        vmnode_t();
        ~vmnode_t();
        vmnode_t(const char *name);
        vmnode_t(const sigil::name_t name);
        sigil::status_t deinit();
        sigil::status_t deinit_self();
        sigil::status_t deinit_subnodes();
        vmnode_t* search(const char *name, int depth_current, int depth_max);
        vmnode_t* spawn_subnode();
        vmnode_t* spawn_subnode(const char *name);
        vmnode_t* peek_subnode(const char *name, int depth_max);
        vmnode_t* peek_master_node();
        vmnode_t* get_subnode(const char *name, int depth_max);
        vmnode_t* get_master_node();
        vmnode_t* get_root_node();
        void release();
        std::string get_node_name();
        std::string get_node_name_tree();
        void print_nodeinfo();
        void print_nodemem();
    };
}




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