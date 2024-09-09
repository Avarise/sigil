#pragma once
#include <cstdlib>
#include "vm/system.h"
#include "vm/core.h"
#include "imgui.h"
namespace sigil {
    // Start the Sigil virtual machine
    inline sigil::status_t
    initialize(int argc, const char *argv[]) {
        vmnode_t *root = nullptr;

        //fakeroot ? root = system::spawn_fakeroot(argc, argv) : root = system::spawn_root(argc, argv);
        root = sigil::system::spawn_fakeroot(argc, argv);
        return root ? VM_OK : VM_INVALID_ROOT; 
    }

    inline sigil::status_t
    rebuild() {
        return VM_NOT_IMPLEMENTED;
    }

    // Stops the Sigil virtual machine
    inline sigil::status_t
    shutdown() {
        return VM_NOT_IMPLEMENTED;
    }

    inline void exit(sigil::status_t status) {
        printf("VM Status: %s (%d), exiting\n", sigil::status_t_cstr(status), status);
        std::exit(0);
    }

    inline sigil::status_t
    start_dwm() {
        printf("Starting dwm\n");
        std::system("startx /opt/sigil/src/scripts/dwm.sh");
        return sigil::VM_OK;
    }

    inline sigil::status_t
    load_fonts(float fontSize, ImGuiIO& io){
        return sigil::VM_NOT_IMPLEMENTED;
    }
}