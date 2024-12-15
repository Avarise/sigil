#include "node.h"
#include <cstring>

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
        this->node_data.data);
    
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
    if (this->refcount == 0) this->node_data.data_cleanup();
    if (master) master->release();
    return sigil::VM_OK;
}