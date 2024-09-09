/*
    Refs
    vmnodes
    name
    status
    parser objects
*/
#pragma once
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#define VM_NODE_VMROOT "vmroot"
#define VM_NODE_RUNTIME "runtime"
#define VM_NODE_PLATFORM "platform"
#define VM_NODE_INV_NAME "INVALID_NAME_DO_NOT_USE"
#define VM_NODE_LOOKUP_MAX_DEPTH 32

const char SHARED_MEMORY_NAME[] = "/SigilVM";
const char SEMAPHORE_NAME[] = "/SigilVM_semaphore";
const size_t SHARED_MEMORY_SIZE = 0xffffff;       // Adjust size as needed

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

// TODO: Remove deprecated name type
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
}
