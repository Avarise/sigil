#include <cassert>
#include <cstdio>
#include "iocommon.h"
#include "system.h"

using namespace sigil;

static sigil::vmnode_t *iocommon_node = nullptr;
static iocommon::iodata_t *iocommon_data = nullptr;

static void cleanup_iocommon_data() {
    if (!iocommon_node) return;
    if (!iocommon_data) return;
    delete iocommon_data;
    iocommon_data = nullptr;
    iocommon_node->node_data.data = nullptr;
}

sigil::status_t sigil::iocommon::initialize(sigil::vmnode_t *vmsr) {
    if (!vmsr) return sigil::VM_ARG_NULL;
    if (system::validate_root(vmsr) != 0) return sigil::VM_INVALID_ROOT;

    sigil::vmnode_t *platform = vmsr->get_subnode("platform", 1);
    if (!platform) return sigil::VM_NOT_FOUND;

    iocommon_node = platform->spawn_subnode("iocommon");
    if (!iocommon_node) return sigil::VM_FAILED_ALLOC;

    iocommon_data = new iodata_t();
    iocommon_node->set_data(iocommon_data, cleanup_iocommon_data);

    return sigil::VM_OK;
}