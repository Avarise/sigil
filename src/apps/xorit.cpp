#include <cstdlib>
#include <cstring>
#include "utils.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s path=<filepath> key=<4-byte hex key>\n", argv[0]);
        return 1;
    }

    char *key_str = strchr(argv[2], '=');
    char *path = strchr(argv[1], '=');

    if (!path || !key_str) {
        fprintf(stderr, "Error: Invalid arguments\n");
        return 1;
    }

    key_str++;
    path++;

    uint32_t key = (uint32_t)strtoul(key_str, NULL, 16);

    FILE *input = fopen(path, "rb");
    if (!input) {
        perror("Error opening input file");
        return 1;
    }

    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s.enc", path);

    FILE *output = fopen(output_path, "wb");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    sigil::xor_encode(input, output, key);

    fclose(input);
    fclose(output);
    return 0;
}
