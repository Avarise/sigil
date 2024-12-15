#include <cerrno>
#include "system.h"
#include "ntt.h"

static sigil::vmnode_t *ntt_store_node = nullptr;
static sigil::vmnode_t *ntt_host_node = nullptr;
std::vector<sigil::ntt::scene_t*> scenes;
uint32_t num_engines;


sigil::status_t sigil::ntt::initialize() {
    
    return sigil::VM_OK;
}

sigil::ntt::scene_t* sigil::ntt::spawn_scene() {
    sigil::ntt::scene_t *new_scene = new sigil::ntt::scene_t;
    scenes.push_back(new_scene);
    return new_scene;
}

inline void sigil::ntt::engine_t::sync_engine() {
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