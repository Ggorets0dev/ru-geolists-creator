#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#include <ctime>
#include <string>
#include <optional>

#if __has_include(<jsoncpp/json/json.h>)
#include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
#include <json/json.h>
#else
#error "JSONCPP header not found"
#endif

bool readJsonFromFile(const std::string& filePath, Json::Value& outValue);

bool writeJsonToFile(const std::string& filePath, const Json::Value& value);

template<typename Type>
bool updateJsonValue(const std::string& filePath, const std::string& key, Type value);

std::optional<std::time_t> parsePublishTime(const Json::Value& value);

#endif // JSON_IO_HPP
