#include "generic.h"

void sigil::utils::xor_encode(FILE *input, FILE *output, uint32_t key) {
    uint8_t buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            buffer[i] ^= ((uint8_t *)&key)[i % 4];
        }
        fwrite(buffer, 1, bytesRead, output);
    }
}