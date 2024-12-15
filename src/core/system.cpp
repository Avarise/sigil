#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <cstdio>
#include <vector>
#include "system.h"

sigil::vmnode_t::vmnode_t() {
    memset((void*)this, 0, sizeof(*this));
    this->type = REF_VMNODE;
}

sigil::vmnode_t::~vmnode_t() {
    printf("Removed: %s\n", this->name.value.c_str());
}

sigil::vmnode_t::vmnode_t(const char *name) {
    memset((void*)this, 0, sizeof(*this));
    this->type = REF_VMNODE;

    if (!name) {
        this->name.value = VM_NODE_INV_NAME;
        return;
    }
    this->name.value = name;
}

sigil::vmnode_t*
sigil::vmnode_t::get_root_node() {
    if (this->depth_at_tree == 0) return this;
    if (!this->master_node) {
        // TODO: Change to logger
       printf("Warning, %s is orphaned node\n", this->name.value.c_str());
        return nullptr;
    }
    return this->master_node->get_root_node();
}

sigil::vmnode_t*
sigil::vmnode_t::spawn_subnode() {
    sigil::vmnode_t* new_node = new vmnode_t();
    new_node->refcount = 0;
    new_node->master_node = this;
    this->refcount++;
    this->subnodes.push_back(new_node);
    new_node->depth_at_tree = this->depth_at_tree + 1;
    return new_node;
}

sigil::vmnode_t*
sigil::vmnode_t::spawn_subnode(const char* name) {
    if (!name) return nullptr;

    sigil::vmnode_t *root = this->get_root_node();
    if (!root) return nullptr;

    if (root->search(name, 0, VM_NODE_LOOKUP_MAX_DEPTH)) {
       printf("%s already exists!\n", name);
        return nullptr;
    }

    sigil::vmnode_t *new_node = this->spawn_subnode();
    new_node->name.value = name;
    return new_node;
}

std::string sigil::vmnode_t::get_node_name() {
    return this->name.value;
}

std::string sigil::vmnode_t::get_node_name_tree() {
    std::string payload = this->get_node_name();

    size_t num_subnodes = subnodes.size();

    for (size_t i = 0; i < num_subnodes; i++) {
        payload += ": ";
        payload += subnodes[i]->get_node_name_tree();
    }

    return payload;
}

// TODO: Implement recursive reference release
void sigil::vmnode_t::release() {
    if (this->refcount > 0) this->refcount--;
}

sigil::vmnode_t *sigil::vmnode_t::search(const char *name, int depth_current, int depth_max) {
    if (!name || depth_current > VM_NODE_LOOKUP_MAX_DEPTH) return nullptr;
    size_t num_subnodes = this->subnodes.size();

    if (this->name.value.compare(std::string(name)) == 0) {
        return this;
    }

    if (num_subnodes > 0) {
        vmnode_t *found = nullptr;

        for (size_t i = 0; i < num_subnodes; i++) if (found == nullptr) {
            found = this->subnodes[i]->search(name, depth_current + 1, depth_max);
        }

        return found;
    }

    return nullptr;
}

sigil::vmnode_t *sigil::vmnode_t::peek_subnode(const char *name, int depth_max) {
    return this->search(name, 0, depth_max);
}

sigil::vmnode_t *sigil::vmnode_t::get_subnode(const char *name, int depth_max) {
    vmnode_t *ref = peek_subnode(name, depth_max);
    if (ref) ref->refcount++;
    return ref;
}

void sigil::vmnode_t::print_nodeinfo() {
   printf("vm-tree: ");

    for (int i = 0; i < this->depth_at_tree; i++) {
       printf("    ");
    }

   printf("%s(%u, RC:%u), ", 
        this->get_node_name().c_str(), depth_at_tree,
        this->refcount);    
   printf("master: %s, data: %p\n",
        this->master_node ? master_node->name.value.c_str() : "self/root",
        this->data);
    
    size_t num_subnodes = this->subnodes.size();
    if (num_subnodes > 0) {
        for (size_t i = 0; i < num_subnodes; i++) {
            this->subnodes[i]->print_nodeinfo();
        }
    }
}

void sigil::vmnode_t::print_nodemem() {
    //sigil::memory::view(this, sizeof(*this), false);
    if (this->subnodes.size() > 0) {
        for (size_t i = 0; i < this->subnodes.size(); i++) {
            this->subnodes.at(i)->print_nodemem();
        }
    }
}

