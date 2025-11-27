#include "log.hpp"
#include "fs_utils.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// ====================
// Settings for custom behavior
// ====================
#define RESOLVE_PROGRESS_STEP 10.0f
constexpr float gkResolveProgressStep = RESOLVE_PROGRESS_STEP / 100.0f;
// ====================

const fs::path gkLogPath = fs::path(std::getenv("HOME")) / ".local" / "share" / "rglc" / "logs" / "rglc.log";

struct OutputMessagesTarget {
    int stdoutCode;
    int stderrCode;
    bool isSupressed;
};

static OutputMessagesTarget gMsgTarget = {
    .isSupressed = false
};


void initLogging() {
    try {
        // Инициализация thread pool для async
        spdlog::init_thread_pool(8192, 1);

        // File sink: 5 MB max, 3 файла ротации
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            gkLogPath,
            5 * 1024 * 1024,
            3
        );

        // Console sink: цветной вывод
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // Общий async логгер
        auto logger = std::make_shared<spdlog::async_logger>(
            "RGLC",
            spdlog::sinks_init_list{console_sink, file_sink},
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        spdlog::set_default_logger(logger);

        // Формат сообщений
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

        // Уровень по умолчанию
        spdlog::set_level(spdlog::level::info);

        // Ошибки — сразу флешить
        spdlog::flush_on(spdlog::level::err);

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "spdlog init failed: " << ex.what() << std::endl;
        exit(1);
    }
}


// ====================
// Your old wrappers
// ====================

void logUrlAccess(const std::string& url, bool status) {
    if (status)
        LOG_INFO("Successfully accessed resource: " + url);
    else
        LOG_ERROR("Failed to access resource: " + url);
}

void logFilterCheckProgress(float progress) {
    static float prevProgress = 0.0f;

    if (prevProgress > progress)
        prevProgress = 0.0f;

    if ((progress - prevProgress) >= gkResolveProgressStep) {
        float pct = progress * 100.0f;

        LOG_INFO("File filter-check in progress: {:.1f} %", pct);

        prevProgress = progress;
    }
}

void logWithMark(const std::string& msg, const std::string& mark, uint32_t level) {
    std::string m = mark + "  " + msg;

    switch (level) {
    case 0: // custom error
        spdlog::error(m);
        break;
    case 1: // warn
        spdlog::warn(m);
        break;
    case 2: // info
        spdlog::info(m);
        break;
    default:
        spdlog::info(m);
        break;
    }
}

// ====================
// Console suppression
// ====================
void suppressConsoleOutput() {
    gMsgTarget.stdoutCode = dup(STDOUT_FILENO);
    gMsgTarget.stderrCode = dup(STDERR_FILENO);

    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null < 0) {
        perror("open");
        return;
    }

    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);
    close(dev_null);

    gMsgTarget.isSupressed = true;
}

void restoreConsoleOutput() {
    if (!gMsgTarget.isSupressed) {
        spdlog::warn("Trying to restore messages output that wasn't suppressed");
        return;
    }

    dup2(gMsgTarget.stdoutCode, STDOUT_FILENO);
    dup2(gMsgTarget.stderrCode, STDERR_FILENO);

    close(gMsgTarget.stdoutCode);
    close(gMsgTarget.stderrCode);

    gMsgTarget.isSupressed = false;
}
