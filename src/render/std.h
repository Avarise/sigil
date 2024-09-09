#pragma once
#include <cstdint>
#include <cstring>

namespace sigil {
    struct color_t {
        uint8_t red, green, blue, alpha;
        color_t() { memset((void*)this, 0, sizeof(*this)); }
    };
}