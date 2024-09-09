#include <cerrno>
#include "ntt.h"
#include "../vm/system.h"

using namespace sigil;

static sigil::vmnode_t *ntt_host_node = nullptr;
static sigil::vmnode_t *ntt_store_node = nullptr;
static ntt::host_data_t *ntt_host_data = nullptr;
static ntt::store_data_t *ntt_store_data = nullptr;

static void cleanup_ntt_host_data() {
    if (!ntt_host_node) return;
    if (!ntt_host_data) return;
    delete ntt_host_data;
    ntt_host_data = nullptr;
    ntt_host_node->node_data.data = nullptr;
}

static void cleanup_ntt_store_data() {
    if (!ntt_store_node) return;
    if (!ntt_store_data) return;
    delete ntt_store_data;
    ntt_store_data = nullptr;
    ntt_store_node->node_data.data = nullptr;
}

sigil::status_t ntt::initialize(sigil::vmnode_t *vmsr) {
    if (!vmsr) return sigil::VM_ARG_NULL;
    if (system::validate_root(vmsr) != 0) return sigil::VM_INVALID_ROOT;

    sigil::vmnode_t *platform = vmsr->peek_subnode("runtime", 1);
    if (!platform) return sigil::VM_NOT_FOUND;

    ntt_host_node = platform->spawn_subnode("ntt-host");
    if (!ntt_host_node) return sigil::VM_FAILED_ALLOC;

    ntt_store_node = platform->spawn_subnode("ntt-store");
    if (!ntt_store_node) return sigil::VM_FAILED_ALLOC;

    ntt_host_data = new host_data_t;
    ntt_host_node->set_data(ntt_host_data, cleanup_ntt_host_data);

    ntt_store_data = new store_data_t;
    ntt_store_node->set_data(ntt_store_data, cleanup_ntt_store_data);
    return sigil::VM_OK;
}

sigil::ntt::scene_t* ntt::spawn_scene() {
    sigil::ntt::scene_t *new_scene = new sigil::ntt::scene_t;
    ntt_host_data->scenes.push_back(new_scene);
    return new_scene;
}