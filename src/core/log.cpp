#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "log.h"

std::string sigil::log::get_current_time() {
    // Get current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert to time_t to get the HH:MM:SS part
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto local_time = *std::localtime(&time_t_now);

    // Get milliseconds part
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Format the time into a string
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

void sigil::log::tt_end_of_year() {
        // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);

    int current_year = now_tm.tm_year + 1900;

    std::tm start_of_year = {0, 0, 0, 1, 0, current_year - 1900};
    std::tm end_of_year = {0, 0, 0, 31, 11, current_year - 1900};

    auto start_time = std::mktime(&start_of_year);
    auto end_time = std::mktime(&end_of_year);

    double total_seconds = std::difftime(end_time, start_time);
    double elapsed_seconds = std::difftime(now_time_t, start_time);
    double percentage = (elapsed_seconds / total_seconds) * 100.0;

    std::cout << "Current date: " 
              << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "\n";
    std::cout << "Elapsed: " 
              << std::fixed << std::setprecision(2) << percentage << "% of the year\n";
}

// void sigil::log::write_log_file(const char* format, va_list args) {
// #   ifdef __linux__
//     std::ofstream log_file("/sigil/vm/logs/debug.log", std::ios::app);

//     if (log_file.is_open()) {
//         char buffer[SIGIL_LOG_BUFF_SIZE];
//         vsnprintf(buffer, sizeof(buffer), format, args);

//         std::time_t t = std::time(nullptr);
//         char time_buf[SIGIL_LOG_TIMESTAMP_SIZE];
//         std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

//         log_file << "[" << time_buf << "] " << buffer << std::endl;
//         //log_file << "[" << time_buf << "] " << buffer;
//         log_file.close();
//     }
// #   endif
// }

// void sigil::log::print(const char* format, ...) {
//     va_list args;
//     va_start(args, format);

// #   ifdef SIGIL_DEBUG_MODE
//     write_log_file(format, args);
// #   endif

//     if (use_console) printf(format, args);

//     va_end(args);
// }

// void sigil::log::debug(const char* format, ...) {
//     va_list args;
//     va_start(args, format);
// #   ifdef SIGIL_DEBUG_MODE
//     write_log_file(format, args);
//     vprintf(format, args);
// #   endif
//     va_end(args);
// }
