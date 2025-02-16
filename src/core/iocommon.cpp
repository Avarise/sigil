#include <cassert>
#include <cstdio>
#include "virtual-machine.h"
#include "iocommon.h"
#include "system.h"
#include "utils.h"

static sigil::vmnode_t *iocommon_node = nullptr;
static bool glfw_initialized = false;

sigil::status_t sigil::iocommon::deinitialize() {
    return VM_OK;
}

sigil::status_t sigil::iocommon::initialize() {
    //status_t status = virtual_machine::is_active();
    //if (status != VM_OK) return status;


    return sigil::VM_OK;
}

// TODO: Cleanup cycle tracker
// #   ifdef __esp8266__
// class esp8266_tracker {
// private:
//     uint32_t start_cycles = 0;
//     uint32_t stop_cycles = 0;
//     const char* func_name;

//     uint32_t get_current_cycles() {
//         uint32_t cycles = 0;
//         asm volatile ("rsr %0, CCOUNT" : "=r"(cycles));
//         return cycles;
//     }

// public:
//     esp8266_tracker(const char* name = __PRETTY_FUNCTION__)
//         : func_name(name) {}

//     void start() {
//         start_cycles = get_current_cycles();
//     }

//     void stop() {
//         stop_cycles = get_current_cycles();
//     }

//     uint32_t elapsed_cycles() const {
//         return stop_cycles - start_cycles;
//     }

//     uint32_t millis() const {
//         return (elapsed_cycles() / (ESP8266_CLOCK_SPEED / 1000));
//     }

//     uint32_t micros() const {
//         return (elapsed_cycles() / (ESP8266_CLOCK_SPEED / 1000000));
//     }

//     void show() const {
//         printf("%s measured %u ms (%u CPU cycles)\n", func_name, millis(), elapsed_cycles());
//     }
// };
// #   endif