#include "print_about_handler.hpp"
#include "software_info.hpp"

#include <iostream>
#include <iomanip>

// --- Platform ---
#if defined(_WIN32) || defined(_WIN64)
#define RGC_PLATFORM "Windows"
#elif defined(__APPLE__) || defined(__MACH__)
#define RGC_PLATFORM "macOS"
#elif defined(__linux__)
#define RGC_PLATFORM "Linux"
#else
#define RGC_PLATFORM "Unknown"
#endif

// --- ARCH ---
#if defined(__x86_64__) || defined(_M_X64)
#define RGC_ARCH "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
#define RGC_ARCH "x86"
#elif defined(__aarch64__)
#define RGC_ARCH "ARM64"
#elif defined(__arm__)
#define RGC_ARCH "ARM"
#else
#define RGC_ARCH "Unknown"
#endif

// If SHA was not thrown using build, then it is unknown
#ifndef RGC_GIT_COMMIT_SHA
#define RGC_GIT_COMMIT_SHA "Unknown"
#endif

void printAbout() {
    std::cout << "============================================================" << std::endl;
    std::cout << " /$$$$$$$   /$$$$$$  /$$        /$$$$$$                     " << std::endl;
    std::cout << "| $$__  $$ /$$__  $$| $$       /$$__  $$                    " << std::endl;
    std::cout << "| $$  \\ $$| $$  \\__/| $$      | $$  \\__/                    " << std::endl;
    std::cout << "| $$$$$$$/| $$ /$$$$| $$      | $$                          " << std::endl;
    std::cout << "| $$__  $$| $$|_  $$| $$      | $$                          " << std::endl;
    std::cout << "| $$  \\ $$| $$  \\ $$| $$      | $$    $$                    " << std::endl;
    std::cout << "| $$  | $$|  $$$$$$/| $$$$$$$$|  $$$$$$/                    " << std::endl;
    std::cout << "|__/  |__/ \\______/ |________/ \\______/                     " << std::endl;
    std::cout << "============================================================" << std::endl;

    std::cout << std::left;
    std::cout << "| " << std::setw(15) << "Name:"          << "ru-geolists-creator v" << RGC_VERSION_STRING << std::endl;
    std::cout << "| " << std::setw(15) << "Developer:"    << RGC_DEVELOPER << std::endl;
    std::cout << "| " << std::setw(15) << "License:"      << RGC_LICENSE << std::endl;
    std::cout << "| " << std::setw(15) << "GitHub:"       << RGC_REPOSITORY << std::endl;
    std::cout << "| " << std::setw(15) << "Platform:"     << RGC_PLATFORM << std::endl;
    std::cout << "| " << std::setw(15) << "Architecture:" << RGC_ARCH << std::endl;
    std::cout << "| " << std::setw(15) << "Build date:"   << __DATE__ << std::endl;
    std::cout << "| " << std::setw(15) << "Build time:"   << __TIME__ << std::endl;
    std::cout << "| " << std::setw(15) << "Git commit:"   << RGC_GIT_COMMIT_SHA << std::endl;
    std::cout << "============================================================" << std::endl;
}
