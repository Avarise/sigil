#include <cstdlib>
#include <cstring>
#include "../utils/generic.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s path=<filepath> key=<4-byte hex key>\n", argv[0]);
        return 1;
    }

    char *path = strchr(argv[1], '=');
    char *keyStr = strchr(argv[2], '=');

    if (!path || !keyStr) {
        fprintf(stderr, "Error: Invalid arguments\n");
        return 1;
    }

    path++;
    keyStr++;

    uint32_t key = (uint32_t)strtoul(keyStr, NULL, 16);

    FILE *input = fopen(path, "rb");
    if (!input) {
        perror("Error opening input file");
        return 1;
    }

    char outputPath[512];
    snprintf(outputPath, sizeof(outputPath), "%s.enc", path);

    FILE *output = fopen(outputPath, "wb");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    sigil::utils::xor_encode(input, output, key);

    fclose(input);
    fclose(output);
    return 0;
}
