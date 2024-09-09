#pragma once
/*
    Header for common IO interface.
    Platform functions:
        - create IO node for buttons etc
        - extend platform with power reports etc
*/
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstdio>
#include "core.h"
#include "node.h"


namespace sigil::iocommon {
    struct iodata_t {
        //std::vector<sigil::window_t*> windows;
        //bool glfw_initialized = false;
    };

    // VM Tree API
    status_t initialize(sigil::vmnode_t *vmsr);
    sigil::vmnode_t probe(sigil::vmnode_t *vmsr);
    status_t deinitialize();
    iodata_t* api_handle(sigil::vmnode_t *vmsr);
    


    // Window Management API
    // sigil::window_t*   spawn_window(const char *window_name); //Returns handle
    // sigil::window_t*   get_window(int index); //Returns handle
    // status_t        destroy_window(sigil::window_t *window);
    // status_t        attach_io_to_window(sigil::window_t *window);
    // status_t        present_window(sigil::window_t *window);
    // status_t        start_gui(sigil::window_t *window);
    
    // status_t initialize_window(sigil::window_t *window);
    // //sigil::status_t attach_vk_to_window(sigil::window_t *window);
    // status_t set_window_presentation_mode(sigil::window_t *window,
    //                                 sigil::window_t::presentation_t mode);
    
    // std::vector<sigil::window_t*>* get_window_table();

    void print_power_report();
//     void print_power_report()  {
//     //     printf("### Battery ###\n");
//     // #   ifdef __linux__
//     //     std::ifstream energy_full;
//     //     energy_full.open("/sys/class/power_supply/BAT0/energy_full");
//     //     if (!energy_full.is_open()) return;

//     //     std::string energy_full_val = "";
//     //     std::getline(energy_full, energy_full_val);
//     //     printf("Energy max: %d\n", std::stoi(energy_full_val));
//     //     energy_full.close();
//     // #   endif
// }
}