#pragma once
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <chrono>


namespace sigil::utils {
    // Generic timer to measure functions
    class exec_timer {
        public:
        void start() {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

        void stop() {
            stop_time_ = std::chrono::high_resolution_clock::now();
        }

        // Get elapsed time in nanoseconds
        uint64_t ns() const {
            return static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time_ - start_time_).count()
            );
        }

        // Get elapsed time in microseconds
        uint64_t us() const {
            return static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(stop_time_ - start_time_).count()
            );
        }

        // Get elapsed time in milliseconds
        uint64_t ms() const {
            return static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(stop_time_ - start_time_).count()
            );
        }

        private:
        std::chrono::high_resolution_clock::time_point start_time_;
        std::chrono::high_resolution_clock::time_point stop_time_;
    };


    void xor_encode(FILE *input, FILE *output, uint32_t key);
}