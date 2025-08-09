#ifndef RUADLIST_HPP
#define RUADLIST_HPP

#include <filesystem>
#include <fstream>
#include <regex>

#include "json_io.hpp"
#include "log.hpp"
#include "filter.hpp"

#define RUADLIST_VERSION_SIZE 12

namespace fs = std::filesystem;

extern bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath);

extern bool
parseRuadlistUpdateDatetime(const Json::Value &value, std::time_t& dtOut);

#endif /* RUADLIST_HPP */
