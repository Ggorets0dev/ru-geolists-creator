#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>

#define FILE_OPEN_ERROR_MSG     "Failed to open file on path: "
#define JSON_PARSE_ERROR_MSG    "Failed to parse JSON file on path: "
#define SOFTWARE_INIT_FAIL_MSG  "Failed to initialize software with its components"
#define CHECK_UPDATES_FAIL_MSG  "Failed to check sources for updates"

#define ERROR_LOG_PREFIX        "-"
#define WARNING_LOG_PREFIX      "!"
#define INFO_LOG_PREFIX         "+"

#define LOG_ERROR(msg)          log(LogType::ERROR, msg)
#define LOG_WARNING(msg)        log(LogType::WARNING, msg)
#define LOG_INFO(msg)           log(LogType::INFO, msg)

enum LogType {
    ERROR,
    WARNING,
    INFO
};

extern void
log(LogType type, std::string_view msg);

extern void
log(LogType type, const std::string& msg1, const std::string& msg2);

#endif // LOG_HPP
