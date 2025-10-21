#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "log.hpp"
#include "fs_utils.hpp"

// ====================
// Settings for C-Ares resolving logs
// ====================
#define RESOLVE_PROGRESS_STEP      10.0f
constexpr float gkResolveProgressStep = RESOLVE_PROGRESS_STEP / 100.0f;
// ====================

struct OutputMessagesTarget {
    int stdoutCode;
    int stderrCode;

    bool isSupressed;
};

LoggerPtr gLogger(Logger::getLogger("RGLC"));

const fs::path gkLogConfigPath = "/etc/ru-geolists-creator/log4cxx.properties";

static OutputMessagesTarget gMsgTarget = {
    .isSupressed = false
};

void initLogging() {
    try {
        PropertyConfigurator::configure(gkLogConfigPath.string());
    } catch (const Exception& e) {
        std::cerr << "Failed to load log4cxx settings: " << e.what() << std::endl;
        exit(1);
    }
}

void logUrlAccess(const std::string& url, bool status) {
    if (status) {
        LOG_INFO("Successfully accessed resource: " + url);
    } else {
        LOG_ERROR("Failed to access resource: " + url);
    }
}

void logResolveProgress(float progress) {
    static float prevProgress;

    std::ostringstream oss;

    if (prevProgress > progress) {
        prevProgress = 0.0f;
    }

    if ((progress - prevProgress) >= gkResolveProgressStep) {
        oss << std::fixed << std::setprecision(2) << progress * 100.0f;

        LOG_INFO("Domain resolving in progress:  " + oss.str() + " %");

        prevProgress = progress;
    }
}

void logWithMark(const std::string& msg, const std::string& mark, uint32_t level) {
    std::string markMsg;
    markMsg.reserve(msg.length() + mark.length() + 1);

    markMsg = mark + "  " + msg;

    switch (level) {
    case log4cxx::Level::ERROR_INT:
        LOG4CXX_ERROR(gLogger, markMsg.c_str());
        break;
    case log4cxx::Level::WARN_INT:
        LOG4CXX_WARN(gLogger, markMsg.c_str());
        break;
    case log4cxx::Level::INFO_INT:
        LOG4CXX_INFO(gLogger, markMsg.c_str());
        break;
    default:
        // Level not supported
        break;
    }
}

void suppressConsoleOutput() {
    // Save original file descriptors for stdout and stderr
    gMsgTarget.stdoutCode = dup(STDOUT_FILENO);
    gMsgTarget.stderrCode = dup(STDERR_FILENO);

    // Open /dev/null for writing
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null < 0) {
        perror("open");
        return;
    }

    // Redirect stdout and stderr to /dev/null
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);
    close(dev_null); // Close /dev/null file descriptor (not needed anymore)
}

void restoreConsoleOutput() {
    if (!gMsgTarget.isSupressed) {
        LOG_WARNING("Trying to restore messages output that wasnt supressed");
        return;
    }

    // Restore stdout and stderr from saved descriptors
    dup2(gMsgTarget.stdoutCode, STDOUT_FILENO);
    dup2(gMsgTarget.stderrCode, STDERR_FILENO);

    // Close saved descriptors
    close(gMsgTarget.stdoutCode);
    close(gMsgTarget.stderrCode);
}