sigil::status_t sigil::vmnode_t::deinit() {
    printf("Querying deinit of %s\n", this->name.value.c_str());

    if (this->subnodes.size() > 0) for (auto s : this->subnodes) {
        s->deinit();
        delete(s);
    }

    auto master = this->master_node;
    //if (this->refcount == 0) this->node_data.data_cleanup();
    if (master) master->release();
    return sigil::VM_OK;
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
// void parse_init_params(int argc, const char *argv[]) {
//     assert(vmroot != nullptr);
//     assert(vmroot_data != nullptr);
//     if (argc <= 1) return;
    
//     for (int i = 1; i < argc; i++) {
//        printf("arg%d:%s\n", i, argv[i]);
//     }
// }

// std::map<std::string, std::string>* system::get_init_params() {
//     assert(vmroot != nullptr);
//     assert(vmroot_data != nullptr);
//     return &vmroot_data->init_params;
// }

bool ext_load_failed(sigil::status_t status) {
    if (status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS) return false;
    return true;
}


// sigil::status_t system::validate_root(sigil::vmnode_t *node) {
//     //printf("Validating node %p\n", node);
//     if (node == nullptr) return sigil::VM_ARG_NULL;
//     if (node->depth_at_tree != 0) return sigil::VM_ARG_INVALID;
//     if (node->name.value.compare(VM_NODE_VMROOT) != 0) return sigil::VM_ARG_INVALID;
//     return sigil::VM_OK;
// }

// sigil::status_t system::shutdown() {
//     if (log::use_console) {
//         wait_for_console();
//     }
    
//     sigil::status_t s = vmroot->deinit();
//     delete(vmroot);
//     return s;
// }

// sigil::vmnode_t* system::spawn_root(int argc, const char *argv[]) {
//     int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
//     if (shm_fd == -1) {     
//        printf("Failed to created shared memory\n");
//         return nullptr;
//     }

//     if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1) {
//        printf("Failed to set shared memory size\n");
//         return nullptr;
//     }

//     void* ptr = mmap(nullptr, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (ptr == MAP_FAILED) {
//        printf("Failed to map shared memory\n");
//         return nullptr;
//     }

//     vmroot = static_cast<sigil::vmnode_t*>(ptr);
//     //return shm_fd;

//     vmroot->name.value = VM_NODE_VMROOT;
//     //vmroot = new sigil::vmnode_t(VM_NODE_VMROOT);
//     vmroot_data = new system::vmroot_data_t;
//     vmroot->set_data(vmroot_data, cleanup_vmroot_data);
    
//     runtime = vmroot->spawn_subnode(VM_NODE_RUNTIME);
//     runtime_data = new system::runtime_data_t;
//     runtime->set_data(runtime_data, cleanup_runtime_data);

//     platform = vmroot->spawn_subnode(VM_NODE_PLATFORM);
//     platform_data = new system::platform_data_t;
//     platform->set_data(platform_data, cleanup_platform_data);
     
//     return vmroot;
// }



// sigil::status_t system::exec(std::string &payload) {
//     //printf("Got command: %s\n", payload.c_str());
    
//     if (payload.compare("exit") == 0) {
//         return sigil::VM_SYSTEM_SHUTDOWN;
//     }

//     if (payload.compare("disable root") == 0) {
//         system::invalidate_root();
//         return sigil::VM_SYSTEM_SHUTDOWN;
//     }

//     if (payload.compare("treeinfo") == 0) {
//         vmroot->print_nodeinfo();
//         return sigil::VM_OK;    
//     }

//     // if (payload.compare("memstats") == 0) {
//     //     memory::print_mem_report();
//     //     return sigil::VM_OK;    
//     // }
    
//     if (payload.compare("cookie") == 0) {
//        printf("Previous cookie: %x\n", vmroot_data->cookie);
//         std::random_device rd;

//         // Create a Mersenne Twister generator seeded with the random device
//         std::mt19937 gen(rd());

//         // Define a uniform distribution in the range of uint32_t
//         std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);

//         // Generate a random uint32_t value
//         uint32_t random_value = dis(gen);
//         vmroot_data->cookie = random_value;
//        printf("New cookie: %x\n", vmroot_data->cookie);
//         return sigil::VM_OK;    
//     }

//     return sigil::VM_OK;
// }



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

void sigil::memview(const void *mem_start, uint32_t num_bytes, bool align) {
    if (!mem_start) {
       printf("mem-view: cannot dereference NULL\n");
        return;
    }

    if (num_bytes == 1) {;
        char byte = ' ';
        char byte_at_addr = *(char*)mem_start;

        if (byte_at_addr > 31 && byte_at_addr < 127) byte = byte_at_addr & 0xFF;

       printf("[%p]: 0x%02x [%c]\n", mem_start, byte, byte);
        return;
    }

    uint32_t align_offset = 0xF * (uint32_t)align;
    const char *printout_start = (const char*)mem_start - align_offset;
    for (int i = 0; i < num_bytes; i++) if (!(i % 16)) memview_chunk(printout_start + i);
}

void sigil::get_memstats(sigil::memstat_t &result) {
#   ifdef __linux__
    const char* memstats_path = "/proc/self/statm";

    FILE *f = fopen(memstats_path,"r");
    if (!f) return;

    fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
            &result.size, &result.resident, &result.share,
            &result.text, &result.lib, &result.data, &result.dt);
    
    fclose(f);
#   endif /* __linux__ */
}

void sigil::print_memstats(const memstat_t &result) {
   printf("Linux memory report: Lib: %ld MB, Share: %ld MB, Resident: %ld MB\n",
        result.lib / 1024, result.share / 1024, result.resident / 1024
    );
   printf("Linux memory report: Size: %ld MB, Data: %ld MB, DT: %ld MB\n",
        result.size / 1024, result.data / 1024, result.dt / 1024
    );
}

// void print_mem_report() {
//     get_memstats(memory_node_data->process_memstats);
//     print_memstats(memory_node_data->process_memstats);
// }



// Print next 16 bytes in memory overwiev style 
void sigil::memview_chunk(const void *mem_start) {
    if (!mem_start) return;

   printf("[%p]  ", mem_start);

    for (int i = 0; i < 16; i++) {
       printf("%02x ", ((char*)mem_start)[i] & 0xff);
    }

    for (int i = 0; i < 16; i++) {
        char byte = '.';

        if (((char*)mem_start)[i] > 31 && ((char*)mem_start)[i] < 127) byte = ((char*)mem_start)[i];
       printf("%c", byte & 0xff);
    }

   printf("\n");
}
