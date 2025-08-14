#ifndef LOG_HPP
#define LOG_HPP

#include <string>

#define FILE_LOCATE_ERROR_MSG       "Failed to locate file on path: "
#define FILE_OPEN_ERROR_MSG         "Failed to open file on path: "
#define JSON_PARSE_ERROR_MSG        "Failed to parse JSON file on path: "
#define SOFTWARE_INIT_FAIL_MSG      "Failed to initialize software with its components"
#define CHECK_UPDATES_FAIL_MSG      "Failed to check sources for updates"
#define DOWNLOAD_UPDATES_FAIL_MSG   "Failed to download latest releases of main sources"

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

extern void
log_url_access(const std::string& url, bool status);

#endif // LOG_HPP
