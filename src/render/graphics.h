#pragma once
#include <cstring>
#include <cstdint>

namespace sigil::graphics {
    struct color_t {
        uint8_t red, green, blue, alpha;
        color_t() { memset((void*)this, 0, sizeof(*this)); }
    };
} 