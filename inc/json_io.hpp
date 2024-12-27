#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#include <fstream>
#include <json/json.h>

#include "log.hpp"

extern bool
readJsonFromFile(const std::string& filePath, Json::Value& outValue);

extern bool
writeJsonToFile(const std::string& filePath, const Json::Value& value);

template<typename Type>
extern bool
updateJsonValue(const std::string& filePath, const std::string& key, Type value);

#endif // JSON_IO_HPP
