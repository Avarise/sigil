#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>  // For sysconf(_SC_PAGESIZE)

#include "system.h"
#include "utils.h"


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

bool ext_load_failed(sigil::status_t status) {
    if (status == sigil::VM_OK || status == sigil::VM_ALREADY_EXISTS) return false;
    return true;
}

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
    std::ifstream file("/proc/self/stat");
    if (!file) {
        throw std::runtime_error("Failed to open /proc/self/stat");
    }

    std::string data;
    std::getline(file, data);
    std::istringstream stream(data);


    // Read first three fields manually
    stream >> result.pid;

    std::string comm;
    stream >> comm;
    while (comm.back() != ')') {
        std::string part;
        stream >> part;
        comm += " " + part;
    }
    result.command = comm.substr(1, comm.size() - 2);  // Remove parentheses

    stream >> result.state;  // Read process state

    // Skip to the 23rd and 24th fields
    for (int i = 0; i < 19; ++i) {
        std::string skip;
        stream >> skip;
    }

    stream >> result.vsize >> result.rss;  // Read virtual size and RSS

    // END    
#   endif /* __linux__ */
}

void sigil::print_memstats(const memstat_t &result) {
    long page_size = sysconf(_SC_PAGESIZE);
    printf("Virtual Machine memory report:\n");
    printf("PID: %d\n", result.pid);
    printf("Command: %s\n", result.command.c_str());
    printf("State: %c\n", result.state);
    printf("Virtual Memory Size: %lu KiB\n", result.vsize / 1024);
    printf("Resident Set Size: %lu KiB\n", page_size * result.rss / 1024);
}


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
