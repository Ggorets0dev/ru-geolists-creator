#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <spdlog/spdlog.h>

// =========================
// Message constants
// =========================
#define FILE_LOCATE_ERROR_MSG       "Failed to locate file on path: "
#define FILE_OPEN_ERROR_MSG         "Failed to open file on path: "
#define JSON_PARSE_ERROR_MSG        "Failed to parse JSON file on path: "
#define SOFTWARE_INIT_FAIL_MSG      "Failed to initialize software with its components"
#define SOFTWARE_DEINIT_FAIL_MSG    "Failed to deinit software, re-init cant be executed"
#define CHECK_UPDATES_FAIL_MSG      "Failed to check sources for updates"
#define DOWNLOAD_UPDATES_FAIL_MSG   "Failed to download latest releases of main sources"
#define REMOVE_EXTRA_FAIL_MSG       "Failed to remove extra source from configuration file"
#define ADD_EXTRA_FAIL_MSG          "Failed to add extra source to configuration file"
#define READ_CFG_FAIL_MSG           "Failed to read config file"
#define WRITE_CFG_FAIL_MSG          "Failed to write config file"
#define GEO_FORMAT_CONVERT_FAIL_MSG "Since the list could not be converted, the requested format was ignored: "

template<typename... Args>
void LOG_ERROR(const std::string& fmt, Args&&... args) {
    spdlog::error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void LOG_WARNING(const std::string& fmt, Args&&... args) {
    spdlog::warn(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void LOG_INFO(const std::string& fmt, Args&&... args) {
    spdlog::info(fmt, std::forward<Args>(args)...);
}

// =========================
// Functions
// =========================

void initLogging();

void loggerFlush();

void logUrlAccess(const std::string& url, bool status);

void logFilterCheckProgress(float progress);

void logWithMark(const std::string& msg, const std::string& mark, uint32_t level);

void suppressConsoleOutput();

void restoreConsoleOutput();

#endif // LOG_HPP
