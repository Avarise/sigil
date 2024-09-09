#include <cerrno>
#include "server.h"
#include "../vm/system.h"

using namespace sigil;
static sigil::vmnode_t *vmwebhost_node = nullptr;
static vmwebhost::vmwebhost_data_t *vmwebhost_data = nullptr;

void cleanup_vmwebhost_data() {
    if (!vmwebhost_node) return;
    if (!vmwebhost_data) return;
    delete vmwebhost_data;
    vmwebhost_data = nullptr;
    vmwebhost_node->node_data.data = nullptr;
}

sigil::status_t sigil::vmwebhost::initialize(sigil::vmnode_t *vmsr) {
    if (!vmsr) return sigil::VM_ARG_NULL;
    if (system::validate_root(vmsr) != 0) return sigil::VM_INVALID_ROOT;

    sigil::vmnode_t *runtime = vmsr->peek_subnode("runtime", 1);
    if (!runtime) return sigil::VM_INVALID_ROOT;

    vmwebhost_node = runtime->peek_subnode("vmwebhost", 32);

    if (vmwebhost_node != nullptr) {
        vmwebhost_data = (vmwebhost_data_t*)vmwebhost_node->node_data.data;
        return sigil::VM_ALREADY_EXISTS;
    }
    
    vmwebhost_node = runtime->spawn_subnode("vmwebhost");
    if (!vmwebhost_node) return sigil::VM_FAILED_ALLOC;

    vmwebhost_data = new vmwebhost_data_t;
    vmwebhost_node->set_data(vmwebhost_data, cleanup_vmwebhost_data);
    return sigil::VM_OK;
}