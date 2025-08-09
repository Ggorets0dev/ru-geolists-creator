#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#include <fstream>
#include <ctime>
#include <optional>
#include <iomanip>

#if __has_include(<jsoncpp/json/json.h>)
#include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
#include <json/json.h>
#endif

#include "log.hpp"

extern bool
readJsonFromFile(const std::string& filePath, Json::Value& outValue);

extern bool
writeJsonToFile(const std::string& filePath, const Json::Value& value);

template<typename Type>
extern bool
updateJsonValue(const std::string& filePath, const std::string& key, Type value);

extern std::optional<std::time_t>
parsePublishTime(const Json::Value& value);

#endif // JSON_IO_HPP
