#include "sigil.h"

void sigil::console_subprogram() {
    status_t status = VM_OK;

    while (status == sigil::VM_OK) {
        printf("sigil -> ");
        fflush(stdout);
        std::string payload;
        std::cin >> payload;
        status = sigil::system::exec(payload);
        printf("\n");
    }
}