#pragma once
#include <chrono>
#include <cstdint>
#include "ntt.h"

namespace sigil {
    struct sync_data_t {
        uint64_t iters; // Iterations of engine
        uint32_t target_render_rate; // Render rate in HZ, 0 for unlimited
        double delta_us; // 10e-6 second delta time 
        std::chrono::time_point<std::chrono::high_resolution_clock> ts_render_end; // Timestamp of last render end
    };

    struct engine_t : sync_data_t {
        sigil::ntt::scene_t *target_scene;

        void sync_engine();
    };
}