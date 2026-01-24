#include "time_tools.hpp"

#include <sstream>
#include <iomanip>
#include <chrono>

std::string
parseUnixTime(const std::time_t& timeU) {
    std::tm* timeInfo = std::localtime(&timeU);

    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%d.%m.%Y %H:%M:%S");

    return oss.str();
}

uint64_t getCurrentUnixTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}