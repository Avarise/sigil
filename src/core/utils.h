/*
    Collection of misc functions
    Dependency for system.h
*/
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <cmath>
#include <vector>

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

/* Memory helpers */
#define HASH_MASK 0xDEADBEEF
#define MSIZE_4K 4096
#define MMASK_4K 0xFFFFF000
#define MSIZE_PAGE 512
#define MMASK_PAGE 0xFFFFFE00
#define MSIZE_STRING 64
#define MMASK_STRING 0xFFFFFFC0
#define MSIZE_CHUNK 16
#define MMASK_CHUNK 0xFFFFFFF0
#define MAX_THREADS 128
#ifndef BIT
#define BIT(x) ((uint32_t)1 << x)
#endif /* BIT */




namespace sigil {
    enum status_t {
        VM_OK                       = 0,
        VM_IDLE                     = BIT(0),
        VM_BUSY                     = BIT(1),
        VM_LOCKED                   = BIT(2),
        VM_FAILED                   = BIT(3),
        VM_SKIPPED                  = BIT(4),
        VM_ARG_NULL                 = BIT(5),
        VM_NOT_FOUND                = BIT(6),
        VM_ARG_INVALID              = BIT(7),
        VM_FAILED_ALLOC             = BIT(8),
        VM_INVALID_ROOT             = BIT(9),
        VM_NOT_SUPPORTED            = BIT(10),
        VM_ALREADY_EXISTS           = BIT(11),
        VM_SYSTEM_SHUTDOWN          = BIT(12),
        VM_UNKNOWN_SUCCESS          = BIT(13),
        VM_NOT_IMPLEMENTED          = BIT(14),
        VM_SWAPCHAIN_REBUILDING     = BIT(15),
        VM_DISABLED,

    };

    inline status_t operator|(const status_t& lhs, const status_t& rhs) {                                             
        using T = std::underlying_type_t<status_t>;
        return static_cast<status_t>(static_cast<T>(lhs) | static_cast<T>(rhs)); 
    }                                   

    inline status_t operator&(const status_t& lhs, const status_t& rhs) {                                                                   
        using T = std::underlying_type_t<status_t>;                              
        return static_cast<status_t>(static_cast<T>(lhs) & static_cast<T>(rhs)); 
    }


    inline const char* status_to_cstr(status_t status);
    // Timers
    struct sync_data_t;

    // Various
    // Write a format string with arguments into a std::string reference
    // returns number of bytes written
    int insert_into_string(std::string &target, const char *format, ...);
    void print_binary(uint32_t num);
    void xor_encode(FILE *input, FILE *output, uint32_t key);

    struct sync_data_t {
        uint64_t iters; // Iterations of engine
        uint32_t target_render_rate; // Render rate in HZ, 0 for unlimited
        double delta_us; // 10e-6 second delta time 
        std::chrono::time_point<std::chrono::high_resolution_clock> ts_render_end; // Timestamp of last render end
    };

    struct name_t {
        std::string value;
    };

    struct memstat_t {
        std::string command;
        uint64_t vsize;
        uint64_t rss;
        uint32_t pid;
        char state;
    };

    void get_memstats(memstat_t &result);
    void print_memstats(const memstat_t &result);
    void print_mem_report();

    // Byte viewer, chuck = 16 bytes
    void memview_chunk(const void *mem_start);
    void memview(const void *mem_start, uint32_t num_bytes, bool align);
    

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

    inline uint32_t random_u32_scoped(uint32_t min, uint32_t max) {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint32_t> dist(min, max);
        return dist(rng);
    }

    inline int32_t random_i32_scoped(int32_t min, int32_t max) {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int32_t> dist(min, max);
        return dist(rng);
    }

    inline void u32swap(uint32_t a, uint32_t b) {
#       ifndef USE_NO_COPY_SWAP
        uint32_t temp = a;
        a = b;
        b = temp;
#       else /* USE COPY SWAP*/
        a = a ^ b;
        b = a ^ b;
        a = a ^ b;
#       endif
    }


    inline void sleep_ms(uint32_t ms) {
        if (ms == 0) return;

#       ifdef __linux__
        timespec ts;
        ts.tv_sec = (ms / 1000);
        ts.tv_nsec = (ms % 1000) * 10e6;

        int res = 0;
        do {
            res = nanosleep(&ts, &ts);
        } while (res && errno == EINTR);

#       else
        return;
#       endif
    }

    inline void boolflip(bool &val) {
        val ^= 1;
    }

    inline void bitflip(uint32_t &bitarray, int bitidx) {
        bitarray ^= 1 << bitidx;
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

    struct argmock {
        int argc = 0;
        char **argv;
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

    sigil::status_t list_dir_content(const char* path);
}

inline const char* sigil::status_to_cstr(sigil::status_t status) {
    switch (status) {
        case VM_OK: return "VM_OK";
        case VM_IDLE: return "VM_IDLE";
        case VM_BUSY: return "VM_BUSY";
        case VM_LOCKED: return "VM_LOCKED";
        case VM_FAILED: return "VM_FAILED";
        case VM_ARG_NULL: return "VM_ARG_NULL";
        case VM_NOT_FOUND: return "VM_NOT_FOUND";
        case VM_FAILED_ALLOC: return "VM_FAILED_ALLOC";
        case VM_INVALID_ROOT: return "VM_INVALID_ROOT";
        case VM_NOT_SUPPORTED: return "VM_NOT_SUPPORTED";
        case VM_ALREADY_EXISTS: return "VM_ALREADY_EXISTS";
        case VM_SYSTEM_SHUTDOWN: return "VM_SYSTEM_SHUTDOWN";
        default: return "VM_UNKNOWN";
    }
}