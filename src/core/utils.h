#pragma once
#include <cstdint>
#include <string>
#include <chrono>

namespace sigil::utils {
    struct sync_data_t {
        uint64_t iters; // Iterations of engine
        uint32_t target_render_rate; // Render rate in HZ, 0 for unlimited
        double delta_us; // 10e-6 second delta time 
        std::chrono::time_point<std::chrono::high_resolution_clock> ts_render_end; // Timestamp of last render end
    };
        // Write a format string with arguments into a std::string reference
    // returns number of bytes written
    int insert_into_string(std::string &target, const char *format, ...);
    void print_binary(uint32_t num);

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

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

namespace sigil {
    inline uint32_t random_u32_scoped(uint32_t min, uint32_t max) {
        uint32_t num = 0;
        num = ((uint32_t)std::rand() % (max + 1)) + min;
        return num;
    }

    inline int32_t random_i32_scoped(int32_t min, int32_t max) {
        int32_t num = 0;
        num = ((int32_t)std::rand() % (max + 1)) + min;
        return num;
    }

    struct v3_t {
        float x, y, z;

        v3_t() {
            x = 0.0f; y = 0.0f; z = 0.0f;
        }
        
        v3_t(float x, float y, float z) {
            x = x; y = y; z = z;
        }

        float abs() {
            float len = 0.0f;
            len += x * x; 
            len += y * y;
            len += z * z;
            return sqrtf(len);
        }
    };

    struct v4_t {
        float x, y, z, w;

        v4_t() {
            x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
        }
        
        v4_t(float x, float y, float z, float w) {
            x = x; y = y; z = z; w = w;
        }

        float abs() {
            float len = 0.0f;
            len += x * x; 
            len += y * y;
            len += z * z;
            len += w * w;
            return sqrtf(len);
        }
    };

    struct quat_t {
        float r, i, j, k;

        quat_t() {
            r = 0.0f; i = 0.0f; j = 0.0f; k = 0.0f;
        }

        quat_t(float r, float i, float j, float k) {
            r = r; i = i; j = j; k = k;
        }
    };

    struct transform3d {
        v3_t translation, scale;
        quat_t rotation;

        transform3d() {
            translation.x = 0.0f;
            translation.y = 0.0f;
            translation.z = 0.0f;
            rotation.r = 0.0f;
            rotation.i = 0.0f;
            rotation.j = 0.0f;
            rotation.k = 0.0f;
            scale.x = 1.0f;
            scale.y = 1.0f;
            scale.z = 1.0f;
        }
    };

    inline uint64_t u64factorial(uint64_t n) {
        uint64_t ans = 1;
        for (int i = 1; i <= n; i++) ans *= i;
        return ans;
    }


    inline uint64_t u64binomial(uint64_t n, uint64_t k) {
        if (k > n) return 0;
        if (k == n || k == 0) return 1;
        return u64factorial(n) / (u64factorial(n - k) * u64factorial(k));
    }


    inline uint32_t get_gcd(uint32_t n1, uint32_t n2) {
        if (n1 > n2) return get_gcd(n1 % n2, n2);
        if (n2 > n1) return get_gcd(n1, n1 % n2);
        return n1;
    }
}