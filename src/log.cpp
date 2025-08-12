#include "log.hpp"

#include <iostream>

// ANSI escape codes for colors
#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_CYAN   "\033[36m"

// Unicode symbols for prefixes
#define ERROR_LOG_PREFIX   "✗"
#define WARNING_LOG_PREFIX "⚠"
#define INFO_LOG_PREFIX    "ℹ"

void
log(LogType type, std::string_view msg) {
    const char* color;
    const char* prefix;

    switch (type) {
    case LogType::ERROR:
        color = COLOR_RED;
        prefix = ERROR_LOG_PREFIX;
        break;
    case LogType::WARNING:
        color = COLOR_YELLOW;
        prefix = WARNING_LOG_PREFIX;
        break;
    case LogType::INFO:
        color = COLOR_GREEN;
        prefix = INFO_LOG_PREFIX;
        break;
    }

    std::cout << COLOR_CYAN << " " << color << prefix << COLOR_CYAN <<
              " " << COLOR_RESET << msg << std::endl;
}

void
log(LogType type, const std::string& msg1, const std::string& msg2) {
    log(type, msg1 + " " + msg2);
}
