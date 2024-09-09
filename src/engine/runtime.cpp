//#include "../utils/utils.h"
#include "runtime.h"
#include "../vm/core.h"

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