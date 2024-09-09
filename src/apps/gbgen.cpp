/*
    Generate 1GB file filled with random data
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE (1L << 30) // 1GB in bytes <<30
#define CHARSET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\n\n\n\n"
#define CHARSET_SIZE (sizeof(CHARSET) - 1)

int main() {
    FILE *file;
    const char *filename = "random_data.txt";
    size_t total_bytes_written = 0;

    // Seed random number generator
    srand(time(NULL));

    // Open file for writing in binary mode
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf("Creating 1GB file: %s\n", filename);

    // Write random data until the file reaches 1GB
    while (total_bytes_written < FILE_SIZE) {
        char random_char = CHARSET[rand() % CHARSET_SIZE];
        fputc(random_char, file);
        total_bytes_written++;
        
        // Optional: Print progress every 100MB
        if (total_bytes_written % (100L << 20) == 0) {
            printf("Progress: %zu MB written\n", total_bytes_written >> 20);
        }
    }

    // Close the file
    fclose(file);

    printf("File creation complete: %s (1GB)\n", filename);
    return 0;
}
