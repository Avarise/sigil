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
#include "system.h"
#include "utils.h"


namespace sigil::iocommon {
    // VM Tree API
    status_t start();
    status_t stop();

    // Hardware info, checking for device presence
    void print_power_report();
    void print_platform_info();
    status_t probe_lcd();
}