#pragma once
#include "system.h"
#include "utils.h"
#include "parser.h"

namespace sigil::virtual_machine {
    status_t initialize(int arg, const char *argv[]);
    status_t deinitialize();
    vmnode_t* get_root_node();
    vmnode_t* get_runtime_node();
    vmnode_t* get_platform_node();
    status_t run_command(sigil::parser::command_t &command);
    status_t add_node(vmnode_t *node /*start, stop*/ );
    status_t remove_node(vmnode_t *node);
    status_t flush();
    status_t reset();
    void console_subprogram();
}

#   ifdef __esp8266__
    class esp8266_tracker {
    private:
        uint32_t start_cycles = 0;
        uint32_t stop_cycles = 0;
        const char* func_name;

        uint32_t get_current_cycles() {
            uint32_t cycles = 0;
            asm volatile ("rsr %0, CCOUNT" : "=r"(cycles));
            return cycles;
        }

    public:
        esp8266_tracker(const char* name = __PRETTY_FUNCTION__)
            : func_name(name) {}

        void start() {
            start_cycles = get_current_cycles();
        }

        void stop() {
            stop_cycles = get_current_cycles();
        }

        uint32_t elapsed_cycles() const {
            return stop_cycles - start_cycles;
        }

        uint32_t millis() const {
            return (elapsed_cycles() / (ESP8266_CLOCK_SPEED / 1000));
        }

        uint32_t micros() const {
            return (elapsed_cycles() / (ESP8266_CLOCK_SPEED / 1000000));
        }

        void show() const {
            printf("%s measured %u ms (%u CPU cycles)\n", func_name, millis(), elapsed_cycles());
        }
    };
#   endif