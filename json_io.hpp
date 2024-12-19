#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#include <fstream>
#include <json/json.h>

#include "log.hpp"

extern bool
readJsonFromFile(const std::string& filePath, Json::Value& outValue);

#endif // JSON_IO_HPP
