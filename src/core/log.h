#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <ctime>

#define SIGIL_LOG_BUFF_SIZE 2048
#define SIGIL_LOG_TIMESTAMP_SIZE 128

namespace sigil::log {
    // printf variants
    void write_log_file(const char* format, va_list args);
    void debug(const char* format, va_list args);

    // other printouts
    std::string get_current_time();
    void tt_end_of_year();
}