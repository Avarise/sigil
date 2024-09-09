#pragma once

namespace sigil::rng {
    enum dice_types {
        D2, D4, D6, D8, D10, D12, D20, D100,
    };

    static const char* dice_type_to_str(dice_types type) {
        if (type == D4) return "d4";
        if (type == D6) return "d6";
        if (type == D8) return "d8";
        if (type == D10) return "d10";
        if (type == D12) return "d12";
        if (type == D20) return "d20";
        return nullptr;
    }
}