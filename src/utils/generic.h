#pragma once
#include <cstdint>
#include <cstdio>

namespace sigil::utils {
    void xor_encode(FILE *input, FILE *output, uint32_t key);
}