#ifndef TIME_TOOLS_HPP
#define TIME_TOOLS_HPP

#include <string>
#include <ctime>
#include <cstdint>

#define MINUTES_IN_HOUR     60
#define SECONDS_IN_MINUTE   60

#define HOURS_TO_SEC(h)      ((h) * MINUTES_IN_HOUR * SECONDS_IN_MINUTE)

std::string parseUnixTime(const std::time_t& timeU);

uint64_t getCurrentUnixTimestamp();

#endif // TIME_TOOLS_HPP
