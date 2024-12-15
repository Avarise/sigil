#include "utils.h"
#include <cstdarg>
#include <memory>

int sigil::insert_into_string(std::string &target, const char *format, ...) {
    // Start with a large buffer for initial formatting
    char buffer[1024];

    // Use variable argument list to format the string
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Check if buffer was large enough, otherwise handle dynamic allocation
    if (written >= static_cast<int>(sizeof(buffer))) {
        // Resize and reformat with a larger buffer if needed
        std::unique_ptr<char[]> dynamicBuffer(new char[written + 1]);
        va_start(args, format);
        vsnprintf(dynamicBuffer.get(), written + 1, format, args);
        va_end(args);
        target += dynamicBuffer.get();
    } else {
        // If within buffer limit, directly append
        target += buffer;
    }

    return written;
}

void sigil::xor_encode(FILE *input, FILE *output, uint32_t key) {
    uint8_t buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            buffer[i] ^= ((uint8_t *)&key)[i % 4];
        }
        fwrite(buffer, 1, bytesRead, output);
    }
}
void sigil::print_binary(uint32_t num) {
    // Iterate over each bit in the 32-bit integer
    for (int i = 31; i >= 0; i--) {
        // Check if the i-th bit is set
        uint32_t mask = 1u << i;
        printf("%c", (num & mask) ? '1' : '0');
    }
    printf("\n"); // Newline for readability
    //     uint32_t n = 0;
    //     for (uint32_t i = 0; i < 32; i++) {
    //         //printf("Before iteration %u: %u\n", i, n);
    //         //print_binary(n);
    //         n = n << (1 & !!(i));
    //         n = n | (uint32_t)1;
    //     }
}