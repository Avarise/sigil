#pragma once
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>

namespace sigil::utils {
    // Write a format string with arguments into a std::string reference
    // returns number of bytes written
    int insert_into_string(std::string &target, const char *format, ...);
    void print_binary(uint32_t num);
}