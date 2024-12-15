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
#include "utils.h"


namespace sigil::iocommon {
    // VM Tree API
    status_t initialize();
    status_t deinitialize();

    // Hardware info, checking for device presence
    void print_platform_info();
    void print_power_report();
    status_t probe_lcd();
}