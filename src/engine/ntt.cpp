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

inline void sigil::engine_t::sync_engine() {
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    long int target_frame_us = 0;

    timestamp = std::chrono::high_resolution_clock::now();

    if (this->target_render_rate > 0) {
        // Converting FPS into microsecond frame time
        target_frame_us = 1000000 / this->target_render_rate;
        this->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                    (timestamp - this->ts_render_end).count();
        if (this->delta_us < target_frame_us) {
            sigil::sleep_ms(target_frame_us - this->delta_us);
            timestamp = std::chrono::high_resolution_clock::now();
            this->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                        (timestamp - this->ts_render_end).count();
        }
    }
    
    else if (this->target_render_rate == 0) {
        this->delta_us = std::chrono::duration_cast<std::chrono::microseconds>
                                    (timestamp - this->ts_render_end).count();
    }

    this->ts_render_end = timestamp;
}