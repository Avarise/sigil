#pragma once
/*
    calls to spawn vmroot node and two t1 nodes: runtime and platform
    runtime: spawns nodes for worker processes and similar
    platform: devices config, device info pci/usb etc? 
*/

#include <cstring>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <mutex>
#include <ctime>
#include "utils.h"

#define VM_NODE_VMROOT "vmroot"
#define VM_NODE_RUNTIME "runtime"
#define VM_NODE_PLATFORM "platform"
#define VM_NODE_INV_NAME "INVALID_NAME_DO_NOT_USE"
#define VM_NODE_LOOKUP_MAX_DEPTH 32

namespace sigil {
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

    inline const char* reference_type_to_cstr(reference_type_t reftype);

    struct reference_t {
        reference_type_t type;
        uint64_t id;
        uint32_t refcount;
        void print_reference_info();
        std::string get_reference_info_string();
    };


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

    struct vmnode_t : reference_t {
        sigil::name_t name;
        vmnode_t *master_node;
        std::vector<vmnode_t*> subnodes;
        uint8_t depth_at_tree;
        std::mutex node_mutex;
        // Raw pointer to whatever a given node deems relevant
        void *data;

        // References to headers
        status_t (*start)(void);
        status_t (*stop)(void);

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

    struct vmnode_descriptor_t {
        sigil::name_t name;
        status_t (*start)(void);
        status_t (*stop)(void);
    };



    inline void reference_t::print_reference_info() {
        printf("sigil: Reference ID: %lu, reference count: %u, type: %s, location: %p\n",
                        this->id, this->refcount, sigil::reference_type_to_cstr(this->type), this);
    }

    inline const char* reference_type_to_cstr(sigil::reference_type_t type) {
        if (type == REF_GENERIC_MODULE) return "REF_GENERIC_MODULE";
        if (type == REF_WORKTHREAD) return "REF_WORKTHREAD";
        if (type == REF_WORKQUEUE) return "REF_WORKQUEUE";
        if (type == REF_SERVICE) return "REF_SERVICE";
        if (type == REF_DEVICE) return "REF_DEVICE";
        if (type == REF_ENGINE) return "REF_ENGINE";
        if (type == REF_VMNODE) return "REF_VMNODE";
        if (type == REF_EVENT) return "REF_EVENT";
        if (type == REF_FILE) return "REF_FILE";
        return "REF_UNKNOWN";
    }

    typedef status_t(*node_deinit_ft)(void); 

}

inline std::string sigil::reference_t::get_reference_info_string() {
    return "";
}