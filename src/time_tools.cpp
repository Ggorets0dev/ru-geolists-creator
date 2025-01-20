#include "time_tools.hpp"

#include <sstream>
#include <iomanip>

std::string
parseUnixTime(const std::time_t& timeU) {
    std::tm* timeInfo = std::localtime(&timeU);

    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%d.%m.%Y %H:%M:%S");

    return oss.str();
}
