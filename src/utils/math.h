#pragma once
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