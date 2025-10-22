#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

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

#define LOG_MARK_ERROR      "❌"
#define LOG_MARK_WARN       "⚠️"
#define LOG_MARK_INFO       "ℹ️"

// Set special emojis before texts in logs
// #define USE_EMOGI_MARKERS

#ifdef USE_EMOGI_MARKERS
#define LOG_ERROR(msg)          logWithMark(msg, LOG_MARK_ERROR, log4cxx::Level::ERROR_INT)
#define LOG_WARNING(msg)        logWithMark(msg, LOG_MARK_WARN, log4cxx::Level::WARN_INT)
#define LOG_INFO(msg)           logWithMark(msg, LOG_MARK_INFO, log4cxx::Level::INFO_INT)
#else
#define LOG_ERROR(msg)          LOG4CXX_ERROR(gLogger, msg)
#define LOG_WARNING(msg)        LOG4CXX_WARN(gLogger, msg)
#define LOG_INFO(msg)           LOG4CXX_INFO(gLogger, msg)
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;

extern LoggerPtr gLogger;

void initLogging();

void logUrlAccess(const std::string& url, bool status);

void logFilterCheckProgress(float progress);

void logWithMark(const std::string& msg, const std::string& mark, uint32_t level);

void suppressConsoleOutput();

void restoreConsoleOutput();

#endif // LOG_HPP
