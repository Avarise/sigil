#pragma once
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <ctime>

#define SIGIL_LOG_BUFF_SIZE 2048
#define SIGIL_LOG_TIMESTAMP_SIZE 128

namespace sigil::log {
    extern bool use_console;
    std::string get_current_time();
    void write_log_file(const char* format, va_list args);
    //void print(const char* format, va_list args);
    //void debug(const char* format, va_list args);
    void print(const char* format, ...);
    void debug(const char* format, ...);

    void tt_end_of_year();

    inline void enable_console() {
        use_console = true;
    }

    inline void disable_console() {
        use_console = false;
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



}