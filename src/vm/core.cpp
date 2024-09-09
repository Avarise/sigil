#include <cstddef>
#include <cstdio>
#include <cstring>
#include "core.h"
#include "node.h"

using namespace sigil;


void sigil::memview(const void *mem_start, uint32_t num_bytes, bool align) {
    if (!mem_start) {
       printf("mem-view: cannot dereference NULL\n");
        return;
    }

    if (num_bytes == 1) {;
        char byte = ' ';
        char byte_at_addr = *(char*)mem_start;

        if (byte_at_addr > 31 && byte_at_addr < 127) byte = byte_at_addr & 0xFF;

       printf("[%p]: 0x%02x [%c]\n", mem_start, byte, byte);
        return;
    }

    uint32_t align_offset = 0xF * (uint32_t)align;
    const char *printout_start = (const char*)mem_start - align_offset;
    for (int i = 0; i < num_bytes; i++) if (!(i % 16)) memview_chunk(printout_start + i);
}

void sigil::get_memstats(sigil::memstat_t &result) {
#   ifdef __linux__
    const char* memstats_path = "/proc/self/statm";

    FILE *f = fopen(memstats_path,"r");
    if (!f) return;

    fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
            &result.size, &result.resident, &result.share,
            &result.text, &result.lib, &result.data, &result.dt);
    
    fclose(f);
#   endif /* __linux__ */
}

void sigil::print_memstats(const memstat_t &result) {
   printf("Linux memory report: Lib: %ld MB, Share: %ld MB, Resident: %ld MB\n",
        result.lib / 1024, result.share / 1024, result.resident / 1024
    );
   printf("Linux memory report: Size: %ld MB, Data: %ld MB, DT: %ld MB\n",
        result.size / 1024, result.data / 1024, result.dt / 1024
    );
}

// void print_mem_report() {
//     get_memstats(memory_node_data->process_memstats);
//     print_memstats(memory_node_data->process_memstats);
// }



// Print next 16 bytes in memory overwiev style 
void sigil::memview_chunk(const void *mem_start) {
    if (!mem_start) return;

   printf("[%p]  ", mem_start);

    for (int i = 0; i < 16; i++) {
       printf("%02x ", ((char*)mem_start)[i] & 0xff);
    }

    for (int i = 0; i < 16; i++) {
        char byte = '.';

        if (((char*)mem_start)[i] > 31 && ((char*)mem_start)[i] < 127) byte = ((char*)mem_start)[i];
       printf("%c", byte & 0xff);
    }

   printf("\n");
}
